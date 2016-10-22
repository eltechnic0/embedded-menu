/*
 * File:   menu_item.h
 * Author: German Gambon
 *
 * Created on 9 de abril de 2016, 17:11
 */

#ifndef MENU_ITEM_H
#define MENU_ITEM_H

#include <string.h>
#include <stdio.h>
#include <cstdint>
#include <cfloat>
#include <climits>
#include "functors.h"
#include "utility.h"

typedef unsigned int uint;

static OnStartEditNOP onStartEditNOP;
static OnEndEditNOP onEndEditNOP;
static OnChangeNOP onChangeNOP;

/**
 * Number of bytes of the biggest data type that a menu item must be able to
 * hold. Currently it's 4 bytes for 32 bit integers, float and double types in
 * AVR atmega-2560.
 */
#define MAX_MENU_ITEM_VALUE_BYTES 4

/**
 * Types of menu items for down-casting when required.
 */
enum class MenuItemType {
    section,
    int32,
    uint16,
    float32,
    sel8u,
    sel32u,
    boolean
};

//=============================================================================
// AbstractMenuItem
//=============================================================================

/**
 * Abstract most-general description of a menu item prividing a common
 * virtual interface for navigation, and querying of basic info.
 */
class AbstractMenuItem {
    const MenuItemType type_;
    bool active_;
    const char *info_string_;
public:
    AbstractMenuItem(MenuItemType type, bool active, const char *info_str)
        : type_(type)
        , active_(active)
        , info_string_(info_str) { }
    AbstractMenuItem(const AbstractMenuItem& other);
    virtual ~AbstractMenuItem() = 0;
    /**
     * Get item display name
     * @param buf destination buffer to copy the info string to
     * @param sz size of the destination buffer
     */
    void getInfoString(char *buf, size_t sz) const {
        strncpy(buf, info_string_, sz);
    }
    /**
     * Get item type - for static conversions
     * @return the type of this item
     */
    MenuItemType getType() const { return type_; }
    /** For convenience */
    bool isSection() const { return type_ == MenuItemType::section; }
    bool isActive() const { return active_; }
    /** Enable/disable item for down-navigation or editing, depending on the type */
    void setActive(bool b) { active_ = b; }
};
inline AbstractMenuItem::~AbstractMenuItem() { }

//=============================================================================
// SectionMenuItem
//=============================================================================

/**
 * Item representing a new section. It contains the ID of such section.
 */
class SectionMenuItem : public AbstractMenuItem {
public:
    SectionMenuItem(uint16_t section_id, bool active, const char *info_str)
        : AbstractMenuItem(MenuItemType::section, active, info_str)
        , section_id_(section_id) { }
    ~SectionMenuItem() override { }
    uint16_t getSectionId() const { return section_id_; };
private:
    uint16_t section_id_;
};

//=============================================================================
// ValueUnion
//=============================================================================

/**
 * This structure is used as a memory pool in which to store any type of value
 * that can fit in it, in order to be able to restore the value in case of
 * editing cancellation.
 * It is similar to a union but we can avoid type checking for later convenience
 * in MenuItem classes.
 */
struct ValueUnion {
	char mem[MAX_MENU_ITEM_VALUE_BYTES];
};

//=============================================================================
// EndpointMenuItem
//=============================================================================

/**
 * Abstract class representing menu items that contain a value. Can be used as
 * virtual interface. Provides a generic interface to manage the wrapped value
 * but without making operations to it other than copying (for TempMenuItem).
 * More detailed control of this value will require down-conversion to the
 * actual class of the object.
 */
