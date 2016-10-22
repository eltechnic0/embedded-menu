/*
 * File:   test_menu.h
 * Author: German Gambon
 *
 * Created on 3 de mayo de 2016, 22:03
 */

#ifndef TEST_MENU_H
#define TEST_MENU_H

#include "menu_factory.h"
#include "menu_section.h"

// used so that the PROGMEM keyword doesn't result in an error when compiling with g++
#define PROGMEM

enum EventId {
	EEPROM_BOOL_VAR,
	EEPROM_FLOAT_VAR,
	EEPROM_UINT_VAR,
};

enum SectionId {
    ROOT,
    SETTINGS
};

// some global vars to be modified through the menu
struct AppManager {
	bool boolean{false};
	float floating{500};
	uint32_t uinteger{300};
};
static AppManager app_mgr;

//=============================================================================
// Sections
//=============================================================================

//========== Root Section ===========

class RootSection : public SectionTemplate<NoCtx, NoOnExit, 2, ROOT> {
	static constexpr const char *settings_info PROGMEM {"Settings"};
    static constexpr const char *calibration_info PROGMEM {"Calibration"};

    SectionMenuItem settings{SETTINGS, true, settings_info};
	SectionMenuItem calibration{ROOT, true, calibration_info};
public:
    RootSection() : SectionTemplate({&settings, &calibration}) { }
};

//========== Settings Section ===========

class SettingsSection : public SectionTemplate<NoCtx, NoOnExit, 4, SETTINGS> {
    AppManager& app_mgr_;
	// menu strings can be defined in the section class itself
    static constexpr const char *bluetooth_info PROGMEM {"Bluetooth"};
    static constexpr const char *idle_timeout_info PROGMEM {"Idle timeout"};
    static constexpr const char *s5m PROGMEM {"5m"};
    static constexpr const char *s1h PROGMEM {"1h"};
	static constexpr const char *cont_thres_info PROGMEM {"Cont. threshold"};
    static constexpr const char *continuity_info PROGMEM {"Continuity"};
	// option 1: does not work with the linker -- silly error/bug!!
    // static constexpr const char *idle_timeout_labels[] {s5m, s1h};
    // static constexpr const uint32_t idle_timeout_values[2] {300, 3600};
	// option 2
	static const char *idle_timeout_labels[2];
    static const uint32_t idle_timeout_values[2];

	// definition of menu items
    BoolMenuItem bluetooth{true, bluetooth_info, &app_mgr_.boolean, EEPROM_BOOL_VAR};
    // this type of initialization gets better error messages, but is equivalent
    Sel32uMenuItem idle_timeout = Sel32uMenuItem(true, idle_timeout_info,
		&app_mgr_.uinteger,	idle_timeout_labels, idle_timeout_values, 2, EEPROM_UINT_VAR);
	Float32MenuItem cont_thres{true, cont_thres_info, &app_mgr_.floating, {0, 1E3, 0}, 2, EEPROM_FLOAT_VAR};
    BoolMenuItem continuity{true, continuity_info, &app_mgr_.boolean, EEPROM_BOOL_VAR};

public:
    SettingsSection(AppManager& app_mgr)
		: SectionTemplate({&bluetooth, &idle_timeout, &cont_thres, &continuity})
		, app_mgr_(app_mgr) { }
};
// for option 2
const uint32_t SettingsSection::idle_timeout_values[] {300, 3600};
const char *SettingsSection::idle_timeout_labels[] {SettingsSection::s5m,
	SettingsSection::s1h};


//=============================================================================
// TestMenu
//=============================================================================

class TestMenu : public MenuFactory {
public:
    ~TestMenu() { }
    BaseMenuSection* createSection(uint16_t section) override {
        if (ROOT == section)
            return new RootSection();
        if (SETTINGS == section)
            return new SettingsSection(app_mgr);
		// TODO: change this so that an invalid id fails compiling
        return NULL; // should never happen -- crashes the program
    }
    BaseMenuSection* createRoot() override { return new RootSection(); }
#ifdef DEBUG_MODE
    virtual void onPreDraw() { }
#endif
};

#endif
