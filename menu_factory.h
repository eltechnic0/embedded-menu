/*
 * File:   menu_factory.h
 * Author: German Gambon
 *
 * Created on 3 de mayo de 2016, 21:55
 */

#ifndef MENU_FACTORY_H
#define MENU_FACTORY_H

// forward declaration
class BaseMenuSection;

//=============================================================================
// MenuFactory
//=============================================================================

class MenuFactory {
public:
    virtual ~MenuFactory() = 0;
    virtual BaseMenuSection* createSection(uint16_t section) = 0;
    virtual BaseMenuSection* createRoot() = 0;
#ifdef DEBUG_MODE
    virtual void onPreDraw() = 0;
#endif
};
inline MenuFactory::~MenuFactory() { }

#endif