class EndpointMenuItem : public AbstractMenuItem {
protected:
    void *data_;
private:
    const uint16_t value_id_;
    const bool cursor_;
    OnStartEditF& onStartEdit_;
    OnEndEditF& onEndEdit_;
    OnChangeF& onChange_;
public:
    EndpointMenuItem(MenuItemType type, bool active, const char *info_str, void *value,
            OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id,
            bool cursor)
        : AbstractMenuItem(type, active, info_str)
        , data_(value)
        , value_id_(value_id)
		, cursor_(cursor)
        , onStartEdit_(f)
        , onEndEdit_(g)
        , onChange_(h) { }
    // EndpointMenuItem(const EndpointMenuItem& orig);
    virtual ~EndpointMenuItem() = 0;
	/** Get the string representation of the value wrapped by the item */
    virtual void getValueAsString(char *buf, size_t sz) const = 0;
	/* These 4 functions are used by the TempMenuItem */
    void * getValuePointer() { return data_; }
    void setValuePointer(void *value) { data_ = value; }
	virtual void setValueUnion(ValueUnion* value) = 0;
	virtual ValueUnion getValueUnion() const = 0;
	/**
	 * Change the value wrapped by this menu item.
	 * @param digit the digit to be changed, if the item is cursor editable.
	 * Digit 0 is the lowest.
	 * @param direction true is up, false is down
	 * @return whether the change was successful. If not, limits have been reached.
	 */
    bool changeValue(uint8_t digit, int8_t direction) {
        if (changeValue_(digit, direction)) {
            onChange_(*this);
            return true;
        }
        return false;
    }
	/**
	 * Get the ID of the variable wrapped by this item. Useful for updating the
	 * value stored in EEPROM after an accepted change.
	 */
    uint16_t getValueStorageId() const { return value_id_; }
	/**
	 * Whether the item can be edited freely with a cursor, or otherwise in
	 * fixed steps.
	 */
    bool isCursorEditable() const { return cursor_; }
    void onChange() { onChange_(*this); }
    void onStartEdit() { onStartEdit_(*this); }
    void onEndEdit() { onEndEdit_(*this); }
protected:
    /**
     * Override this so that onChange callback is automatically called. Returns
     * true if the value was changed, false if trying to pass through limits.
     */
    virtual bool changeValue_(uint8_t digit, int8_t direction) = 0;
};
inline EndpointMenuItem::~EndpointMenuItem() { }

//=============================================================================
// IntegerRangeDefVal
//=============================================================================

/**
* Template helper class for choosing min and max depending on the type (conditional
* template).
*/
template<typename T>
struct IntegerRangeDefVal { };

/** Template specialization */
template<>
struct IntegerRangeDefVal<int32_t> {
    static const int32_t min = INT32_MAX;
    static const int32_t max = INT32_MAX;
};

/** Template specialization */
template<>
struct IntegerRangeDefVal<uint16_t> {
    static const uint16_t min = 0;
    static const uint16_t max = UINT16_MAX;
};

//=============================================================================
// IntegerMenuItem
//=============================================================================

template<typename T, MenuItemType item_type>
class IntegerMenuItem : public EndpointMenuItem {
public:
    /**
     * Range specification structure. Default values are chosen statically
     * depending on T. Default init of the structure allows full range editing
     * with the cursor. In C++14 defining a constructor is no longer needed.
     */
    struct RangeSpec {
        T min = IntegerRangeDefVal<T>::min;
        T max = IntegerRangeDefVal<T>::max;
        T step = 0;
        RangeSpec(T min, T max, T step) : min(min), max(max), step(step) { }
    };
private:
    const RangeSpec range_;
    /** Alias for convenience */
public:

    IntegerMenuItem(bool active, const char* info_str, T* value, const RangeSpec& range,
            OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id)
        : EndpointMenuItem(item_type, active, info_str, (void*)value, f, g, h,
                value_id, range.step == 0)
        , range_(range) { }

    IntegerMenuItem(bool active, const char* info_str, T* value, const RangeSpec& range, uint16_t value_id)
        : IntegerMenuItem(active, info_str, (void*)value, range, onStartEditNOP,
				onEndEditNOP, onChangeNOP, value_id) {}
    /**
    * Shorter constructor that uses default init for range specs, which chooses
    * full range and cursor editing.
    */
    IntegerMenuItem(bool active, const char* info_str, T* value,
            OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id)
        : EndpointMenuItem(item_type, active, info_str, (void*)value, f, g, h,
                value_id, true) { }

    ~IntegerMenuItem() override { }

    void getValueAsString(char* buf, size_t sz) const override {
        snprintf(buf, sz, "%d", *(T*)data_);
    }

    /**
     * Increase or decrease the value of the variable wrapped by the item. If
     * step==0 add/substract 10^digit, otherwise use the ammount stored in step.
     * If the value is not at the limits, do the operation and clamp to them,
     * returning true. If already at the limits, do nothign and return false.
     */
    bool changeValue_(uint8_t digit, int8_t direction) override {
		T& value = *(T*)data_;
        if (value == range_.max && direction)
            return false;
        if (value == range_.min && !direction)
            return false;

        T step = range_.step;
        if (step == 0)
            step = base10pow(digit);

        if (direction >= 0)
            value += step;
        else
            value -= step;

        if (value > range_.max)
            value = range_.max;

        if (value < range_.min)
            value = range_.min;

        return true;
    }

    T getValue() const { return *(T*)data_; }
    void setValue(T value) { *(T*)data_ = value; }
	void setValueUnion(ValueUnion* value) override { *(T*)data_ = *(T*)value; }
	ValueUnion getValueUnion() const override { return *(ValueUnion*)data_; }
};

/** Aliases for convenience */
typedef IntegerMenuItem<int32_t, MenuItemType::int32> Int32MenuItem;
typedef IntegerMenuItem<uint16_t, MenuItemType::uint16> Uint16MenuItem;

