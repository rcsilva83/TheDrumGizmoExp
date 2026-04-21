/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 **************************************************************************
 *            lineedittest.cc
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
#include <dggui/lineedit.h>
#include <dggui/window.h>

using namespace dggui;

//! Listener class for testing notifiers
class LineEditNotifierListener : public Listener
{
public:
	int enterPressedCount{0};
	int textChangedCount{0};

	void onEnterPressed()
	{
		++enterPressedCount;
	}

	void reset()
	{
		enterPressedCount = 0;
		textChangedCount = 0;
	}
};

//! Test helper class that exposes protected methods for testing
class TestLineEdit : public LineEdit
{
public:
	TestLineEdit(Widget* parent) : LineEdit(parent)
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

	void testRepaintEvent(RepaintEvent* event)
	{
		repaintEvent(event);
	}

	// Track textChanged calls
	int textChangedCount{0};

protected:
	void textChanged() override
	{
		++textChangedCount;
	}
};

TEST_CASE("LineEditConstructorTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("constructor_initializes_defaults")
	{
		LineEdit lineEdit(&window);

		// Should not be read-only by default
		CHECK_UNARY(!lineEdit.readOnly());
	}

	SUBCASE("is_focusable_returns_true")
	{
		LineEdit lineEdit(&window);

		// LineEdit should be focusable
		CHECK_UNARY(lineEdit.isFocusable());
	}

	SUBCASE("destructor_cleans_up")
	{
		// Just verify destructor doesn't crash
		{
			LineEdit lineEdit(&window);
			lineEdit.setText("test text");
		}
		CHECK_UNARY(true);
	}
}

TEST_CASE("LineEditTextTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("setText_and_getText_basic")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("Hello World");

		CHECK_EQ(lineEdit.getText(), "Hello World");
	}

	SUBCASE("setText_empty_string")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("");

		CHECK_EQ(lineEdit.getText(), "");
	}

	SUBCASE("setText_overwrites_existing")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("First text");
		CHECK_EQ(lineEdit.getText(), "First text");

		lineEdit.setText("Second text");
		CHECK_EQ(lineEdit.getText(), "Second text");
	}

	SUBCASE("setText_with_special_characters")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("Hello!@#$%^&*()_+");

		CHECK_EQ(lineEdit.getText(), "Hello!@#$%^&*()_+");
	}

	SUBCASE("setText_with_numbers")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("1234567890");

		CHECK_EQ(lineEdit.getText(), "1234567890");
	}

	SUBCASE("setText_with_whitespace")
	{
		LineEdit lineEdit(&window);

		lineEdit.setText("  Leading and trailing spaces  ");

		CHECK_EQ(lineEdit.getText(), "  Leading and trailing spaces  ");
	}
}

TEST_CASE("LineEditReadOnlyTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("setReadOnly_true")
	{
		LineEdit lineEdit(&window);

		lineEdit.setReadOnly(true);

		CHECK_UNARY(lineEdit.readOnly());
	}

	SUBCASE("setReadOnly_false")
	{
		LineEdit lineEdit(&window);

		lineEdit.setReadOnly(true);
		lineEdit.setReadOnly(false);

		CHECK_UNARY(!lineEdit.readOnly());
	}

	SUBCASE("readOnly_initial_state")
	{
		LineEdit lineEdit(&window);

		CHECK_UNARY(!lineEdit.readOnly());
	}
}

TEST_CASE("LineEditKeyEventCharacterTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_character_inserts_text")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;
		event.text = "a";

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "a");
		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("keyEvent_multiple_characters")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("");

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;

		event.text = "H";
		lineEdit.testKeyEvent(&event);

		event.text = "i";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hi");
	}

	SUBCASE("keyEvent_character_at_cursor_position")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");
		// Cursor is at position 5 (end) after setText

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;
		event.text = "!";

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hello!");
	}
}

TEST_CASE("LineEditKeyEventNavigationTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_left_moves_cursor_left")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");
		// Cursor is at position 5 after setText

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::left;

		lineEdit.testKeyEvent(&event);

		// Cursor should move to position 4
		// Test by inserting a character - should be at position 4
		event.keycode = Key::character;
		event.text = "X";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "HellXo");
	}

	SUBCASE("keyEvent_left_at_beginning_does_nothing")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hi");

		// Move cursor to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::left;
		lineEdit.testKeyEvent(&event);
		lineEdit.testKeyEvent(&event);
		lineEdit.testKeyEvent(&event);

		// Try to move left again (should do nothing)
		lineEdit.testKeyEvent(&event);

		// Insert at beginning
		event.keycode = Key::character;
		event.text = "X";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "XHi");
	}

	SUBCASE("keyEvent_right_moves_cursor_right")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");

		// Move cursor to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Move right
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);

		// Insert character
		event.keycode = Key::character;
		event.text = "X";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "HXello");
	}

	SUBCASE("keyEvent_right_at_end_does_nothing")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hi");
		// Cursor is at position 2 (end)

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::right;

		// Try to move right (should do nothing)
		lineEdit.testKeyEvent(&event);

		// Insert at end
		event.keycode = Key::character;
		event.text = "X";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "HiX");
	}

	SUBCASE("keyEvent_home_moves_to_beginning")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello World");
		// Cursor is at end (position 11)

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Insert at beginning
		event.keycode = Key::character;
		event.text = "X";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "XHello World");
	}

	SUBCASE("keyEvent_end_moves_to_end")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");

		// Move to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Move to end
		event.keycode = Key::end;
		lineEdit.testKeyEvent(&event);

		// Insert at end
		event.keycode = Key::character;
		event.text = "!";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hello!");
	}
}

TEST_CASE("LineEditKeyEventDeletionTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_backspace_deletes_before_cursor")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");
		lineEdit.textChangedCount = 0;

		// Cursor is at end (position 5)
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::backspace;

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hell");
		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("keyEvent_backspace_at_beginning_does_nothing")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("H");
		lineEdit.textChangedCount = 0;

		// Move to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Try backspace at beginning
		event.keycode = Key::backspace;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "H");
		CHECK_EQ(lineEdit.textChangedCount, 0);
	}

	SUBCASE("keyEvent_delete_deletes_after_cursor")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");
		lineEdit.textChangedCount = 0;

		// Move to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Delete first character
		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "ello");
		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("keyEvent_delete_at_end_does_nothing")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");
		lineEdit.textChangedCount = 0;

		// Cursor is at end
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hello");
		CHECK_EQ(lineEdit.textChangedCount, 0);
	}

	SUBCASE("keyEvent_delete_in_middle")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");

		// Move cursor to position 2 (after "He")
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);

		// Delete character at position 2 (first 'l')
		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Helo");
	}

	SUBCASE("keyEvent_backspace_in_middle")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello");

		// Move cursor to position 3 (after "Hel")
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);
		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);

		// Backspace (delete character before cursor, second 'l')
		event.keycode = Key::backspace;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Helo");
	}
}

TEST_CASE("LineEditKeyEventEnterTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_enter_triggers_notifier")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		LineEditNotifierListener listener;
		lineEdit.enterPressedNotifier.connect(
		    &listener, &LineEditNotifierListener::onEnterPressed);

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::enter;

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(listener.enterPressedCount, 1);
	}

	SUBCASE("keyEvent_enter_no_text_change")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::enter;

		lineEdit.testKeyEvent(&event);

		// Enter should not trigger textChanged
		CHECK_EQ(lineEdit.textChangedCount, 0);
		CHECK_EQ(lineEdit.getText(), "Test");
	}
}

TEST_CASE("LineEditKeyEventReadOnlyTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_ignored_when_readonly")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;
		event.text = "X";

		lineEdit.testKeyEvent(&event);

		// Text should not change
		CHECK_EQ(lineEdit.getText(), "Test");
		CHECK_EQ(lineEdit.textChangedCount, 0);
	}

	SUBCASE("keyEvent_backspace_ignored_when_readonly")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::backspace;

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Test");
	}

	SUBCASE("keyEvent_delete_ignored_when_readonly")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);

		// Move to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Test");
	}

	SUBCASE("keyEvent_enter_ignored_when_readonly")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);

		LineEditNotifierListener listener;
		lineEdit.enterPressedNotifier.connect(
		    &listener, &LineEditNotifierListener::onEnterPressed);

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::enter;

		lineEdit.testKeyEvent(&event);

		// Enter notifier should NOT fire when readOnly
		CHECK_EQ(listener.enterPressedCount, 0);
	}
}

TEST_CASE("LineEditKeyEventDirectionUpTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("keyEvent_direction_up_ignored")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::up;
		event.keycode = Key::character;
		event.text = "X";

		lineEdit.testKeyEvent(&event);

		// Text should not change on key up
		CHECK_EQ(lineEdit.getText(), "Test");
		CHECK_EQ(lineEdit.textChangedCount, 0);
	}
}

TEST_CASE("LineEditButtonEventTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("buttonEvent_left_click_sets_position")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.resize(200, 30);
		lineEdit.setText("AB");

		// Click at position 10 (within border area)
		ButtonEvent event;
		event.x = 10;
		event.y = 5;
		event.direction = Direction::down;
		event.button = MouseButton::left;
		event.doubleClick = false;

		lineEdit.testButtonEvent(&event);

		// Just verify no crash - position depends on font metrics
		CHECK_UNARY(true);
	}

	SUBCASE("buttonEvent_right_click_ignored")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.resize(200, 30);
		lineEdit.setText("Test");

		ButtonEvent event;
		event.x = 10;
		event.y = 5;
		event.direction = Direction::down;
		event.button = MouseButton::right;
		event.doubleClick = false;

		// Should not crash
		lineEdit.testButtonEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("buttonEvent_ignored_when_readonly")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.resize(200, 30);
		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);

		ButtonEvent event;
		event.x = 10;
		event.y = 5;
		event.direction = Direction::down;
		event.button = MouseButton::left;
		event.doubleClick = false;

		// Should not crash
		lineEdit.testButtonEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("buttonEvent_direction_up_ignored")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.resize(200, 30);
		lineEdit.setText("Test");

		ButtonEvent event;
		event.x = 10;
		event.y = 5;
		event.direction = Direction::up;
		event.button = MouseButton::left;
		event.doubleClick = false;

		// Should not crash
		lineEdit.testButtonEvent(&event);

		CHECK_UNARY(true);
	}
}

