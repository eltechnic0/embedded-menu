/*
 * File:   menu_controller.h
 * Author: German Gambon
 *
 * Created on 9 de abril de 2016, 17:11
 */

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <string.h>
#include <stdio.h>
#include <cstdint>
#include <cfloat>
#include "functors.h"
#include "utility.h"
#include "menu_factory.h"
#include "menu_item.h"
#include "menu_section.h"

#define MAX_MENU_DEPTH 8

//=============================================================================
// StateInfo
//=============================================================================

class StateInfo {
public:

	enum class Mode : uint8_t { NAVIGATE, EDIT };

	/**
	 * Used for indicating the result of a MenuController action, so that the
	 * parent object using it can act accordingly (beep on top reached, etc.)
	 */
	enum class ActionResult : uint8_t {
		OK,

		EDIT_TOP_REACHED,
		EDIT_BOTTOM_REACHED,
		EDIT_CURSOR_BLOCKED,
		EDIT_CURSOR_RIGHT,
		EDIT_CURSOR_LEFT,
		EDIT_INACTIVE,
		EDIT_START,
		EDIT_ACCEPT,
		EDIT_CANCEL,
		EDIT_VALUE_UP,
		EDIT_VALUE_DOWN,

		MENU_LEVEL_DOWN,
		MENU_LEVEL_UP,
		MENU_AT_ROOT, // tried to go one level up when already at the root
		MENU_MOVE_DOWN, // navigate to the item below
		MENU_MOVE_UP, // navigate to the item above
		MENU_MOVE_TOP, // selection to top after wrapping around at the bottom
		MENU_MOVE_BOTTOM // the other way around
	};

private:
	uint8_t substate_{0}; // in edit mode it is the position of the cursor
	Mode state_{Mode::NAVIGATE};
	ActionResult result_{ActionResult::MENU_LEVEL_DOWN}; // to signal screen refresh on start
	friend class MenuController;
	friend class MenuNavByPages;

public:
	uint8_t getSubstate() const {
		return substate_;
	}

	Mode getState() const {
		return state_;
	}

	ActionResult getActionResult() const {
		return result_;
	}

	void resetActionResult() {
		result_ = ActionResult::OK;
	}

	/** Convenience function */
	bool isLevelChanged() {
		return result_ == ActionResult::MENU_LEVEL_DOWN ||
			result_ == ActionResult::MENU_LEVEL_UP;
	}
};

//=============================================================================
// MenuNavByPages
//=============================================================================

class MenuNavByPages {
	class MenuController& ctrl_;
	// AbstractMenuItem* first_;
	// AbstractMenuItem* last_;
	uint8_t first_idx_;
	uint8_t last_idx_;
	const uint8_t visible_;
private:
	// set first and last pointers according to page number and size
	void changePage(uint8_t page);
public:
	MenuNavByPages(class MenuController& ctrl, uint8_t visible);
	// show first page
	void onSectionDown();
	// show previous page
	void onSectionUp();
	void onMoveDown();
	void onMoveUp();
	// AbstractMenuItem* getFirst() { return first_; }
	// AbstractMenuItem* getLast() { return last_; }
	// AbstractMenuItem** getVisible() { return &first_; }
	AbstractMenuItem** getVisible(uint8_t* size);
	/** Max number of positions -- the number of pages */
	// NOTE: not sure about this
	// uint8_t getCount() { return (last_ - first_)/sizeof(AbstractMenuItem); }
	uint8_t getCount() { return last_idx_ - first_idx_; }
	/** Current position in the count -- the page */
	uint8_t getPosition();
};

//=============================================================================
// MenuController
//=============================================================================

class MenuController {
public:
	/**
	 * Description of the menu path followed until the current section.
	 */
	struct Path {
		const uint8_t max_depth = MAX_MENU_DEPTH;
		uint8_t level{0}; // current level in the menu
		uint8_t section_id[MAX_MENU_DEPTH]{0};
		uint8_t item_idx[MAX_MENU_DEPTH]{0};
	};

private:
	MenuFactory& menu_; // defines the menu and generates sections
	BaseMenuSection *section_;
	Path path_;
	TempMenuItem temp_item_;
	StateInfo state_;
	// EventQueue& event_queue_;
	SecOnExitCtx* sec_onexit_ctxs_[MAX_MENU_DEPTH];

	friend MenuNavByPages;
	// MenuNavByPages nav_ctrl_{*this, 5};
	MenuNavByPages nav_ctrl_;

private:
	void onPreKeyEvent() {}
	void onPostKeyEvent() {}
	AbstractMenuItem* getCurrentItem_() { return section_->getItems()[path_.item_idx[path_.level]]; }
	void setCurrentIndex(uint8_t index) { path_.item_idx[path_.level] = index; }