//=============================================================================
// SelectionMenuItem
//=============================================================================

/**
 * Class for items that contain only a predefined set of values, each with an
 * associated label that is displayed instead of the actual value. E.g. values
 * for the value: [30, 60, 300, 600, 1800, 3600] with labels ["30s", "1m", "5m",
 * "10m", "30m", "1h"].
 */
template<typename T, MenuItemType item_type>
class SelectionMenuItem : public EndpointMenuItem {
    const char * const* labels_;
    const T* values_;
    const uint8_t count_;
    uint8_t index_{0};
public:
	SelectionMenuItem(bool active, const char* info_string, T* value,
			const char * const* labels, const T* values, uint8_t count,
			OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint8_t value_id)
		: EndpointMenuItem(item_type, active, info_string, (void*)value, f, g, h, value_id, false)
		, labels_(labels)
		, values_(values)
		, count_(count) {
		setValue(*value);
	}
	SelectionMenuItem(bool active, const char* info_string, T* value,
			const char * const* labels, const T* values, uint8_t count, uint8_t value_id)
		: SelectionMenuItem(active, info_string, value, labels, values, count,
				onStartEditNOP, onEndEditNOP, onChangeNOP, value_id) {}

	void getValueAsString(char* buf, size_t sz) const override {
        strncpy(buf, labels_[index_], sz);
    }
    bool changeValue_(uint8_t digit, int8_t direction) {
        // increase/decrease with wrap-around
        if (direction >= 0) {
            index_++;
            if (index_ == count_)
                index_ = 0;
        } else {
            if (index_ == 0)
                index_ = count_ - 1;
            else
                index_--;
        }
        // update the destination variable
        *(T*)data_ = values_[index_];
        return true;
    }
    T getValue() const { return *(T*)data_; }
    void getValues(const T* values, uint8_t& sz) const {
		values = values_;
		sz = count_;
	}
    uint8_t getIndex() const { return index_; }
	void setValue(T value) {
        // search for the current value
        for (uint8_t i = 0; i < count_; i++) {
            if (values_[i] == value)
                index_ = i;
        }
        // TODO: consider adding some sort of assertion to warn when value is
        // not found in values_
	}
    void setValueIdx(uint8_t index) { index_ = index < count_ ? index : index_; }
	void setValueUnion(ValueUnion* value) override { *(T*)data_ = *(T*)value; }
	ValueUnion getValueUnion() const override { return *(ValueUnion*)data_; }
};

/** Aliases for convenience */
typedef SelectionMenuItem<uint8_t, MenuItemType::sel8u> Sel8uMenuItem;
typedef SelectionMenuItem<uint32_t, MenuItemType::sel32u> Sel32uMenuItem;

//=============================================================================
// BoolMenuItem
//=============================================================================

class BoolMenuItem : public EndpointMenuItem {
public:
    BoolMenuItem(bool active, const char* info_string, bool* value,
            OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id)
        : EndpointMenuItem(MenuItemType::boolean, active, info_string, (void*)value,
                f, g, h, value_id, false) {}
    BoolMenuItem(bool active, const char* info_string, bool* value, uint16_t value_id)
        : EndpointMenuItem(MenuItemType::boolean, active, info_string, (void*)value,
                onStartEditNOP, onEndEditNOP, onChangeNOP, value_id, false) {}
    void getValueAsString(char* buf, size_t sz) const override {
        if (*(bool*)data_)
            strncpy(buf, "On", sz);
        else
            strncpy(buf, "Off", sz);
    }
    bool changeValue_(uint8_t digit, int8_t direction) {
        *(bool*)data_ = *(bool*)data_ ? false : true;
        return true;
    }
    bool getValue() const { return *(bool*)data_; }
    void setValue(bool value) { *(bool*)data_ = value; }
	void setValueUnion(ValueUnion* value) override { *(bool*)data_ = *(bool*)value; }
	ValueUnion getValueUnion() const override { return *(ValueUnion*)data_; }
};

//=============================================================================
// DecimalRangeDefVal
//=============================================================================

/**
* Template helper class for choosing min and max depending on the type (conditional
* template).
*/
template<typename T>
struct DecimalRangeDefVal { };

/** Float template specialization */
template<>
struct DecimalRangeDefVal<float> {
    static constexpr float min{FLT_MIN};
    static constexpr float max{FLT_MAX};
};

/** Double template specialization */
template<>
struct DecimalRangeDefVal<double> {
    static constexpr double min{DBL_MIN};
    static constexpr double max{DBL_MAX};
};

//=============================================================================
// DecimalMenuItem
//=============================================================================

