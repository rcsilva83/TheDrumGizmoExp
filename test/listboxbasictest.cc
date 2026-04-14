/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            listboxbasictest.cc
 *
 *  Tue Apr 14 2026
 *  Copyright 2026 DrumGizmo Authors
 *  deva@aasimon.org
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

#include <dggui/canvas.h>
#include <dggui/listboxbasic.h>
#include <dggui/painter.h>
#include <dggui/window.h>

class TestWindow final : public dggui::Window
{
public:
	TestWindow() : dggui::Window(nullptr)
	{
	}

	void needsRedraw() override
	{
		needs_redraw = true;
	}
};

class TestableCanvas : public dggui::Canvas
{
public:
	TestableCanvas(std::size_t width, std::size_t height)
	    : pixbuf(width, height)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

private:
	dggui::PixelBufferAlpha pixbuf;
};

class TestListBoxBasic : public dggui::ListBoxBasic
{
public:
	explicit TestListBoxBasic(dggui::Widget* parent)
	    : dggui::ListBoxBasic(parent)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}
};

TEST_CASE("ListBoxBasicTest")
{
	SUBCASE("testAddItem")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		CHECK_EQ(listbox.selectedName(), "Item 1");
	}

	SUBCASE("testAddMultipleItems")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		std::vector<dggui::ListBoxBasic::Item> items;
		items.push_back({"Item 1", "value1"});
		items.push_back({"Item 2", "value2"});
		items.push_back({"Item 3", "value3"});
		listbox.addItems(items);

		CHECK_EQ(listbox.selectedName(), "Item 1");
	}

	SUBCASE("testClear")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.clear();

		CHECK_EQ(listbox.selectedName(), "");
	}

	SUBCASE("testSelectItem")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.addItem("Item 3", "value3");

		CHECK_UNARY(listbox.selectItem(1));
		CHECK_EQ(listbox.selectedName(), "Item 2");
	}

	SUBCASE("testSelectItemInvalidIndex")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		CHECK_UNARY(!listbox.selectItem(-1));
		CHECK_UNARY(!listbox.selectItem(10));
	}

	SUBCASE("testSelectedValue")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		CHECK_EQ(listbox.selectedValue(), "value1");
	}

	SUBCASE("testClearSelectedValue")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.clearSelectedValue();

		CHECK_EQ(listbox.selectedName(), "");
	}

	SUBCASE("testKeyEventUp")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.addItem("Item 3", "value3");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::up;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventUpAtMarkedZero")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::up;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventDown")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::down;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventDownAtLastItem")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::down;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventHome")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.addItem("Item 3", "value3");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::home;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventEnd")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::end;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventCharacterSpace")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::character;
		event.text = " ";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventEnter")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::enter;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testKeyEventUpDirection")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::up;
		event.keycode = dggui::Key::up;
		event.text = "";

		listbox.keyEvent(&event);
	}

	SUBCASE("testButtonEventScrollUp")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.addItem("Item 3", "value3");
		listbox.addItem("Item 4", "value4");
		listbox.addItem("Item 5", "value5");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::left;
		event.x = 90;
		event.y = 5;
		event.doubleClick = false;

		listbox.buttonEvent(&event);
	}

	SUBCASE("testButtonEventScrollDown")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");
		listbox.addItem("Item 3", "value3");
		listbox.addItem("Item 4", "value4");
		listbox.addItem("Item 5", "value5");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::left;
		event.x = 90;
		event.y = 95;
		event.doubleClick = false;

		listbox.buttonEvent(&event);
	}

	SUBCASE("testButtonEventItemClick")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::up;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 10;
		event.doubleClick = false;

		listbox.buttonEvent(&event);
	}

	SUBCASE("testButtonEventNonLeft")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::right;
		event.x = 50;
		event.y = 10;
		event.doubleClick = false;

		listbox.buttonEvent(&event);
	}

	SUBCASE("testButtonEventDoubleClick")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::up;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 10;
		event.doubleClick = true;

		listbox.buttonEvent(&event);
	}

	SUBCASE("testRepaintEvent")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 100;

		listbox.repaintEvent(&event);
	}

	SUBCASE("testRepaintEventZeroSize")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(0, 0);

		listbox.addItem("Item 1", "value1");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 0;
		event.height = 0;

		listbox.repaintEvent(&event);
	}

	SUBCASE("testScrollEvent")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.addItem("Item 1", "value1");
		listbox.addItem("Item 2", "value2");

		dggui::ScrollEvent event;
		event.x = 50;
		event.y = 50;
		event.delta = 0.1f;

		listbox.scrollEvent(&event);
	}

	SUBCASE("testResize")
	{
		TestWindow window;
		TestListBoxBasic listbox(&window);
		listbox.resize(100, 100);

		listbox.resize(200, 150);
		CHECK_EQ(listbox.width(), 200u);
		CHECK_EQ(listbox.height(), 150u);
	}
}