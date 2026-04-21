/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 **************************************************************************
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

	SUBCASE("constructor_is_focusable")
	{
		dggui::ListBoxBasic listbox(&window);

		// ListBoxBasic should be focusable
		CHECK_UNARY(listbox.isFocusable());
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

TEST_CASE("ListBoxBasicKeyEventTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_up_arrow_moves_marked")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// First select item 1 (marked becomes 1)
		listbox.selectItem(1);

		// Press up
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::down;
		keyEvent.keycode = dggui::Key::up;
		listbox.keyEvent(&keyEvent);

		// Marked should now be 0, but selected stays at 1
		// We verify by checking that a subsequent space press selects item 0
		keyEvent.keycode = dggui::Key::character;
		keyEvent.text = " ";
		listbox.keyEvent(&keyEvent);

		CHECK_EQ("Item1", listbox.selectedName());
	}

	SUBCASE("keyEvent_down_arrow_moves_marked")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// Press down
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::down;
		keyEvent.keycode = dggui::Key::down;
		listbox.keyEvent(&keyEvent);

		// Space to select the marked item
		keyEvent.keycode = dggui::Key::character;
		keyEvent.text = " ";
		listbox.keyEvent(&keyEvent);

		CHECK_EQ("Item2", listbox.selectedName());
	}

	SUBCASE("keyEvent_home_moves_to_first")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// First move down
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::down;
		keyEvent.keycode = dggui::Key::down;
		listbox.keyEvent(&keyEvent);
		listbox.keyEvent(&keyEvent);

		// Press home
		keyEvent.keycode = dggui::Key::home;
		listbox.keyEvent(&keyEvent);

		// Space to select
		keyEvent.keycode = dggui::Key::character;
		keyEvent.text = " ";
		listbox.keyEvent(&keyEvent);

		CHECK_EQ("Item1", listbox.selectedName());
	}

	SUBCASE("keyEvent_end_moves_to_last")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");
		listbox.addItem("Item3", "Value3");

		// Press end
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::down;
		keyEvent.keycode = dggui::Key::end;
		listbox.keyEvent(&keyEvent);

		// Space to select
		keyEvent.keycode = dggui::Key::character;
		keyEvent.text = " ";
		listbox.keyEvent(&keyEvent);

		CHECK_EQ("Item3", listbox.selectedName());
	}

	SUBCASE("keyEvent_enter_triggers_selection_notifier")
	{
		dggui::ListBoxBasic listbox(&window);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);

		// Press enter
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::down;
		keyEvent.keycode = dggui::Key::enter;
		listbox.keyEvent(&keyEvent);

		CHECK_EQ(1, probe.selectionCount);
	}

	SUBCASE("keyEvent_ignores_up_direction")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		// Select second item first
		listbox.selectItem(1);

		// Try down with up direction (should be ignored)
		dggui::KeyEvent keyEvent;
		keyEvent.direction = dggui::Direction::up;
		keyEvent.keycode = dggui::Key::down;
		listbox.keyEvent(&keyEvent);

		// Selection should remain at Item2
		CHECK_EQ("Item2", listbox.selectedName());
	}
}

TEST_CASE("ListBoxBasicButtonEventTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("buttonEvent_left_click_selects_item")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		// Click on second item area
		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 30; // Below first item
		buttonEvent.doubleClick = false;

		listbox.buttonEvent(&buttonEvent);

		// Selection should change to Item2
		CHECK_EQ("Item2", listbox.selectedName());
	}

	SUBCASE("buttonEvent_triggers_click_notifier")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);

		// Click on item
		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = false;

		listbox.buttonEvent(&buttonEvent);

		CHECK_EQ(1, probe.clickCount);
	}

	SUBCASE("buttonEvent_double_click_triggers_selection_notifier")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);

		// Double-click on item
		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = true;

		listbox.buttonEvent(&buttonEvent);

		CHECK_EQ(1, probe.selectionCount);
	}

	SUBCASE("buttonEvent_ignores_non_left_buttons")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);

		// Right-click on item
		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::right;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = false;

		listbox.buttonEvent(&buttonEvent);

		// Click notifier should not be triggered
		CHECK_EQ(0, probe.clickCount);
		// Selection should not change
		CHECK_EQ("Item1", listbox.selectedName());
	}
}

TEST_CASE("ListBoxBasicScrollEventTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("scrollEvent_forwards_to_scrollbar")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		// Add many items to enable scrolling
		for(int i = 0; i < 20; ++i)
		{
			listbox.addItem(
			    "Item" + std::to_string(i), "Value" + std::to_string(i));
		}

		// Create scroll event
		dggui::ScrollEvent scrollEvent;
		scrollEvent.x = 50;
		scrollEvent.y = 50;
		scrollEvent.delta = 1.0f;

		// Should not crash
		listbox.scrollEvent(&scrollEvent);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}
}

TEST_CASE("ListBoxBasicRepaintEventTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("repaintEvent_handles_zero_size")
	{
		dggui::ListBoxBasic listbox(&window);

		listbox.addItem("Item1", "Value1");

		// Repaint event with zero size (widget not yet sized)
		dggui::RepaintEvent repaintEvent;
		repaintEvent.x = 0;
		repaintEvent.y = 0;
		repaintEvent.width = 0;
		repaintEvent.height = 0;

		// Should not crash
		listbox.repaintEvent(&repaintEvent);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}

	SUBCASE("repaintEvent_renders_items")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);

		listbox.addItem("Item1", "Value1");
		listbox.addItem("Item2", "Value2");

		dggui::RepaintEvent repaintEvent;
		repaintEvent.x = 0;
		repaintEvent.y = 0;
		repaintEvent.width = 200;
		repaintEvent.height = 100;

		// Should not crash
		listbox.repaintEvent(&repaintEvent);

		// Test passes if no crash occurs
		CHECK_UNARY(true);
	}
}

TEST_CASE("ListBoxBasicNotifierTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("selectionNotifier_fires_on_double_click")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);

		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = true;

		listbox.buttonEvent(&buttonEvent);

		CHECK_EQ(1, probe.selectionCount);
	}

	SUBCASE("clickNotifier_fires_on_click")
	{
		dggui::ListBoxBasic listbox(&window);
		listbox.resize(200, 100);
		ListBoxTestProbe probe;

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);

		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = false;

		listbox.buttonEvent(&buttonEvent);

		CHECK_EQ(1, probe.clickCount);
	}

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

		listbox.addItem("Item1", "Value1");

		CONNECT(&listbox, selectionNotifier, &probe,
		    &ListBoxTestProbe::onSelection);
		CONNECT(&listbox, clickNotifier, &probe, &ListBoxTestProbe::onClick);
		CONNECT(&listbox, valueChangedNotifier, &probe,
		    &ListBoxTestProbe::onValueChange);

		// Trigger click and selection
		dggui::ButtonEvent buttonEvent;
		buttonEvent.button = dggui::MouseButton::left;
		buttonEvent.direction = dggui::Direction::up;
		buttonEvent.x = 10;
		buttonEvent.y = 10;
		buttonEvent.doubleClick = true;

		listbox.buttonEvent(&buttonEvent);

		CHECK_EQ(1, probe.selectionCount);
		CHECK_EQ(1, probe.clickCount);
		// valueChangedNotifier fires on add, not on click
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
