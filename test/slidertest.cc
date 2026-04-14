/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            slidertest.cc
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
#include <dggui/painter.h>
#include <dggui/slider.h>
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

class TestSlider : public dggui::Slider
{
public:
	explicit TestSlider(dggui::Widget* parent) : dggui::Slider(parent)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}
};

bool hasAnyDrawnPixels(TestableCanvas& canvas)
{
	auto& pixbuf = canvas.getPixelBuffer();
	for(std::size_t x = 0; x < pixbuf.width; ++x)
	{
		for(std::size_t y = 0; y < pixbuf.height; ++y)
		{
			const auto& c = pixbuf.pixel(x, y);
			if(c.red() > 0 || c.green() > 0 || c.blue() > 0 || c.alpha() > 0)
			{
				return true;
			}
		}
	}
	return false;
}

TEST_CASE("SliderTest")
{
	SUBCASE("testSliderCreation")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		CHECK_EQ(slider.value(), 0.0f);
	}

	SUBCASE("testSetValue")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(0.5f);
		CHECK_EQ(slider.value(), 0.5f);
	}

	SUBCASE("testSetValueNegative")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(-0.5f);
		CHECK_EQ(slider.value(), 0.0f);
	}

	SUBCASE("testSetValueAboveOne")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(1.5f);
		CHECK_EQ(slider.value(), 1.0f);
	}

	SUBCASE("testSetColour")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setColour(dggui::Slider::Colour::Green);
		slider.setColour(dggui::Slider::Colour::Red);
		slider.setColour(dggui::Slider::Colour::Blue);
		slider.setColour(dggui::Slider::Colour::Yellow);
		slider.setColour(dggui::Slider::Colour::Purple);
		slider.setColour(dggui::Slider::Colour::Grey);
	}

	SUBCASE("testSetEnabled")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setEnabled(false);
		slider.setEnabled(true);
	}

	SUBCASE("testSetColourWhenDisabled")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setEnabled(false);
		slider.setColour(dggui::Slider::Colour::Red);
		slider.setEnabled(true);
	}

	SUBCASE("testButtonEvent")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		slider.buttonEvent(&event);
	}

	SUBCASE("testButtonEventNonLeftButton")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::right;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		slider.buttonEvent(&event);
	}

	SUBCASE("testButtonEventDisabled")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setEnabled(false);

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::down;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		slider.buttonEvent(&event);
	}

	SUBCASE("testButtonEventUp")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(0.5f);

		dggui::ButtonEvent event;
		event.direction = dggui::Direction::up;
		event.button = dggui::MouseButton::left;
		event.x = 50;
		event.y = 15;
		event.doubleClick = false;

		slider.buttonEvent(&event);
	}

	SUBCASE("testMouseMoveEvent")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ButtonEvent downEvent;
		downEvent.direction = dggui::Direction::down;
		downEvent.button = dggui::MouseButton::left;
		downEvent.x = 50;
		downEvent.y = 15;
		downEvent.doubleClick = false;
		slider.buttonEvent(&downEvent);

		dggui::MouseMoveEvent moveEvent;
		moveEvent.x = 75;
		moveEvent.y = 15;
		slider.mouseMoveEvent(&moveEvent);
	}

	SUBCASE("testScrollEvent")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ScrollEvent event;
		event.delta = 0.1f;
		event.x = 50;
		event.y = 15;

		slider.scrollEvent(&event);
	}

	SUBCASE("testScrollEventDisabled")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setEnabled(false);

		dggui::ScrollEvent event;
		event.delta = 0.1f;
		event.x = 50;
		event.y = 15;

		slider.scrollEvent(&event);
	}

	SUBCASE("testScrollEventNegativeClamp")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(0.0f);

		dggui::ScrollEvent event;
		event.delta = -1.0f;
		event.x = 50;
		event.y = 15;

		slider.scrollEvent(&event);
	}

	SUBCASE("testScrollEventAboveOneClamp")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(1.0f);

		dggui::ScrollEvent event;
		event.delta = 1.0f;
		event.x = 50;
		event.y = 15;

		slider.scrollEvent(&event);
	}

	SUBCASE("testRepaintEvent")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 100;
		event.height = 30;

		slider.repaintEvent(&event);
	}

	SUBCASE("testRepaintEventSmallWidth")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(10, 30);

		dggui::RepaintEvent event;
		event.x = 0;
		event.y = 0;
		event.width = 10;
		event.height = 30;

		slider.repaintEvent(&event);
	}

	SUBCASE("testGetControlWidth")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		slider.setValue(0.5f);
		CHECK_GE(slider.value(), 0.0f);
		CHECK_LE(slider.value(), 1.0f);
	}

	SUBCASE("testRecomputeCurrentValueAtMin")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ButtonEvent downEvent;
		downEvent.direction = dggui::Direction::down;
		downEvent.button = dggui::MouseButton::left;
		downEvent.x = 0;
		downEvent.y = 15;
		downEvent.doubleClick = false;
		slider.buttonEvent(&downEvent);

		CHECK_EQ(slider.value(), 0.0f);
	}

	SUBCASE("testRecomputeCurrentValueBeyondMax")
	{
		TestWindow window;
		TestSlider slider(&window);
		slider.resize(100, 30);

		dggui::ButtonEvent downEvent;
		downEvent.direction = dggui::Direction::down;
		downEvent.button = dggui::MouseButton::left;
		downEvent.x = 200;
		downEvent.y = 15;
		downEvent.doubleClick = false;
		slider.buttonEvent(&downEvent);

		CHECK_EQ(slider.value(), 1.0f);
	}
}