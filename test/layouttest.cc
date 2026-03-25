/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            layouttest.cc
 *
 *  Tue Mar 24 18:00:00 CET 2026
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

#include <dggui/layout.h>

// Minimal LayoutItem implementation that records position and size without
// requiring any GUI backend.
struct MockItem : public dggui::LayoutItem
{
	int _x{0};
	int _y{0};
	std::size_t _width{0};
	std::size_t _height{0};

	void resize(std::size_t w, std::size_t h) override
	{
		_width = w;
		_height = h;
	}

	void move(int x, int y) override
	{
		_x = x;
		_y = y;
	}

	int x() const override
	{
		return _x;
	}
	int y() const override
	{
		return _y;
	}
	std::size_t width() const override
	{
		return _width;
	}
	std::size_t height() const override
	{
		return _height;
	}
};

// A parent MockItem whose size is controlled by the test and which accepts a
// layout.  Because it is not a dggui::Widget, the layout does NOT auto-
// connect to sizeChangeNotifier; resize just updates the stored values.
struct MockParent : public MockItem
{
};

TEST_CASE("VBoxLayoutTest")
{
	MockParent parent;
	parent.resize(200, 300);

	SUBCASE("items_are_stacked_vertically_no_resize")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);

		MockItem item1;
		item1.resize(50, 30);
		MockItem item2;
		item2.resize(60, 40);
		MockItem item3;
		item3.resize(40, 20);

		layout.addItem(&item1);
		layout.addItem(&item2);
		layout.addItem(&item3);

		// Items should be stacked vertically starting from y=0
		CHECK_EQ(0, item1.y());
		CHECK_EQ(30, item2.y());
		CHECK_EQ(70, item3.y());
	}

	SUBCASE("items_are_centered_horizontally_by_default")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);

		MockItem item;
		item.resize(50, 30);
		layout.addItem(&item);

		// center: x = (200/2) - (50/2) = 75
		CHECK_EQ(75, item.x());
	}

	SUBCASE("items_are_left_aligned")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setHAlignment(dggui::HAlignment::left);

		MockItem item;
		item.resize(50, 30);
		layout.addItem(&item);

		CHECK_EQ(0, item.x());
	}

	SUBCASE("items_are_right_aligned")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setHAlignment(dggui::HAlignment::right);

		MockItem item;
		item.resize(50, 30);
		layout.addItem(&item);

		// right: x = 200 - 50 = 150
		CHECK_EQ(150, item.x());
	}

	SUBCASE("spacing_between_items")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setSpacing(5);

		MockItem item1;
		item1.resize(50, 30);
		MockItem item2;
		item2.resize(50, 40);

		layout.addItem(&item1);
		layout.addItem(&item2);

		CHECK_EQ(0, item1.y());
		// item2.y = item1.height + spacing = 30 + 5 = 35
		CHECK_EQ(35, item2.y());
	}

	SUBCASE("resize_children_distributes_height_evenly")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(true);

		MockItem item1;
		MockItem item2;

		layout.addItem(&item1);
		layout.addItem(&item2);

		// 2 items, parent height 300, no spacing -> each gets 150
		CHECK_EQ(std::size_t(150u), item1.height());
		CHECK_EQ(std::size_t(150u), item2.height());
		CHECK_EQ(std::size_t(200u), item1.width());
		CHECK_EQ(std::size_t(200u), item2.width());
	}

	SUBCASE("remove_item_relayouts")
	{
		dggui::VBoxLayout layout(&parent);
		layout.setResizeChildren(false);

		MockItem item1;
		item1.resize(50, 30);
		MockItem item2;
		item2.resize(50, 40);
		MockItem item3;
		item3.resize(50, 20);

		layout.addItem(&item1);
		layout.addItem(&item2);
		layout.addItem(&item3);

		CHECK_EQ(0, item1.y());
		CHECK_EQ(30, item2.y());
		CHECK_EQ(70, item3.y());

		layout.removeItem(&item2);
		// After removal: item1 stays at 0, item3 moves to 30
		CHECK_EQ(0, item1.y());
		CHECK_EQ(30, item3.y());
	}
}

TEST_CASE("HBoxLayoutTest")
{
	MockParent parent;
	parent.resize(300, 100);

	SUBCASE("items_are_stacked_horizontally_no_resize")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(false);

		MockItem item1;
		item1.resize(40, 30);
		MockItem item2;
		item2.resize(50, 20);

		layout.addItem(&item1);
		layout.addItem(&item2);

		// x positions: item1 starts at 0, item2 starts at 40
		CHECK_EQ(0, item1.x());
		CHECK_EQ(40, item2.x());
	}

	SUBCASE("items_are_centered_vertically_by_default")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(false);

		MockItem item;
		item.resize(40, 30);
		layout.addItem(&item);

		// center: y = (100/2) - (30/2) = 35
		CHECK_EQ(35, item.y());
	}

	SUBCASE("items_aligned_top")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setVAlignment(dggui::VAlignment::top);

		MockItem item;
		item.resize(40, 30);
		layout.addItem(&item);

		CHECK_EQ(0, item.y());
	}

	SUBCASE("items_aligned_bottom")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setVAlignment(dggui::VAlignment::bottom);

		MockItem item;
		item.resize(40, 30);
		layout.addItem(&item);

		// bottom: y = 100 - 30 = 70
		CHECK_EQ(70, item.y());
	}

	SUBCASE("resize_children_distributes_width_evenly")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(true);

		MockItem item1;
		MockItem item2;
		MockItem item3;

		layout.addItem(&item1);
		layout.addItem(&item2);
		layout.addItem(&item3);

		// 3 items, parent width 300, no spacing -> each gets 100
		CHECK_EQ(std::size_t(100u), item1.width());
		CHECK_EQ(std::size_t(100u), item2.width());
		CHECK_EQ(std::size_t(100u), item3.width());
		CHECK_EQ(std::size_t(100u), item1.height());
	}

	SUBCASE("spacing_between_items")
	{
		dggui::HBoxLayout layout(&parent);
		layout.setResizeChildren(false);
		layout.setSpacing(10);

		MockItem item1;
		item1.resize(40, 30);
		MockItem item2;
		item2.resize(50, 20);

		layout.addItem(&item1);
		layout.addItem(&item2);

		// item2.x = item1.width + spacing = 40 + 10 = 50
		CHECK_EQ(0, item1.x());
		CHECK_EQ(50, item2.x());
	}
}

