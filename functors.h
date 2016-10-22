/*
 * File:   functors.h
 * Author: German Gambon
 *
 * Created on 12 de abril de 2016, 21:10
 */

#ifndef FUNCTORS_H
#define FUNCTORS_H

// forward declaration
class EndpointMenuItem;

//=============================================================================
// Functors
//=============================================================================

/*
 * Functors, or function objects, which are like C callbacks but can hold their
 * own context. Since the programmer knows at compile time to which item is the
 * functor intended to go, down-casting to access the item's full interface is
 * safe.
 */

class OnChangeF {
public:
    virtual ~OnChangeF() = default;
    virtual void operator()(EndpointMenuItem& item) = 0;
};

class OnStartEditF {
public:
    virtual ~OnStartEditF() = default;
    virtual void operator()(EndpointMenuItem& item) = 0;
};

class OnEndEditF {
public:
    virtual ~OnEndEditF() = default;
    virtual void operator()(EndpointMenuItem& item) = 0;
};

class OnEnterF {
public:
    virtual ~OnEnterF() = default;
    virtual void operator()() = 0;
};

class OnExitF {
public:
    virtual ~OnExitF() = default;
    virtual void operator()() = 0;
};

//=============================================================================
// NOP functors (no operation)
//=============================================================================

/**
 * Use this instance for convenience when no action is desired.
 */
class OnStartEditNOP : public OnStartEditF {
public:
    void operator()(EndpointMenuItem& item) { }
};

/**
 * Use this instance for convenience when no action is desired.
 */
class OnEndEditNOP : public OnEndEditF {
public:
    void operator()(EndpointMenuItem& item) { }
};

/**
 * Use this instance for convenience when no action is desired.
 */
class OnChangeNOP : public OnChangeF {
public:
    void operator()(EndpointMenuItem& item) { }
};

/**
 * The way the section code works, no instance of this class is needed.
 */
class OnExitNOP : public OnExitF {
public:
    void operator()() { }
};

#endif /* FUNCTORS_H */
