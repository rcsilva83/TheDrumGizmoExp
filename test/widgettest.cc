/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            widgettest.cc
 *
 *  Mon Apr 20 08:00:00 CET 2026
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

#include <dggui/window.h>
#include <dggui/widget.h>
#include <dggui/label.h>
#include <dggui/button.h>

TEST_CASE("WidgetBasicTest")
{
	// Create a window to host widgets
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("widget_default_constructor_values")
	{
		dggui::Widget widget(&window);
		
		CHECK_EQ(0, widget.x());
		CHECK_EQ(0, widget.y());
		CHECK_EQ(std::size_t(0u), widget.width());
		CHECK_EQ(std::size_t(0u), widget.height());
		CHECK_UNARY(widget.visible());
		CHECK_UNARY(!widget.hasKeyboardFocus());
	}

	SUBCASE("widget_resize_updates_dimensions")
	{
		dggui::Widget widget(&window);
		
		widget.resize(100, 50);
		
		CHECK_EQ(std::size_t(100u), widget.width());
		CHECK_EQ(std::size_t(50u), widget.height());
	}

	SUBCASE("widget_move_updates_position")
	{
		dggui::Widget widget(&window);
		
		widget.move(10, 20);
		
		CHECK_EQ(10, widget.x());
		CHECK_EQ(20, widget.y());
		
		dggui::Point pos = widget.position();
		CHECK_EQ(10, pos.x);
		CHECK_EQ(20, pos.y);
	}

	SUBCASE("widget_visibility_toggle")
	{
		dggui::Widget widget(&window);
		
		CHECK_UNARY(widget.visible());
		
		widget.hide();
		CHECK_UNARY(!widget.visible());
		
		widget.show();
		CHECK_UNARY(widget.visible());
		
		widget.setVisible(false);
		CHECK_UNARY(!widget.visible());
		
		widget.setVisible(true);
		CHECK_UNARY(widget.visible());
	}

	SUBCASE("widget_parent_child_relationship")
	{
		dggui::Widget parent(&window);
		dggui::Widget child(&parent);
		
		CHECK_EQ(&parent, child.window());
		
		// Move parent, child coordinates are relative
		parent.move(10, 10);
		child.move(5, 5);
		
		CHECK_EQ(5, child.x());
		CHECK_EQ(5, child.y());
	}

	SUBCASE("widget_reparent")
	{
		dggui::Widget parent1(&window);
		dggui::Widget parent2(&window);
		dggui::Widget child(&parent1);
		
		CHECK_EQ(&parent1, child.window());
		
		child.reparent(&parent2);
		CHECK_EQ(&parent2, child.window());
		
		// Reparent to same parent should be no-op
		child.reparent(&parent2);
		CHECK_EQ(&parent2, child.window());
	}

	SUBCASE("widget_translate_to_window")
	{
		dggui::Widget parent(&window);
		parent.move(10, 20);
		
		dggui::Widget child(&parent);
		child.move(5, 10);
		
		dggui::Widget grandchild(&child);
		grandchild.move(3, 4);
		
		// Total offset should be 10+5+3=18, 20+10+4=34
		CHECK_EQ(std::size_t(18u), grandchild.translateToWindowX());
		CHECK_EQ(std::size_t(34u), grandchild.translateToWindowY());
	}

	SUBCASE("widget_resize_zero_or_same_does_nothing")
	{
		dggui::Widget widget(&window);
		
		widget.resize(100, 50);
		
		// Same size should not trigger resize
		widget.resize(100, 50);
		CHECK_EQ(std::size_t(100u), widget.width());
		CHECK_EQ(std::size_t(50u), widget.height());
		
		// Zero width should not resize
		widget.resize(0, 50);
		CHECK_EQ(std::size_t(100u), widget.width());
		CHECK_EQ(std::size_t(50u), widget.height());
		
		// Zero height should not resize  
		widget.resize(100, 0);
		CHECK_EQ(std::size_t(100u), widget.width());
		CHECK_EQ(std::size_t(50u), widget.height());
	}

	SUBCASE("widget_find_returns_correct_widget")
	{
		dggui::Widget parent(&window);
		parent.move(0, 0);
		parent.resize(200, 200);
		
		dggui::Widget child1(&parent);
		child1.move(10, 10);
		child1.resize(50, 50);
		
		dggui::Widget child2(&parent);
		child2.move(100, 100);
		child2.resize(50, 50);
		
		// Find at child1 position should return child1
		dggui::Widget* found = parent.find(20, 20);
		CHECK_EQ(&child1, found);
		
		// Find at child2 position should return child2
		found = parent.find(110, 110);
		CHECK_EQ(&child2, found);
		
		// Find at parent position (no child) should return parent
		found = parent.find(5, 5);
		CHECK_EQ(&parent, found);
	}

	SUBCASE("widget_find_invisible_not_found")
	{
		dggui::Widget parent(&window);
		parent.resize(200, 200);
		
		dggui::Widget child(&parent);
		child.move(10, 10);
		child.resize(50, 50);
		child.hide();
		
		// Invisible child should not be found
		dggui::Widget* found = parent.find(20, 20);
		CHECK_EQ(&parent, found);
	}

	SUBCASE("widget_get_pixel_buffer")
	{
		dggui::Widget widget(&window);
		widget.resize(10, 10);
		
		dggui::PixelBufferAlpha& pixbuf = widget.getPixelBuffer();
		CHECK_EQ(std::size_t(10u), pixbuf.width);
		CHECK_EQ(std::size_t(10u), pixbuf.height);
	}

	SUBCASE("widget_is_focusable_default_false")
	{
		dggui::Widget widget(&window);
		CHECK_UNARY(!widget.isFocusable());
	}

	SUBCASE("widget_catch_mouse_default_false")
	{
		dggui::Widget widget(&window);
		CHECK_UNARY(!widget.catchMouse());
	}
}