TEST_CASE("GridLayoutTest")
{
	MockParent parent;
	parent.resize(200, 100);

	SUBCASE("single_item_fills_cell")
	{
		dggui::GridLayout layout(&parent, 2, 2);
		layout.setResizeChildren(true);

		MockItem item;
		layout.addItem(&item);
		layout.setPosition(&item, {0, 1, 0, 1});
		layout.layout();

		// Cell size: 200/2 = 100 wide, 100/2 = 50 tall
		CHECK_EQ(std::size_t(100u), item.width());
		CHECK_EQ(std::size_t(50u), item.height());
		CHECK_EQ(0, item.x());
		CHECK_EQ(0, item.y());
	}

	SUBCASE("item_spanning_two_columns")
	{
		dggui::GridLayout layout(&parent, 2, 1);
		layout.setResizeChildren(true);

		MockItem item;
		layout.addItem(&item);
		layout.setPosition(&item, {0, 2, 0, 1}); // spans both columns
		layout.layout();

		// Spans 2 of 2 columns -> full width 200
		CHECK_EQ(std::size_t(200u), item.width());
		CHECK_EQ(std::size_t(100u), item.height());
	}

	SUBCASE("two_items_in_separate_columns")
	{
		dggui::GridLayout layout(&parent, 2, 1);
		layout.setResizeChildren(true);

		MockItem item1;
		MockItem item2;

		layout.addItem(&item1);
		layout.setPosition(&item1, {0, 1, 0, 1});
		layout.addItem(&item2);
		layout.setPosition(&item2, {1, 2, 0, 1});
		layout.layout();

		CHECK_EQ(std::size_t(100u), item1.width());
		CHECK_EQ(0, item1.x());

		CHECK_EQ(std::size_t(100u), item2.width());
		CHECK_EQ(100, item2.x());
	}

	SUBCASE("last_used_row_and_column")
	{
		dggui::GridLayout layout(&parent, 3, 3);

		MockItem item1;
		MockItem item2;

		layout.addItem(&item1);
		layout.setPosition(&item1, {0, 2, 0, 2}); // cols 0-1, rows 0-1
		layout.addItem(&item2);
		layout.setPosition(&item2, {1, 3, 1, 3}); // cols 1-2, rows 1-2

		// lastUsedRow for column 0: only item1 uses it (row_end-1 = 1)
		CHECK_EQ(1, layout.lastUsedRow(0));
		// lastUsedRow for column 2: only item2 uses it (row_end-1 = 2)
		CHECK_EQ(2, layout.lastUsedRow(2));
		// No item uses column 3 -> -1
		CHECK_EQ(-1, layout.lastUsedRow(3));

		// lastUsedColumn for row 0: only item1 uses it (column_end-1 = 1)
		CHECK_EQ(1, layout.lastUsedColumn(0));
		// lastUsedColumn for row 2: only item2 uses it (column_end-1 = 2)
		CHECK_EQ(2, layout.lastUsedColumn(2));
		// No item uses row 3 -> -1
		CHECK_EQ(-1, layout.lastUsedColumn(3));
	}

	SUBCASE("remove_item_clears_grid_range")
	{
		dggui::GridLayout layout(&parent, 2, 1);
		layout.setResizeChildren(true);

		MockItem item;
		layout.addItem(&item);
		layout.setPosition(&item, {0, 1, 0, 1});
		layout.layout();

		CHECK_EQ(std::size_t(100u), item.width());

		layout.removeItem(&item);

		// After removal, lastUsedRow/Column should return -1
		CHECK_EQ(-1, layout.lastUsedRow(0));
		CHECK_EQ(-1, layout.lastUsedColumn(0));
	}

	SUBCASE("spacing_shifts_item_position")
	{
		dggui::GridLayout layout(&parent, 2, 1);
		layout.setResizeChildren(true);
		layout.setSpacing(10);

		MockItem item1;
		MockItem item2;

		layout.addItem(&item1);
		layout.setPosition(&item1, {0, 1, 0, 1});
		layout.addItem(&item2);
		layout.setPosition(&item2, {1, 2, 0, 1});
		layout.layout();

		// available_width=200, spacing=10, 2 cols -> cell_width=(200-10)/2=95
		// item1: x=0, item2: x=95+10=105
		CHECK_EQ(std::size_t(95u), item1.width());
		CHECK_EQ(0, item1.x());
		CHECK_EQ(std::size_t(95u), item2.width());
		CHECK_EQ(105, item2.x());
	}
}