template<typename T, MenuItemType item_type>
class DecimalMenuItem : public EndpointMenuItem {
public:
    /**
     * Range specification structure. Default values are chosen statically
     * depending on T. Default init of the structure allows full range editing
     * with the cursor. In C++14 defining a constructor is no longer needed.
     */
    struct RangeSpec {
        T min = DecimalRangeDefVal<T>::min;
        T max = DecimalRangeDefVal<T>::max;
        T step = 0;
        RangeSpec(T min, T max, T step) : min(min), max(max), step(step) { }
    };
private:
    /** Number of digits of the string representation */
    RangeSpec range_;
    uint8_t precision_;
    /** Alias for convenience */
public:

    DecimalMenuItem(bool active, const char* info_str, T* value, const RangeSpec& range,
            uint8_t precision, OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id)
        : EndpointMenuItem(item_type, active, info_str, (void*)value, f, g, h, value_id, range.step == 0)
        , range_(range)
        , precision_(precision) { }

    DecimalMenuItem(bool active, const char* info_str, T* value, const RangeSpec& range,
            uint8_t precision, uint16_t value_id)
		: DecimalMenuItem(active, info_str, value, range, precision, onStartEditNOP,
				onEndEditNOP, onChangeNOP, value_id) {}
    /**
    * Shorter constructor that uses default init for range specs, which chooses
    * full range and cursor editing.
    */
    DecimalMenuItem(const char* info_str, bool active, T* value,
            uint8_t precision, OnStartEditF& f, OnEndEditF& g, OnChangeF& h, uint16_t value_id)
        : EndpointMenuItem(item_type, active, info_str, (void*)value, f, g, h, value_id, true)
        , precision_(precision) { }

    ~DecimalMenuItem() override { }

    void getValueAsString(char* buf, size_t sz) const override {
        ftoaFix(buf, sz, *(T*)data_, precision_);
    }

    /**
     * Increase or decrease the value of the variable wrapped by the item. If
     * step==0 add/substract 10^digit, otherwise use the ammount stored in step.
     * If the value is not at the limits, do the operation and clamp to them,
     * returning true. If already at the limits, do nothign and return false.
     */
    bool changeValue_(uint8_t digit, int8_t direction) override {
		T& value = *(T*)data_;
        if (value == range_.max && direction)
            return false;
        if (value == range_.min && !direction)
            return false;

        // TODO: seems to be a bug when editing decimal positions
        T step = range_.step;
        if (step == 0)
            step = fbase10pow<T>(digit);

        if (direction >= 0)
            value += step;
        else
            value -= step;

        if (value > range_.max)
            value = range_.max;

        if (value < range_.min)
            value = range_.min;

        return true;
    }

    T getValue() const { return *(T*)data_; }
    void setValue(T value) { *(T*)data_ = value; }
	void setValueUnion(ValueUnion* value) override { *(T*)data_ = *(T*)value; }
	ValueUnion getValueUnion() const override { return *(ValueUnion*)data_; }
};

/** Aliases for convenience */
typedef DecimalMenuItem<float, MenuItemType::float32> Float32MenuItem;

//=============================================================================
// TempMenuItem
//=============================================================================

/**
 * This class swaps the target variable that a menu item wraps so that changes
 * made to the item after calling startEdit() are done to a temporal variable.
 * When endEdit() is called, the original item is put back into its original
 * state and if the change is accepted, the item value is set to that of the
 * temporal variable.
 *
 * This allows to use a menu item scheme where the temporal values under change
 * don't need to be stored per item, but just on a TempMenuItem which can hold
 * any value that fits in the structure ValueUnion. This temp item provides an
 * easy interface for stating and ending edit mode. During edit mode, the item
 * can be managed normally, and changes to it will be done to the temp var.
 */
class TempMenuItem {
private:
    void* source_;
    ValueUnion temp_;
    EndpointMenuItem* item_;
public:
	/**
	 * Swap the item's wrapped variable with a temporal one so that changes
	 * don't affect to original one.
	 * @param item the item to be put into edit mode
	 */
    void startEdit(EndpointMenuItem* item) {
        item->onStartEdit();
        item_ = item;
        source_ = item_->getValuePointer(); // get original target var
        temp_ = item_->getValueUnion(); // copy current value into temp var
        item_->setValuePointer((void*)&temp_); // swap target variable
    }
	/**
	 * Revert the swap.
	 * @param accept set the destination variable with the new value.
	 */
    void endEdit(bool accept) {
		item_->setValuePointer(source_); // swap back to original target
        if (accept) {
            item_->onEndEdit();
            item_->setValueUnion(&temp_); // set the new value
			// TODO: send message to store in eeprom
        }
    }
};

// TODO: EnablerMenuItem

#endif /* MENU_ITEM_H */
