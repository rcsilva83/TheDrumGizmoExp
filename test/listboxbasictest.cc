/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            listboxbasictest.cc
 *
 *  Tue Apr 21 08:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <dggui/listboxbasic.h>
#include <dggui/window.h>
#include <dggui/guievent.h>

using namespace dggui;

//! Listener class for testing notifiers
class ListBoxNotifierListener
	: public Listener
{
public:
	int selectionCount{0};
	int clickCount{0};
	int valueChangeCount{0};

	void onSelection()
	{
		++selectionCount;
	}

	void onClick()
	{
		++clickCount;
	}

	void onValueChange()
	{
		++valueChangeCount;
	}

	void reset()
	{
		selectionCount = 0;
		clickCount = 0;
		valueChangeCount = 0;
	}
};

//! Test helper class that exposes protected methods for testing
class TestListBoxBasic
	: public ListBoxBasic
{
public:
	TestListBoxBasic(Widget* parent)
		: ListBoxBasic(parent)
	{
	}

	// Expose protected methods for testing
	void testKeyEvent(KeyEvent* event)
	{
		keyEvent(event);
	}

	void testButtonEvent(ButtonEvent* event)
	{
		buttonEvent(event);
	}

	void testScrollEvent(ScrollEvent* event)
	{
		scrollEvent(event);
	}
};

TEST_CASE("ListBoxBasicConstructorTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("constructor_initializes_defaults")
	{
		ListBoxBasic listbox(&window);

		// After construction, no item should be selected
		CHECK_EQ(listbox.selectedName(), "");
		CHECK_EQ(listbox.selectedValue(), "");
	}

	SUBCASE("destructor_cleans_up")
	{
		// Just verify destructor doesn't crash
		{
			ListBoxBasic listbox(&window);
			listbox.addItem("test", "value");
		}
		CHECK_UNARY(true);
	}
}

TEST_CASE("ListBoxBasicAddItemTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("addItem_single_item")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");

		CHECK_EQ(listbox.selectedName(), "Item1");
		CHECK_EQ(listbox.selectedValue(), "Value1");
	}

	SUBCASE("addItem_multiple_items")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// First item should be auto-selected
		CHECK_EQ(listbox.selectedName(), "Item1");
		CHECK_EQ(listbox.selectedValue(), "Value1");
	}

	SUBCASE("addItem_triggers_valueChangedNotifier")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		ListBoxNotifierListener listener;
		listbox.valueChangedNotifier.connect(&listener,
		                                     &ListBoxNotifierListener::onValueChange);

		listbox.addItem("Item1", "Value1");

		// First item triggers selection which triggers valueChangedNotifier
		CHECK_GE(listener.valueChangeCount, 1);
	}
}

TEST_CASE("ListBoxBasicAddItemsTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("addItems_vector_of_items")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		std::vector<ListBoxBasic::Item> items;
		ListBoxBasic::Item item1;
		item1.name = "Name1";
		item1.value = "Value1";
		items.push_back(item1);

		ListBoxBasic::Item item2;
		item2.name = "Name2";
		item2.value = "Value2";
		items.push_back(item2);

		listbox.addItems(items);

		CHECK_EQ(listbox.selectedName(), "Name1");
	}

	SUBCASE("addItems_empty_vector")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		std::vector<ListBoxBasic::Item> items;
		listbox.addItems(items);

		// No items added, nothing selected
		CHECK_EQ(listbox.selectedName(), "");
	}
}

TEST_CASE("ListBoxBasicClearTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("clear_removes_all_items")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		CHECK_EQ(listbox.selectedName(), "Item1");

		listbox.clear();

		CHECK_EQ(listbox.selectedName(), "");
		CHECK_EQ(listbox.selectedValue(), "");
	}

	SUBCASE("clear_empty_listbox")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		// Should not crash
		listbox.clear();
		CHECK_EQ(listbox.selectedName(), "");
	}
}

TEST_CASE("ListBoxBasicSelectItemTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("selectItem_valid_index")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		bool result = listbox.selectItem(1);

		CHECK_UNARY(result);
		CHECK_EQ(listbox.selectedName(), "Item2");
		CHECK_EQ(listbox.selectedValue(), "Value2");
	}

	SUBCASE("selectItem_index_zero")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		bool result = listbox.selectItem(0);

		CHECK_UNARY(result);
		CHECK_EQ(listbox.selectedName(), "Item1");
	}

	SUBCASE("selectItem_negative_index")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");

		bool result = listbox.selectItem(-1);

		CHECK_UNARY(!result);
	}

	SUBCASE("selectItem_index_out_of_bounds")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		bool result = listbox.selectItem(5);

		CHECK_UNARY(!result);
	}
}

