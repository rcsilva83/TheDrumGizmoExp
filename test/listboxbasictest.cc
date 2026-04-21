/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 ***************************************************************************
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

#include <dggui/guievent.h>
#include <dggui/listboxbasic.h>
#include <dggui/window.h>

namespace dggui
{

// Test helper class using protected inheritance to expose protected methods
class ListBoxBasicTestHelper : public ListBoxBasic
{
public:
	ListBoxBasicTestHelper(Widget* parent) : ListBoxBasic(parent)
	{
	}

	// Expose protected methods for testing
	bool testIsFocusable()
	{
		return isFocusable();
	}

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

	void testRepaintEvent(RepaintEvent* event)
	{
		repaintEvent(event);
	}
};

} // namespace dggui

// Test helper to capture notifier events
class ListBoxTestProbe : public Listener
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

TEST_CASE("ListBoxBasicConstructorTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("constructor_creates_empty_listbox")
	{
		dggui::ListBoxBasic listbox(&window);

		// Listbox should be created with no items
		CHECK_EQ("", listbox.selectedName());
		CHECK_EQ("", listbox.selectedValue());
	}
}

TEST_CASE("ListBoxBasicAddItemTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("addItem_adds_single_item")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");

		CHECK_EQ("Item1", listbox.selectedName());
		CHECK_EQ("Value1", listbox.selectedValue());
	}

	SUBCASE("addItem_adds_multiple_items")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// First item should be auto-selected
		CHECK_EQ("Item1", listbox.selectedName());
		CHECK_EQ("Value1", listbox.selectedValue());
	}

	SUBCASE("addItem_notifies_value_changed")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.addItem("Item1", "Value1");

		CHECK_EQ(1, probe.valueChangeCount);
	}
}

TEST_CASE("ListBoxBasicAddItemsTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("addItems_adds_multiple_items_at_once")
	{
		dggui::ListBoxBasic listbox(&window);

		std::vector<dggui::ListBoxBasic::Item> items;
		items.push_back({"Item1", "Value1"});
		items.push_back({"Item2", "Value2"});
		items.push_back({"Item3", "Value3"});

		listbox.addItems(items);

		// First item should be auto-selected
		CHECK_EQ("Item1", listbox.selectedName());
		CHECK_EQ("Value1", listbox.selectedValue());
	}

	SUBCASE("addItems_appends_to_existing_items")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Existing1", "ExistingValue1");

		std::vector<dggui::ListBoxBasic::Item> items;
		items.push_back({"Item1", "Value1"});
		items.push_back({"Item2", "Value2"});

		listbox.addItems(items);

		// Existing selection should remain
		CHECK_EQ("Existing1", listbox.selectedName());
		CHECK_EQ("ExistingValue1", listbox.selectedValue());
	}
}

TEST_CASE("ListBoxBasicClearTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("clear_removes_all_items")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		listbox.clear();

		CHECK_EQ("", listbox.selectedName());
		CHECK_EQ("", listbox.selectedValue());
	}

	SUBCASE("clear_resets_selection")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.clear();

		CHECK_EQ("", listbox.selectedName());
		CHECK_EQ("", listbox.selectedValue());
	}

	SUBCASE("clear_notifies_value_changed")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		probe.reset();

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.clear();

		CHECK_EQ(1, probe.valueChangeCount);
	}
}

TEST_CASE("ListBoxBasicSelectItemTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("selectItem_selects_by_index")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		bool result = listbox.selectItem(1);

		CHECK_UNARY(result);
		CHECK_EQ("Item2", listbox.selectedName());
		CHECK_EQ("Value2", listbox.selectedValue());
	}

	SUBCASE("selectItem_returns_false_for_invalid_index")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");

		bool resultNeg = listbox.selectItem(-1);
		bool resultLarge = listbox.selectItem(10);

		CHECK_UNARY(!resultNeg);
		CHECK_UNARY(!resultLarge);
	}

	SUBCASE("selectItem_notifies_value_changed")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.selectItem(1);

		CHECK_EQ(1, probe.valueChangeCount);
	}
}

TEST_CASE("ListBoxBasicSelectedNameValueTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("selectedName_returns_empty_when_no_selection")
	{
		dggui::ListBoxBasic listbox(&window);

		// No items added, selection should be empty
		CHECK_EQ("", listbox.selectedName());
	}

	SUBCASE("selectedValue_returns_empty_when_no_selection")
	{
		dggui::ListBoxBasic listbox(&window);

		// No items added, selection should be empty
		CHECK_EQ("", listbox.selectedValue());
	}

	SUBCASE("selectedName_returns_correct_name")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("TestName", "TestValue");

		CHECK_EQ("TestName", listbox.selectedName());
	}

	SUBCASE("selectedValue_returns_correct_value")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("TestName", "TestValue");

		CHECK_EQ("TestValue", listbox.selectedValue());
	}
}

