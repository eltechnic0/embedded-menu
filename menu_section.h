/*
 * File:   menu_section.h
 * Author: German Gambon
 *
 * Created on 22 de abril de 2016, 21:10 */

#ifndef MENU_SECTION_H
#define MENU_SECTION_H

#include "menu_item.h"
#include "functors.h"

typedef unsigned int uint;

typedef void NoCtx;
typedef void NoOnExit;

//=============================================================================
// Helper classes
//=============================================================================

/**
 * Empty, virtual base class as interface for correct destruction of any
 * derived context used for a given section. Used as dummy section ctx too.
 */
struct SecCtx {
    virtual ~SecCtx();
};
inline SecCtx::~SecCtx() { }

/**
 * Structure to encapsulate the data required to run onExit functors after their
 * section has been destroyed.
 *
 * The class constructor expects heap-allocated objects and they are guaranteed to be
 * deleted by the destructor.
 */
struct SecOnExitCtx {
    SecCtx *ctx;
    OnExitF *onExitF;
    uint section;
    SecOnExitCtx(SecCtx* ctx, OnExitF* onExitF, uint section)
        : ctx(ctx), onExitF(onExitF), section(section) { }
    ~SecOnExitCtx() {
        delete onExitF;
        delete ctx;
    }
	void onExit() { (*onExitF)(); }
};

//=============================================================================
// BaseMenuSection
//=============================================================================

/**
 * Base class for sections, encapsulating all implementation details and
 * allowing a very simple way to define any section.
 */
class BaseMenuSection {
public:
    BaseMenuSection(uint section) : section_(section) { }
    /**
     * Destroy everything or almost everything if control has been transfered.
     */
    virtual ~BaseMenuSection() = 0;
    /** Number of items in the section. */
    virtual uint8_t getSize() = 0;
    virtual AbstractMenuItem** getItems() = 0;
    /** Return the type of the section. */
    uint getId() { return section_; }
    /**
     * Returns the context required for running onExit callbacks so that they
     * can be executed after the section has been destroyed.
     */
    virtual SecOnExitCtx* getOnExitContext() { return NULL; }
    virtual void onEnter() { }
    virtual void onExit() { }
protected:
    uint section_;
    /**
     * Reference to the app manager or equivalent to give full control of the
     * platform to the sections.
     */
    // class ApplicationManager& manager;
};
inline BaseMenuSection::~BaseMenuSection() { }

//=============================================================================
// Section template
//=============================================================================

/**
 * Template of a section that should be derived in order to define new
 * sections with minimal redundancy.
 *
 * To create a new section derive from SectionTemplate and specify the context,
 * onExit functor, number of items, and section ID. Then define the items and
 * their functors statically. In the constructor just pass context init
 * parameters and the addresses of the items. The first item should appear at the
 * top.
 */
template<typename CTX, typename ONEXIT, size_t SZ, uint SEC>
class SectionTemplate : public BaseMenuSection {
protected:
    static const uint8_t size_{SZ};
    CTX ctx_;
    // hack to allow list initialization
    struct Items {
        // item list
        AbstractMenuItem* items[size_];
    } items_;
    // section's onExit callback
    ONEXIT onExit_{ctx_};
public:
    SectionTemplate(const CTX& ctx, const Items& items)
        : BaseMenuSection(SEC), ctx_(ctx), items_(items) { }
    ~SectionTemplate() { }
    void onExit() { onExit_(); }

    uint8_t getSize() override { return size_; }
    AbstractMenuItem** getItems() override { return items_.items; }
    SecOnExitCtx* getOnExitContext() override {
        return new SecOnExitCtx{new CTX(ctx_), new ONEXIT(onExit_), SEC};
    }
};

/**
 * Specialization for no onExit callback. NOTE: is this one actually useful??
 */
template<typename CTX, size_t SZ, uint SEC>
class SectionTemplate<CTX, void, SZ, SEC> : public BaseMenuSection {
protected:
    CTX ctx_;
    static const uint8_t size_{SZ};
    struct Items {
        AbstractMenuItem* items[size_];
    } items_;
public:
    SectionTemplate(const CTX& ctx, const Items& items)
        : BaseMenuSection(SEC), ctx_(ctx), items_(items) { }
    ~SectionTemplate() { }

    uint8_t getSize() override { return size_; }
    AbstractMenuItem** getItems() override { return items_.items; }
};

/**
 * Specialization for no context nor onExit callback.
 */
template<size_t SZ, uint SEC>
class SectionTemplate<void, void, SZ, SEC> : public BaseMenuSection {
protected:
    static const uint8_t size_{SZ};
    struct Items {
        AbstractMenuItem* items[size_];
    } items_;
public:
    SectionTemplate(const Items& items)
        : BaseMenuSection(SEC), items_(items) { }
    ~SectionTemplate() { }

    uint8_t getSize() override { return size_; }
    AbstractMenuItem** getItems() override { return items_.items; }
};

/**
 * Specialization for no context.
 */
template<typename ONEXIT, size_t SZ, uint SEC>
class SectionTemplate<void, ONEXIT, SZ, SEC> : public BaseMenuSection {
protected:
    static const uint8_t size_{SZ};
    struct Items {
        AbstractMenuItem* items[size_];
    } items_;
    ONEXIT onExit_; // functor without context in this case
public:
    SectionTemplate(const Items& items)
        : BaseMenuSection(SEC), items_(items) { }
    ~SectionTemplate() { }
    void onExit() { onExit_(); }

    uint8_t getSize() override { return size_; }
    AbstractMenuItem** getItems() override { return items_.items; }
    SecOnExitCtx* getOnExitContext() override {
        return new SecOnExitCtx{NULL, new ONEXIT(onExit_), SEC};
    }
};

#endif /* MENU_SECTION_H */