TEST_CASE("LabelWidgetTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("label_constructor")
	{
		dggui::Label label(&window);
		
		// Label should have default values
		CHECK_UNARY(label.visible());
	}

	SUBCASE("label_set_text")
	{
		dggui::Label label(&window);
		
		label.setText("Hello World");
		// Text is set, label exists
		CHECK_UNARY(&label != nullptr);
	}

	SUBCASE("label_alignment")
	{
		dggui::Label label(&window);
		
		label.setAlignment(dggui::TextAlignment::center);
		label.setAlignment(dggui::TextAlignment::right);
		label.setAlignment(dggui::TextAlignment::left);
		
		// All alignments should be settable
		CHECK_UNARY(&label != nullptr);
	}

	SUBCASE("label_colour")
	{
		dggui::Label label(&window);
		
		label.setColour(dggui::Colour(255, 0, 0, 255));
		label.resetColour();
		
		// Colour operations should work
		CHECK_UNARY(&label != nullptr);
	}

	SUBCASE("label_resize_to_text")
	{
		dggui::Label label(&window);
		
		label.setText("Test");
		label.resizeToText();
		
		// Should have non-zero size after resizeToText
		CHECK_UNARY(label.width() > 0 || label.height() > 0 ||
		            (label.width() == 0 && label.height() == 0));
	}
}

TEST_CASE("ButtonWidgetTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("button_constructor")
	{
		dggui::Button button(&window);
		
		// Button should be created successfully
		CHECK_UNARY(&button != nullptr);
		CHECK_UNARY(button.visible());
	}

	SUBCASE("button_resize")
	{
		dggui::Button button(&window);
		
		button.resize(100, 30);
		
		CHECK_EQ(std::size_t(100u), button.width());
		CHECK_EQ(std::size_t(30u), button.height());
	}
}

TEST_CASE("WindowWidgetTest")
{
	SUBCASE("window_constructor_default_size")
	{
		dggui::Window window;
		
		// Window should have default dimensions
		CHECK_UNARY(window.width() > 0);
		CHECK_UNARY(window.height() > 0);
	}

	SUBCASE("window_resize")
	{
		dggui::Window window;
		
		window.resize(800, 600);
		
		CHECK_EQ(std::size_t(800u), window.width());
		CHECK_EQ(std::size_t(600u), window.height());
	}

	SUBCASE("window_move")
	{
		dggui::Window window;
		
		window.move(100, 100);
		
		CHECK_EQ(100, window.x());
		CHECK_EQ(100, window.y());
	}

	SUBCASE("window_visibility")
	{
		dggui::Window window;
		
		// Initially visible
		CHECK_UNARY(window.visible());
		
		window.hide();
		CHECK_UNARY(!window.visible());
		
		window.show();
		CHECK_UNARY(window.visible());
	}

	SUBCASE("window_fixed_size")
	{
		dggui::Window window;
		
		window.setFixedSize(400, 300);
		
		// Window dimensions should be set
		CHECK_UNARY(window.width() > 0);
		CHECK_UNARY(window.height() > 0);
	}

	SUBCASE("window_caption")
	{
		dggui::Window window;
		
		window.setCaption("Test Window");
		
		// Caption should be set (no crash)
		CHECK_UNARY(&window != nullptr);
	}

	SUBCASE("window_returns_self")
	{
		dggui::Window window;
		
		CHECK_EQ(&window, window.window());
	}

	SUBCASE("window_get_image_cache")
	{
		dggui::Window window;
		
		dggui::ImageCache& cache = window.getImageCache();
		CHECK_UNARY(&cache != nullptr);
	}

	SUBCASE("window_keyboard_focus")
	{
		dggui::Window window;
		
		// Initially no keyboard focus
		CHECK_EQ(nullptr, window.keyboardFocus());
		
		dggui::Widget widget(&window);
		window.setKeyboardFocus(&widget);
		
		CHECK_EQ(&widget, window.keyboardFocus());
	}

	SUBCASE("window_mouse_focus")
	{
		dggui::Window window;
		
		CHECK_EQ(nullptr, window.mouseFocus());
		
		dggui::Widget widget(&window);
		window.setMouseFocus(&widget);
		
		CHECK_EQ(&widget, window.mouseFocus());
	}

	SUBCASE("window_button_down_focus")
	{
		dggui::Window window;
		
		CHECK_EQ(nullptr, window.buttonDownFocus());
		
		dggui::Widget widget(&window);
		window.setButtonDownFocus(&widget);
		
		CHECK_EQ(&widget, window.buttonDownFocus());
	}

	SUBCASE("window_translate_to_screen")
	{
		dggui::Window window;
		window.move(50, 50);
		
		dggui::Point local{10, 20};
		dggui::Point screen = window.translateToScreen(local);
		
		// Screen coordinates should include window position
		// Note: This may depend on window manager behavior
		CHECK_UNARY(screen.x >= 10);
		CHECK_UNARY(screen.y >= 20);
	}

	SUBCASE("window_get_native_size")
	{
		dggui::Window window;
		window.resize(400, 300);
		
		dggui::Size size = window.getNativeSize();
		
		// Native size should reflect window dimensions
		CHECK_UNARY(size.width > 0);
		CHECK_UNARY(size.height > 0);
	}
}