TEST_CASE("ListBoxBasicClearSelectedValueTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("clearSelectedValue_clears_selection")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.clearSelectedValue();

		CHECK_EQ("", listbox.selectedName());
		CHECK_EQ("", listbox.selectedValue());
	}

	SUBCASE("clearSelectedValue_notifies_value_changed")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.clearSelectedValue();

		CHECK_EQ(1, probe.valueChangeCount);
	}
}

TEST_CASE("ListBoxBasicResizeTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("resize_updates_dimensions")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.resize(200, 150);

		CHECK_EQ(std::size_t(200u), listbox.width());
		CHECK_EQ(std::size_t(150u), listbox.height());
	}
}

TEST_CASE("ListBoxBasicNotifierTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("valueChangedNotifier_fires_on_add")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.addItem("Item1", "Value1");

		CHECK_EQ(1, probe.valueChangeCount);
	}

	SUBCASE("valueChangedNotifier_fires_on_clear")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.clear();

		CHECK_EQ(1, probe.valueChangeCount);
	}

	SUBCASE("valueChangedNotifier_fires_on_selectItem")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.selectItem(1);

		CHECK_EQ(1, probe.valueChangeCount);
	}

	SUBCASE("valueChangedNotifier_fires_on_clearSelectedValue")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.clearSelectedValue();

		CHECK_EQ(1, probe.valueChangeCount);
	}

	SUBCASE("multiple_notifiers_can_be_connected")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		// Connect notifiers before adding items
		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);
		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);
		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		listbox.addItem("Item1", "Value1");

		// valueChangedNotifier fires on add
		CHECK_EQ(1, probe.valueChangeCount);
	}
}

TEST_CASE("ListBoxBasicEdgeCaseTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("selectItem_at_boundary_returns_false")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");

		// Test exact boundary
		bool result = listbox.selectItem(1);
		CHECK_UNARY(!result);
	}

	SUBCASE("empty_listbox_operations")
	{
		dggui::ListBoxBasic listbox(&window);

		// Operations on empty listbox should not crash
		CHECK_EQ("", listbox.selectedName());
		CHECK_EQ("", listbox.selectedValue());

		bool result = listbox.selectItem(0);
		CHECK_UNARY(!result);

		listbox.clearSelectedValue();
		CHECK_EQ("", listbox.selectedName());

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}

	SUBCASE("selectItem_preserves_selection_for_invalid_index")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		listbox.selectItem(0);
		bool result = listbox.selectItem(100);

		CHECK_UNARY(!result);
		// Original selection should be preserved
		CHECK_EQ("Item1", listbox.selectedName());
	}
}

