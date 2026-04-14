/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            lineedittest.cc
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
#include <dggui/lineedit.h>
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

class TestLineEdit : public dggui::LineEdit
{
public:
	explicit TestLineEdit(dggui::Widget* parent) : dggui::LineEdit(parent)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}
};

TEST_CASE("LineEditTest")
{
	SUBCASE("testSetText")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");
		CHECK_EQ(edit.getText(), "Hello");
	}

	SUBCASE("testSetTextEmpty")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("");
		CHECK_EQ(edit.getText(), "");
	}

	SUBCASE("testReadOnly")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setReadOnly(true);
		CHECK_UNARY(edit.readOnly());

		edit.setReadOnly(false);
		CHECK_UNARY(!edit.readOnly());
	}

	SUBCASE("testKeyEventLeft")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::left;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventLeftAtStart")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::left;
		event.text = "";

		edit.keyEvent(&event);
		edit.keyEvent(&event);
		edit.keyEvent(&event);
		edit.keyEvent(&event);
		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventRight")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::right;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventRightAtEnd")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::right;
		event.text = "";

		for(int i = 0; i < 10; ++i)
		{
			edit.keyEvent(&event);
		}
	}

	SUBCASE("testKeyEventHome")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::home;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventEnd")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::end;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventDeleteKey")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::deleteKey;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventDeleteKeyAtEnd")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::deleteKey;
		event.text = "";

		for(int i = 0; i < 10; ++i)
		{
			edit.keyEvent(&event);
		}
	}

	SUBCASE("testKeyEventBackspace")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::backspace;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventBackspaceAtStart")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::backspace;
		event.text = "";

		for(int i = 0; i < 10; ++i)
		{
			edit.keyEvent(&event);
		}
	}

	SUBCASE("testKeyEventCharacter")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::character;
		event.text = "X";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventEnter")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::enter;
		event.text = "";

		edit.keyEvent(&event);
	}

	SUBCASE("testKeyEventReadOnly")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setReadOnly(true);
		edit.setText("Hello");

		dggui::KeyEvent event;
		event.direction = dggui::Direction::down;
		event.keycode = dggui::Key::character;
		event.text = "X";

		edit.keyEvent(&event);
	}

	SUBCASE("testButtonEvent")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		edit.buttonEvent(&event);
	}

	SUBCASE("testButtonEventNonLeft")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::right;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		edit.buttonEvent(&event);
	}

	SUBCASE("testRepaintEvent")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setText("Hello");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 30;

		edit.repaintEvent(&event);
	}

	SUBCASE("testRepaintEventZeroSize")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(0, 0);

		edit.setText("Hello");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 0;
		event.height = 0;

		edit.repaintEvent(&event);
	}

	SUBCASE("testRepaintEventReadOnly")
	{
		TestWindow window;
		TestLineEdit edit(&window);
		edit.resize(100, 30);

		edit.setReadOnly(true);
		edit.setText("Hello");

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 30;

		edit.repaintEvent(&event);
	}
}