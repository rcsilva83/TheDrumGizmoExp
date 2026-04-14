/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            guieventtest.cc
 *
 *  Tue Apr 14 00:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <dggui/guievent.h>

TEST_CASE("EventType enum values")
{
	SUBCASE("all_event_types_are_distinct")
	{
		CHECK(dggui::EventType::mouseMove != dggui::EventType::repaint);
		CHECK(dggui::EventType::mouseMove != dggui::EventType::button);
		CHECK(dggui::EventType::button != dggui::EventType::scroll);
		CHECK(dggui::EventType::scroll != dggui::EventType::key);
		CHECK(dggui::EventType::key != dggui::EventType::close);
		CHECK(dggui::EventType::close != dggui::EventType::resize);
		CHECK(dggui::EventType::resize != dggui::EventType::move);
		CHECK(dggui::EventType::move != dggui::EventType::mouseEnter);
		CHECK(dggui::EventType::mouseEnter != dggui::EventType::mouseLeave);
	}
}

TEST_CASE("MouseMoveEvent")
{
	dggui::MouseMoveEvent event;
	event.x = 100;
	event.y = 200;
	CHECK_EQ(event.type(), dggui::EventType::mouseMove);
	CHECK_EQ(event.x, 100);
	CHECK_EQ(event.y, 200);
}

TEST_CASE("ButtonEvent")
{
	dggui::ButtonEvent event;
	event.x = 50;
	event.y = 75;
	event.direction = dggui::Direction::down;
	event.button = dggui::MouseButton::left;
	event.doubleClick = false;
	CHECK_EQ(event.type(), dggui::EventType::button);
	CHECK_EQ(event.x, 50);
	CHECK_EQ(event.y, 75);
	CHECK_EQ(event.direction, dggui::Direction::down);
	CHECK_EQ(event.button, dggui::MouseButton::left);
	CHECK_FALSE(event.doubleClick);

	SUBCASE("double_click_flag")
	{
		dggui::ButtonEvent dbl;
		dbl.doubleClick = true;
		CHECK(dbl.doubleClick);
	}

	SUBCASE("direction_up")
	{
		dggui::ButtonEvent up;
		up.direction = dggui::Direction::up;
		CHECK_EQ(up.direction, dggui::Direction::up);
	}

	SUBCASE("mouse_buttons")
	{
		CHECK(dggui::MouseButton::left != dggui::MouseButton::right);
		CHECK(dggui::MouseButton::left != dggui::MouseButton::middle);
		CHECK(dggui::MouseButton::middle != dggui::MouseButton::right);
	}
}

TEST_CASE("ScrollEvent")
{
	dggui::ScrollEvent event;
	event.x = 30;
	event.y = 40;
	event.delta = 3.5f;
	CHECK_EQ(event.type(), dggui::EventType::scroll);
	CHECK_EQ(event.x, 30);
	CHECK_EQ(event.y, 40);
	CHECK_EQ(event.delta, 3.5f);
}

TEST_CASE("RepaintEvent")
{
	dggui::RepaintEvent event;
	event.x = 5;
	event.y = 10;
	event.width = 100;
	event.height = 200;
	CHECK_EQ(event.type(), dggui::EventType::repaint);
	CHECK_EQ(event.width, std::size_t(100));
	CHECK_EQ(event.height, std::size_t(200));
}

TEST_CASE("KeyEvent")
{
	dggui::KeyEvent event;
	event.direction = dggui::Direction::down;
	event.keycode = dggui::Key::left;
	event.text = "";
	CHECK_EQ(event.type(), dggui::EventType::key);
	CHECK_EQ(event.keycode, dggui::Key::left);

	SUBCASE("key_enum_values")
	{
		CHECK(dggui::Key::unknown != dggui::Key::left);
		CHECK(dggui::Key::left != dggui::Key::right);
		CHECK(dggui::Key::right != dggui::Key::up);
		CHECK(dggui::Key::up != dggui::Key::down);
		CHECK(dggui::Key::down != dggui::Key::deleteKey);
		CHECK(dggui::Key::deleteKey != dggui::Key::backspace);
		CHECK(dggui::Key::backspace != dggui::Key::home);
		CHECK(dggui::Key::home != dggui::Key::end);
		CHECK(dggui::Key::end != dggui::Key::pageDown);
		CHECK(dggui::Key::pageDown != dggui::Key::pageUp);
		CHECK(dggui::Key::pageUp != dggui::Key::enter);
		CHECK(dggui::Key::enter != dggui::Key::character);
	}

	SUBCASE("key_with_text")
	{
		dggui::KeyEvent char_event;
		char_event.keycode = dggui::Key::character;
		char_event.text = "a";
		CHECK_EQ(char_event.keycode, dggui::Key::character);
		CHECK_EQ(char_event.text, std::string("a"));
	}
}

TEST_CASE("CloseEvent")
{
	dggui::CloseEvent event;
	CHECK_EQ(event.type(), dggui::EventType::close);
}

TEST_CASE("ResizeEvent")
{
	dggui::ResizeEvent event;
	event.width = 800;
	event.height = 600;
	CHECK_EQ(event.type(), dggui::EventType::resize);
	CHECK_EQ(event.width, std::size_t(800));
	CHECK_EQ(event.height, std::size_t(600));
}

TEST_CASE("MoveEvent")
{
	dggui::MoveEvent event;
	event.x = 10;
	event.y = 20;
	CHECK_EQ(event.type(), dggui::EventType::move);
	CHECK_EQ(event.x, 10);
	CHECK_EQ(event.y, 20);
}

TEST_CASE("MouseEnterEvent")
{
	dggui::MouseEnterEvent event;
	event.x = 15;
	event.y = 25;
	CHECK_EQ(event.type(), dggui::EventType::mouseEnter);
	CHECK_EQ(event.x, 15);
	CHECK_EQ(event.y, 25);
}

TEST_CASE("MouseLeaveEvent")
{
	dggui::MouseLeaveEvent event;
	event.x = 0;
	event.y = 0;
	CHECK_EQ(event.type(), dggui::EventType::mouseLeave);
}

TEST_CASE("Rect empty")
{
	SUBCASE("default_constructed_is_empty")
	{
		dggui::Rect r{};
		CHECK(r.empty());
	}

	SUBCASE("same_corners_is_empty")
	{
		dggui::Rect r{5, 5, 5, 5};
		CHECK(r.empty());
	}

	SUBCASE("nonzero_area_is_not_empty")
	{
		dggui::Rect r{0, 0, 10, 20};
		CHECK_FALSE(r.empty());
	}

	SUBCASE("single_pixel_is_not_empty")
	{
		dggui::Rect r{0, 0, 1, 1};
		CHECK_FALSE(r.empty());
	}
}

TEST_CASE("EventQueue is a list of shared_ptr<Event>")
{
	dggui::EventQueue queue;
	CHECK(queue.empty());

	auto event = std::make_shared<dggui::MouseMoveEvent>();
	queue.push_back(event);
	CHECK_EQ(queue.size(), std::size_t(1));

	auto btn = std::make_shared<dggui::ButtonEvent>();
	queue.push_back(btn);
	CHECK_EQ(queue.size(), std::size_t(2));

	CHECK_EQ(queue.front()->type(), dggui::EventType::mouseMove);
	CHECK_EQ(queue.back()->type(), dggui::EventType::button);
}