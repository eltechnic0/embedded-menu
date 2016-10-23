/*
 * File:   main.cpp
 * Author: German Gambon
 *
 * Created on 9 de abril de 2016, 17:00
 */

#include <cstdlib>
#include <iostream>
// #include "test_sections.h"
#include "menu_controller.h"
#include "menu_item.h"
#include "test_menu.h"

using namespace std;

// void simpleTest() {
// 	char buf[64];
// 	TempMenuItem temp;
//     TestSection section;
//     AbstractMenuItem** iterator = section.getItems();

//     cout << "Info strings:" << endl;
//     for (size_t i = 0; i < section.getSize(); i++) {
//         AbstractMenuItem* item = iterator[i];
//         item->getInfoString(buf, sizeof (buf));
//         cout << buf << endl;
//         if (!item->isSection()) {
//             // ((Int32MenuItem*)item)->onEndEdit();
// 			EndpointMenuItem* epitem = (EndpointMenuItem*)item;
// 			temp.startEdit(epitem);
// 			epitem->changeValue(0, 1);
// 			temp.endEdit(true);
// 			epitem->getValueAsString(buf, sizeof(buf));
// 			cout << "Value is: " << buf << endl;
// 		}
//     }
// }

// TODO: improve function by showing the cursor on the selected digit when in edit mode
void drawSection(MenuController& controller) {
	// controller.onPreDraw();
	const char* separator = "------------------------------";
	char buf[64];

	uint8_t size;
	auto nav_ctrl = controller.getNavCtrl();
	auto items = nav_ctrl.getVisible(&size);

	cout << "Page: " << (int) nav_ctrl.getPosition() << " / " << (int) nav_ctrl.getPages() << endl;

	cout << separator << endl;

	for (int i = 0; i < size; i++) {
		const AbstractMenuItem* item = items[i];
		item->getInfoString(buf, sizeof(buf));
		if (item == controller.getCurrentItem()
				&& controller.getStateInfo().getState() == StateInfo::Mode::NAVIGATE)
			cout << "> " << buf;
		else
			cout << buf;
		if (!item->isSection()) {
			const EndpointMenuItem* epitem = (const EndpointMenuItem*)item;
			epitem->getValueAsString(buf, sizeof(buf));
			if (controller.getStateInfo().getState() == StateInfo::Mode::EDIT
					&& item == controller.getCurrentItem())
				cout << "\t\t" << "> " << buf << endl;
			else
				cout << "\t\t" << buf << endl;
		} else
			cout << endl;
	}

	cout << separator << endl;

	// controller.onPostDraw();
}

void automaticTest() {
	TestMenu testMenu;
	MenuController controller(testMenu);
	drawSection(controller);

	cout << "Move down" << endl;
	controller.down();
	drawSection(controller);

	cout << "Move down" << endl;
	controller.down();
	drawSection(controller);

	cout << "Move in" << endl;
	controller.enter();
	drawSection(controller);

	cout << "Move up" << endl;
	controller.up();
	drawSection(controller);

	cout << "Move down" << endl;
	controller.down();
	drawSection(controller);

	cout << "Change" << endl;
	controller.enter();
	controller.down();
	drawSection(controller);

	cout << "Cancel" << endl;
	controller.escape();
	drawSection(controller);

	cout << "Move out" << endl;
	controller.escape();
	drawSection(controller);
}

void interactiveTest() {
	TestMenu testMenu;
	MenuController controller(testMenu);
	char key;
	bool running = true;

	do {
		cout << "Controls - wasd: movement, q: escape, e: enter, z: quit" << endl;
		drawSection(controller);
		cin >> key;
		switch (key) {
		case 'w':
			controller.up();
			break;
		case 'a':
			controller.left();
			break;
		case 's':
			controller.down();
			break;
		case 'd':
			controller.right();
			break;
		case 'q':
			controller.escape();
			break;
		case 'e':
			controller.enter();
			break;
		case 'z':
			running = false;
			break;
		}
	} while(running);
}

int main(int argc, char** argv) {
	// simpleTest();
	// automaticTest();
	interactiveTest();

    return 0;
}