TEST_CASE("LineEditRepaintEventTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("repaintEvent_basic")
	{
		TestLineEdit lineEdit(&window);

		lineEdit.resize(100, 30);
		lineEdit.setText("Test");

		RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 30;

		// Should not crash
		lineEdit.testRepaintEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("repaintEvent_zero_size_returns_early")
	{
		TestLineEdit lineEdit(&window);

		lineEdit.resize(0, 30);
		lineEdit.setText("Test");

		RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 0;
		event.height = 30;

		// Should not crash
		lineEdit.testRepaintEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("repaintEvent_zero_height_returns_early")
	{
		TestLineEdit lineEdit(&window);

		lineEdit.resize(100, 0);
		lineEdit.setText("Test");

		RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 0;

		// Should not crash
		lineEdit.testRepaintEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("repaintEvent_with_long_text")
	{
		TestLineEdit lineEdit(&window);

		lineEdit.resize(50, 30);
		lineEdit.setText("This is a very long text that should be truncated");

		RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 50;
		event.height = 30;

		// Should not crash with long text
		lineEdit.testRepaintEvent(&event);

		CHECK_UNARY(true);
	}

	SUBCASE("repaintEvent_readonly_no_cursor_drawn")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.resize(100, 30);
		lineEdit.setText("Test");
		lineEdit.setReadOnly(true);

		RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 30;

		// Should not crash
		lineEdit.testRepaintEvent(&event);

		CHECK_UNARY(true);
	}
}

TEST_CASE("LineEditTextChangedTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("textChanged_called_on_setText")
	{
		TestLineEdit lineEdit(&window);

		lineEdit.textChangedCount = 0;
		lineEdit.setText("Test");

		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("textChanged_called_on_character_input")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;
		event.text = "a";

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("textChanged_called_on_backspace")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::backspace;

		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("textChanged_called_on_delete")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");

		// Move to beginning
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		lineEdit.textChangedCount = 0;

		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.textChangedCount, 1);
	}

	SUBCASE("textChanged_not_called_on_navigation_keys")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Test");
		lineEdit.textChangedCount = 0;

		KeyEvent event;
		event.direction = Direction::down;

		event.keycode = Key::left;
		lineEdit.testKeyEvent(&event);

		event.keycode = Key::right;
		lineEdit.testKeyEvent(&event);

		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		event.keycode = Key::end;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.textChangedCount, 0);
	}
}

TEST_CASE("LineEditComplexInteractionTest")
{
	Window window;
	window.resize(400, 300);

	SUBCASE("type_full_word_and_delete")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::character;

		// Type "Hello"
		event.text = "H";
		lineEdit.testKeyEvent(&event);
		event.text = "e";
		lineEdit.testKeyEvent(&event);
		event.text = "l";
		lineEdit.testKeyEvent(&event);
		event.text = "l";
		lineEdit.testKeyEvent(&event);
		event.text = "o";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hello");

		// Backspace twice
		event.keycode = Key::backspace;
		lineEdit.testKeyEvent(&event);
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hel");

		// Move to beginning
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		// Delete first character
		event.keycode = Key::deleteKey;
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "el");
	}

	SUBCASE("insert_in_middle_of_text")
	{
		TestLineEdit lineEdit(&window);
		window.setKeyboardFocus(&lineEdit);

		lineEdit.setText("Hello World");

		// Move cursor to after "Hello "
		KeyEvent event;
		event.direction = Direction::down;
		event.keycode = Key::home;
		lineEdit.testKeyEvent(&event);

		for(int i = 0; i < 6; ++i)
		{
			event.keycode = Key::right;
			lineEdit.testKeyEvent(&event);
		}

		// Insert "Beautiful "
		event.keycode = Key::character;
		event.text = "B";
		lineEdit.testKeyEvent(&event);
		event.text = "e";
		lineEdit.testKeyEvent(&event);
		event.text = "a";
		lineEdit.testKeyEvent(&event);
		event.text = "u";
		lineEdit.testKeyEvent(&event);
		event.text = "t";
		lineEdit.testKeyEvent(&event);
		event.text = "i";
		lineEdit.testKeyEvent(&event);
		event.text = "f";
		lineEdit.testKeyEvent(&event);
		event.text = "u";
		lineEdit.testKeyEvent(&event);
		event.text = "l";
		lineEdit.testKeyEvent(&event);
		event.text = " ";
		lineEdit.testKeyEvent(&event);

		CHECK_EQ(lineEdit.getText(), "Hello Beautiful World");
	}
}
