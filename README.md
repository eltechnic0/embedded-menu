# Embedded Menu

## Introduction

This repository contains a menu library in C++ that is originally intended for
use in 8-bit microcontrollers. It is basically a design exercise in OOP, C++
callbacks and C++ templates. I initially developed a similar version without
using templates for a project at my workplace. Then I felt it could be improved
to reduce code boilerplate when defining new sections, and refine the
inheritance structure and other details.

It is not really finished, much improvable for sure, and may have some bugs, but
I uploaded it anyway for reference mainly, and maybe to discuss improvements or
just talk about the current design.

It should be usable as of now too.

## Design

The structure of the menu system is the following:

On the one hand, there are a number of menu item types defined on `menu_item.h`.
These follow a pretty simple inheritance pattern where there is a pure virtual
base item `AbstractMenuItem` and two other classes are derived from it:
`SectionMenuItem` and `EndpointMenuItem`. The latter is another pure virtual
class for editable items. They only point to the variables containing those
values, they don't copy them to avoid data redundancy and the need to keep those
values updated if they are changed externally for some reason. However, since an
editing cancellation functionality is desired, there is a temporal item
`TempMenuItem` used during an edition operation, the value of which is copied
back to the original item if the value change is finally committed.

There are 4 types of editable menu items which are used in the definition of
menu sections:

- `IntegerMenuItem`: for items holding editable integer values: `uint8_t`,
`int32_t`, etc. The range of

- `DecimalMenuItem`: same as the previous but for floating point types.

- `BoolMenuItem`: for boolean types.

- `SelectionMenuItem`: for items displaying a text string for the limited amount
of values they can hold, much like a drop-down list in computer graphical
interfaces.

A menu is defined with a series of interleaved sections, which are classes
derived from `SectionTemplate`. This can be seen in `test_menu.h`. Then, the
sections are created by a subclass of the `MenuFactory` class, defined in
`menu_factory.h`. This is done with a creation function taking a section ID and
returning a new instance of the section requested.

Everything is controlled by the `MenuController`, defined in
`menu_controller.h`, which exposes a straightforward interface for navigating
through the menu, editing item values, committing or discarding changes, getting
information used by GUI manager to do its task, etc.

In order to decouple as much as possible the menu controller from the display
controller, there is a specific class `StateInfo` used for returning information
of the last menu action, which is useful to display error messages when the user
tries to perform an action that is forbidden, when the user navigates past the
end of the section back to the top item, or when a screen refresh is needed
after a change of section, for example.

For similar reasons, there is a class for controlling the way the items arrange
as the selection moves along the list of items. This functionality is very
coupled to that of the GUI so separating the completely is difficult. Currently
there is the class `MenuNavByPages`, which is intended to be used with a GUI
controller that displays a number of items in the screen until the cursor moves
past the top or bottom. A different type of menu movement could be a scrolling
mechanism, for instance. In order to accommodate for some flexibility that is
clearly needed, the idea of the current implementation is to provide a mechanism
to override some functions of the menu controller class with those of the chosen
navigation implementation, probably by means of C++ templates, and guaranteeing
that different navigation methods are easily interchangeable without unwanted
side effects. As of now however, this is unfinished and therefore still tightly
coupled, so the only navigation method available is by pages (and not well
tested yet).

## Testing the menu on the PC

The current test design, as defined in `main.cpp`, displays the name of the
node/item on the left column and the value it holds on the right, if any. If no
value is shown on the right, then that is a navigation item, i.e. an item
containing a subsection.

Navigation can be done with `wasd`, but once in edit mode, only accepting or
cancelling the change is allowed for exiting this mode.

The build process is done with a simple make file `Makefile`, so typing `make`
at the terminal should be enough. There are no external dependencies apart from
the standard C++ library. The code has been successfully compiled using g++
(GCC) 4.8.1.