	void sectionDown();
	void sectionUp();
	void startEdit();
	void acceptEdit();
	void cancelEdit();
	void moveCursor(int8_t dir);
	void changeValue(int8_t dir);

public:
	// MenuController(IMenuFactory& menu, EventQueue& event_queue)
	// 	: menu_(menu), event_queue_(event_queue) { }
	MenuController(MenuFactory& menu)
		: menu_(menu), section_(menu.createRoot()), nav_ctrl_{*this, 5} {
		path_.section_id[0] = section_->getId();
	}
	~MenuController();
	void right();
	void left();
	void down();
	void up();
	void escape();
	void enter();
	void onPreDraw() { }
	void onPostDraw() { }
	const AbstractMenuItem* const * getItems() { return section_->getItems(); }
	uint8_t getCurrentIndex() { return path_.item_idx[path_.level]; }
	const AbstractMenuItem* getCurrentItem() { return getCurrentItem_(); }
	// const AbstractMenuItem* getFirstVisible() { return nav_ctrl_.getFirst(); }
	// const AbstractMenuItem* getLastVisible() { return nav_ctrl_.getLast(); }
	const AbstractMenuItem* const * getVisibleItems(uint8_t* size) { return nav_ctrl_.getVisible(size); }
	const Path& getPath() { return path_; }
	StateInfo& getStateInfo() { return state_; }
};

//=============================================================================
// MenuNavByPages
//=============================================================================

inline MenuNavByPages::MenuNavByPages(MenuController& ctrl, uint8_t visible)
	: ctrl_(ctrl), visible_(visible) {
	changePage(0);
}

// inline void MenuNavByPages::changePage(uint8_t page) {
// 	uint8_t first_idx = page * visible_;
// 	first_ = ctrl_.section_->getItems()[first_idx];

// 	uint8_t size = ctrl_.section_->getSize();
// 	uint8_t last_idx = first_idx + visible_;
// 	last_idx = last_idx < size ? last_idx : size;

// 	last_ = ctrl_.section_->getItems()[last_idx];
// }

inline void MenuNavByPages::changePage(uint8_t page) {
	first_idx_ = page * visible_;

	uint8_t size = ctrl_.section_->getSize();
	last_idx_ = first_idx_ + visible_;
	last_idx_ = last_idx_ < size ? last_idx_ : size;
}

inline AbstractMenuItem** MenuNavByPages::getVisible(uint8_t* size) {
	*size = getCount();
	// return (&(ctrl_.section_->getItems()[ctrl_.getCurrentIndex() / visible_]));
	return &(ctrl_.section_->getItems()[first_idx_]);
}

// show first page
inline void MenuNavByPages::onSectionDown() {
	changePage(0);
}

// show previous page
inline void MenuNavByPages::onSectionUp() {
	changePage(ctrl_.getCurrentIndex() / visible_);
}

inline void MenuNavByPages::onMoveDown() {
	ctrl_.setCurrentIndex(ctrl_.getCurrentIndex() + 1);
	if (ctrl_.getCurrentIndex() == ctrl_.section_->getSize()) {
		// no more pages - back to top
		ctrl_.setCurrentIndex(0);
		changePage(0);
		ctrl_.state_.result_ = StateInfo::ActionResult::MENU_MOVE_TOP;
	} else {
		// down
		if (ctrl_.getCurrentIndex() == last_idx_) {
			// next page
			changePage(ctrl_.getCurrentIndex() / visible_);
		}
		ctrl_.state_.result_ = StateInfo::ActionResult::MENU_MOVE_DOWN;
	}
}

inline void MenuNavByPages::onMoveUp() {
	if (ctrl_.getCurrentIndex() == 0) {
		// no more pages - go to last
		ctrl_.setCurrentIndex(ctrl_.section_->getSize() - 1);
		changePage(ctrl_.getCurrentIndex() / visible_);
		ctrl_.state_.result_ = StateInfo::ActionResult::MENU_MOVE_BOTTOM;
	} else {
		// up
		ctrl_.setCurrentIndex(ctrl_.getCurrentIndex() - 1);
		if (ctrl_.getCurrentIndex() < first_idx_) {
			// previous page
			changePage(ctrl_.getCurrentIndex() / visible_);
		}
		ctrl_.state_.result_ = StateInfo::ActionResult::MENU_MOVE_UP;
	}
}

/** Current position in the count -- the page */
inline uint8_t MenuNavByPages::getPosition() {
	return ctrl_.getCurrentIndex() / visible_;
}

#endif /* MENU_CONTROLLER_H */