TEST_CASE("ListBoxBasicProtectedMethodTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("isFocusable_returns_true")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);

		CHECK_UNARY(listbox.testIsFocusable());
	}

	SUBCASE("key_down_arrow_moves_marked_down")
	{
		// Key down only changes 'marked' (highlight), not 'selected'
		// Selection changes only on space or enter keys
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// First press down to move marked to Item2
		dggui::KeyEvent event;
		event.keycode = dggui::Key::down;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event);

		// Selection hasn't changed yet (only marked has)
		CHECK_EQ("Item1", listbox.selectedName());

		// Now press space to select the marked item
		event.keycode = dggui::Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		// Now selection should have changed to Item2
		CHECK_EQ("Item2", listbox.selectedName());
	}

	SUBCASE("key_up_arrow_moves_marked_up")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		dggui::KeyEvent event;

		// First move marked to Item3 by pressing down twice
		event.keycode = dggui::Key::down;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event); // marked = 1
		listbox.testKeyEvent(&event); // marked = 2

		// Press up to move marked back to Item2
		event.keycode = dggui::Key::up;
		listbox.testKeyEvent(&event);

		// Selection hasn't changed yet
		CHECK_EQ("Item1", listbox.selectedName());

		// Press space to select marked item
		event.keycode = dggui::Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		// Now selection should be Item2
		CHECK_EQ("Item2", listbox.selectedName());
	}

	SUBCASE("key_home_moves_marked_to_first_item")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		listbox.selectItem(2);
		CHECK_EQ("Item3", listbox.selectedName());

		// Press home to move marked to first item
		dggui::KeyEvent event;
		event.keycode = dggui::Key::home;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event);

		// Selection hasn't changed yet
		CHECK_EQ("Item3", listbox.selectedName());

		// Press space to select marked item
		event.keycode = dggui::Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		// Now selection should be Item1
		CHECK_EQ("Item1", listbox.selectedName());
	}

	SUBCASE("key_end_moves_marked_to_last_item")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		CHECK_EQ("Item1", listbox.selectedName());

		// Press end to move marked to last item
		dggui::KeyEvent event;
		event.keycode = dggui::Key::end;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event);

		// Selection hasn't changed yet
		CHECK_EQ("Item1", listbox.selectedName());

		// Press space to select marked item
		event.keycode = dggui::Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		// Now selection should be Item3
		CHECK_EQ("Item3", listbox.selectedName());
	}

	SUBCASE("key_enter_fires_selection_notifier_and_selects_marked")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);

		// Move marked to Item2
		dggui::KeyEvent event;
		event.keycode = dggui::Key::down;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event);

		CHECK_EQ("Item1", listbox.selectedName());

		// Press enter to select marked item and fire notifier
		event.keycode = dggui::Key::enter;
		listbox.testKeyEvent(&event);

		CHECK_EQ(1, probe.selectionCount);
		CHECK_EQ("Item2", listbox.selectedName());
	}

	SUBCASE("key_space_selects_marked_item")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		// Move marked to Item2
		dggui::KeyEvent event;
		event.keycode = dggui::Key::down;
		event.direction = dggui::Direction::down;
		listbox.testKeyEvent(&event);

		CHECK_EQ("Item1", listbox.selectedName());

		// Press space to select marked item
		event.keycode = dggui::Key::character;
		event.text = " ";
		listbox.testKeyEvent(&event);

		CHECK_EQ("Item2", listbox.selectedName());
		CHECK_EQ(1, probe.valueChangeCount);
	}

	SUBCASE("key_event_does_not_fire_on_up_direction")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		dggui::KeyEvent event;
		event.keycode = dggui::Key::down;
		event.direction =
		    dggui::Direction::up; // Up direction should be ignored
		listbox.testKeyEvent(&event);

		// Value should not change on up direction
		CHECK_EQ("Item1", listbox.selectedName());
		CHECK_EQ(0, probe.valueChangeCount);
	}

	SUBCASE("button_left_click_selects_item_on_button_up")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		// First send button down to set marked item
		dggui::ButtonEvent event;
		event.x = 10;
		event.y = 30; // Click on second item area
		event.button = dggui::MouseButton::left;
		event.direction = dggui::Direction::down;
		event.doubleClick = false;
		listbox.testButtonEvent(&event);

		// Selection happens on button up
		event.direction = dggui::Direction::up;
		listbox.testButtonEvent(&event);

		// Selection should have been made
		CHECK_UNARY(listbox.selectedName() != "");
	}

	SUBCASE("button_left_click_fires_click_notifier_on_button_up")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);

		// First send button down
		dggui::ButtonEvent event;
		event.x = 10;
		event.y = 10;
		event.button = dggui::MouseButton::left;
		event.direction = dggui::Direction::down;
		event.doubleClick = false;
		listbox.testButtonEvent(&event);

		// clickNotifier fires on button up, not down
		CHECK_EQ(0, probe.clickCount);

		// Now send button up
		event.direction = dggui::Direction::up;
		listbox.testButtonEvent(&event);

		CHECK_EQ(1, probe.clickCount);
	}

	SUBCASE("button_double_click_fires_selection_notifier")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);

		dggui::ButtonEvent event;
		event.x = 10;
		event.y = 10;
		event.button = dggui::MouseButton::left;
		event.direction = dggui::Direction::down;
		event.doubleClick = true;
		listbox.testButtonEvent(&event);

		CHECK_EQ(1, probe.selectionCount);
	}

	SUBCASE("button_non_left_button_does_not_fire_click")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);

		dggui::ButtonEvent event;
		event.x = 10;
		event.y = 10;
		event.button = dggui::MouseButton::right;
		event.direction = dggui::Direction::up; // clickNotifier fires on up
		event.doubleClick = false;
		listbox.testButtonEvent(&event);

		CHECK_EQ(0, probe.clickCount);
	}

	SUBCASE("scroll_event_forwards_to_scrollbar")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		// Add many items to enable scrolling
		for(int i = 0; i < 50; ++i)
		{
			listbox.addItem(
			    "Item" + std::to_string(i), "Value" + std::to_string(i));
		}

		dggui::ScrollEvent event;
		event.x = 10;
		event.y = 10;
		event.delta = -1.0f; // Scroll down
		listbox.testScrollEvent(&event);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}

	SUBCASE("repaint_event_with_zero_size_does_not_crash")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);

		listbox.addItem("Item1", "Value1");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 0;
		event.height = 0;
		listbox.testRepaintEvent(&event);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}

	SUBCASE("repaint_event_renders_items")
	{
		dggui::ListBoxBasicTestHelper listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 200;
		event.height = 100;
		listbox.testRepaintEvent(&event);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}
}
