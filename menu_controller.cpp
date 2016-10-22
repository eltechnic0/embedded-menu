/*
 * File:   menu_controller.cpp
 * Author: German Gambon
 *
 * Created on 30 de abril de 2016, 16:36
 */

#include "menu_controller.h"

enum { CURSOR_RIGHT, CURSOR_LEFT, SELECTION_UP, SELECTION_DOWN, VALUE_UP=1, VALUE_DOWN=-1 };

//=============================================================================
// MenuController
//=============================================================================

MenuController::~MenuController() {
	section_->onExit();
	delete section_;
	for (int8_t i = path_.level - 1; i > 0; --i) {
		sec_onexit_ctxs_[i]->onExit();
		delete sec_onexit_ctxs_[i];
	}
}

void MenuController::up() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		// moveSelection(SELECTION_UP);
		nav_ctrl_.onMoveUp();
	} else { // StateInfo::Mode::EDIT
		changeValue(VALUE_UP);
	}
}

void MenuController::down() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		// moveSelection(SELECTION_DOWN);
		nav_ctrl_.onMoveDown();
	} else { // StateInfo::Mode::EDIT
		changeValue(VALUE_DOWN);
	}
}

void MenuController::right() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		if (getCurrentItem_()->isSection()) {
			sectionDown();
		} else {
			startEdit();
		}
	} else { // StateInfo::Mode::EDIT
		moveCursor(CURSOR_RIGHT);
	}
}

void MenuController::enter() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		if (getCurrentItem_()->isSection()) {
			sectionDown();
		} else {
			startEdit();
		}
	} else { // StateInfo::Mode::EDIT
		acceptEdit();
	}
}

void MenuController::escape() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		sectionUp();
	} else { // StateInfo::Mode::EDIT
		cancelEdit();
	}
}

void MenuController::left() {
	if (state_.state_ == StateInfo::Mode::NAVIGATE) {
		sectionUp();
	} else { // StateInfo::Mode::EDIT
		moveCursor(CURSOR_LEFT);
	}
}

void MenuController::sectionDown() {
	// decouple the context of the current section and keep it. Then destroy the
	// section
	sec_onexit_ctxs_[path_.level++] = section_->getOnExitContext();
	uint16_t sec_id = ((SectionMenuItem*)getCurrentItem_())->getSectionId();
	delete section_;
	section_ = menu_.createSection(sec_id);
	path_.section_id[path_.level] = sec_id;
	path_.item_idx[path_.level] = 0;
	state_.result_ = StateInfo::ActionResult::MENU_LEVEL_DOWN;

	nav_ctrl_.onSectionDown();
}

void MenuController::sectionUp() {
	// check if top section
	if (path_.level == 0) {
		state_.result_ = StateInfo::ActionResult::MENU_AT_ROOT;
		return;
	}
	section_->onExit();
	delete section_;
	// TODO: pass this to createSection rather than destroying it
	delete sec_onexit_ctxs_[--path_.level];
	section_ = menu_.createSection(path_.section_id[path_.level]);
	state_.result_ = StateInfo::ActionResult::MENU_LEVEL_UP;

	nav_ctrl_.onSectionUp();
}

void MenuController::startEdit() {
	EndpointMenuItem* item = (EndpointMenuItem*)getCurrentItem_();
	state_.state_ = StateInfo::Mode::EDIT;
	temp_item_.startEdit(item);
	char buf[16];
	item->getValueAsString(buf, sizeof(buf));
	state_.substate_ = strlen(buf) - 1;
	state_.result_ = StateInfo::ActionResult::EDIT_START;
}

void MenuController::acceptEdit() {
	state_.state_ = StateInfo::Mode::NAVIGATE;
	temp_item_.endEdit(true);
	// TODO: send message here or in TempMenuItem?
	state_.result_ = StateInfo::ActionResult::EDIT_ACCEPT;
}

void MenuController::cancelEdit() {
	state_.state_ = StateInfo::Mode::NAVIGATE;
	temp_item_.endEdit(false);
	state_.result_ = StateInfo::ActionResult::EDIT_CANCEL;
}

void MenuController::moveCursor(int8_t dir) {
	EndpointMenuItem* item = (EndpointMenuItem*)getCurrentItem_();
	bool cursor = item->isCursorEditable();
	if (!cursor) {
		state_.result_ = StateInfo::ActionResult::EDIT_CURSOR_BLOCKED;
		return;
	}
	char buf[16];
	item->getValueAsString(buf, sizeof(buf));
	uint8_t len = strlen(buf);
	if (dir == CURSOR_RIGHT) {
		state_.substate_++;
		// jump across the comma
		if (buf[state_.substate_] == '.')
			state_.substate_++;
		if (state_.substate_ == len)
			state_.substate_ = 0;
		state_.result_ = StateInfo::ActionResult::EDIT_CURSOR_RIGHT;
	} else {
		if (state_.substate_ == 0)
			state_.substate_ = len - 1;
		else {
			state_.substate_--;
			// jump across the comma
			if (buf[state_.substate_] == '.')
				state_.substate_--;
		}
		state_.result_ = StateInfo::ActionResult::EDIT_CURSOR_LEFT;
	}
}

void MenuController::changeValue(int8_t dir) {
	EndpointMenuItem* item = (EndpointMenuItem*)getCurrentItem_();
	bool res;
	if (item->isCursorEditable()) {
		char buf[16];
		item->getValueAsString(buf, sizeof(buf));
		uint8_t digit;
		if (item->getType() == MenuItemType::float32) {
			// don't count the comma as a digit
			char* comma = strchr(buf, '.');
			digit = strlen(buf) - state_.substate_ - 1;
			if (state_.substate_ < comma - buf) {
				digit -= 1;
			}
		} else {
			digit = strlen(buf) - state_.substate_ - 1;
		}
		res = item->changeValue(digit, dir);
	} else {
		res = item->changeValue(0, dir);
	}
	if (res) {
		if (dir == VALUE_UP)
			state_.result_ = StateInfo::ActionResult::EDIT_VALUE_UP;
		else
			state_.result_ = StateInfo::ActionResult::EDIT_VALUE_DOWN;
	} else {
		if (dir == VALUE_UP)
			state_.result_ = StateInfo::ActionResult::EDIT_TOP_REACHED;
		else
			state_.result_ = StateInfo::ActionResult::EDIT_BOTTOM_REACHED;
	}
}

// void MenuController::moveSelection(int8_t dir) {
// 	if (dir == SELECTION_UP) {
// 		if (path_.item_idx == 0) {
// 			path_.item_idx = section_.getSize() - 1;
// 			state_.result_ = StateInfo::ActionResult::MENU_MOVE_BOTTOM;
// 		}
// 		else {
// 			path_.item_idx--;
// 			state_.result_ = StateInfo::ActionResult::MENU_MOVE_UP;
// 		}
// 	} else {
// 		path_.item_idx++;
// 		if (path_.item_idx == section_.getSize()) {
// 			path_.item_idx = 0;
// 			state_.result_ = StateInfo::ActionResult::MENU_MOVE_TOP;
// 		} else {
// 			state_.result_ = StateInfo::ActionResult::MENU_MOVE_DOWN;
// 		}
// 	}
// }