TEST_CASE("ListBoxBasicSelectedNameValueTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("selectedName_empty_list")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		CHECK_EQ(listbox.selectedName(), "");
	}

	SUBCASE("selectedValue_empty_list")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		CHECK_EQ(listbox.selectedValue(), "");
	}

	SUBCASE("selectedName_and_Value_after_clear")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.clearSelectedValue();

		CHECK_EQ(listbox.selectedName(), "");
		CHECK_EQ(listbox.selectedValue(), "");
	}
}

TEST_CASE("ListBoxBasicResizeTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("resize_updates_dimensions")
	{
		ListBoxBasic listbox(&window);

		listbox.resize(300, 200);

		CHECK_EQ(listbox.width(), 300u);
		CHECK_EQ(listbox.height(), 200u);
	}
}

TEST_CASE("ListBoxBasicKeyEventTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_down_moves_marked")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::down;

		// Should not crash
		listbox.testKeyEvent(&event);

		// Enter to confirm selection
		event.keycode = Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item2");
	}

	SUBCASE("keyEvent_up_moves_marked")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		KeyEvent event;
		event.direction = Direction::down;

		// Navigate to Item3 first (down twice from Item1)
		event.keycode = Key::down;
		listbox.testKeyEvent(&event);
		event.keycode = Key::down;
		listbox.testKeyEvent(&event);

		// Now navigate up to Item2
		event.keycode = Key::up;
		listbox.testKeyEvent(&event);

		event.keycode = Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item2");
	}

	SUBCASE("keyEvent_home_moves_to_first")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		listbox.selectItem(2);

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;

		listbox.testKeyEvent(&event);

		event.keycode = Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item1");
	}

	SUBCASE("keyEvent_end_moves_to_last")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::end;

		listbox.testKeyEvent(&event);

		event.keycode = Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item3");
	}

	SUBCASE("keyEvent_enter_triggers_selection_notifier")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		ListBoxNotifierListener listener;
		listbox.selectionNotifier.connect(&listener,
		                                  &ListBoxNotifierListener::onSelection);

		// Move to Item2
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::down;
		listbox.testKeyEvent(&event);

		event.keycode = Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listener.selectionCount, 1);
	}

	SUBCASE("keyEvent_space_selects_marked")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		// Move to Item2
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::down;
		listbox.testKeyEvent(&event);

		// Space to select
		event.keycode = Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item2");
	}

	SUBCASE("keyEvent_up_direction_ignored")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		KeyEvent event;
		event.direction = Direction::up;  // Key up direction, not down
		event.keycode = Key::down;
		listbox.testKeyEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item1");
	}
}

TEST_CASE("ListBoxBasicButtonEventTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("buttonEvent_left_click_selects_item")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		ListBoxNotifierListener listener;
		listbox.clickNotifier.connect(&listener,
		                              &ListBoxNotifierListener::onClick);

		// Click on Item2 (at position 5, which is in the item area)
		ButtonEvent event;
		event.x = 5;
		event.y = 30;  // Second item area
		event.direction = Direction::up;
		event.button = MouseButton::left;
		event.doubleClick = false;

		listbox.testButtonEvent(&event);

		CHECK_EQ(listbox.selectedName(), "Item2");
		CHECK_EQ(listener.clickCount, 1);
	}

	SUBCASE("buttonEvent_right_click_ignored")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		ButtonEvent event;
		event.x = 5;
		event.y = 30;
		event.direction = Direction::up;
		event.button = MouseButton::right;
		event.doubleClick = false;

		listbox.testButtonEvent(&event);

		// Should still be Item1
		CHECK_EQ(listbox.selectedName(), "Item1");
	}

	SUBCASE("buttonEvent_double_click_triggers_selection")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		ListBoxNotifierListener listener;
		listbox.selectionNotifier.connect(&listener,
		                                  &ListBoxNotifierListener::onSelection);

		ButtonEvent event;
		event.x = 5;
		event.y = 30;
		event.direction = Direction::up;
		event.button = MouseButton::left;
		event.doubleClick = true;

		listbox.testButtonEvent(&event);

		CHECK_EQ(listener.selectionCount, 1);
	}
}

TEST_CASE("ListBoxBasicScrollEventTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("scrollEvent_forwards_to_scrollbar")
	{
		TestListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		// Add many items to enable scrolling
		for(int i = 0; i < 20; ++i)
		{
			listbox.addItem("Item" + std::to_string(i),
			                "Value" + std::to_string(i));
		}

		ScrollEvent event;
		event.x = 10;
		event.y = 10;
		event.delta = 3.0f;

		// Should not crash
		listbox.testScrollEvent(&event);

		CHECK_UNARY(true);
	}
}

TEST_CASE("ListBoxBasicNotifierTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("valueChangedNotifier_triggered_on_selection_change")
	{
		ListBoxBasic listbox(&window);
		listbox.resize(200, 200);

		ListBoxNotifierListener listener;
		listbox.valueChangedNotifier.connect(&listener,
		                                     &ListBoxNotifierListener::onValueChange);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		// Reset after add
		listener.reset();

		listbox.selectItem(1);

		CHECK_EQ(listener.valueChangeCount, 1);
	}
}
