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
