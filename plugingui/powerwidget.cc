/* -*- Mode: c++ -*- */
/***************************************************************************
 *            powerwidget.cc
 *
 *  Fri Apr 24 17:30:45 CEST 2020
 *  Copyright 2020 Bent Bisballe Nyeng
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
#include "powerwidget.h"

#include "painter.h"

#include <notifier.h>
#include <settings.h>
#include <colour.h>

#include <powermap.h>

#include <hugin.hpp>
#include <cmath>

PowerWidget::PowerWidget(GUI::Widget* parent,
                         Settings& settings,
                         SettingsNotifier& settings_notifier)
	: GUI::Widget(parent)
	, canvas(this, settings, settings_notifier)
	, settings(settings)
{
	canvas.move(7, 7);

	CONNECT(&checkbox_enable, stateChangedNotifier, this, &PowerWidget::chk_enable);
	CONNECT(&knob0_x, valueChangedNotifier, this, &PowerWidget::k0_x);
	CONNECT(&knob0_y, valueChangedNotifier, this, &PowerWidget::k0_y);
	CONNECT(&knob1_x, valueChangedNotifier, this, &PowerWidget::k1_x);
	CONNECT(&knob1_y, valueChangedNotifier, this, &PowerWidget::k1_y);
	CONNECT(&knob2_x, valueChangedNotifier, this, &PowerWidget::k2_x);
	CONNECT(&knob2_y, valueChangedNotifier, this, &PowerWidget::k2_y);
	CONNECT(&checkbox_shelf, stateChangedNotifier, this, &PowerWidget::chk_shelf);

	checkbox_enable.resize(100, 42);
	knob0_x.resize(42, 42);
	knob0_y.resize(42, 42);
	knob1_x.resize(42, 42);
	knob1_y.resize(42, 42);
	knob2_x.resize(42, 42);
	knob2_y.resize(42, 42);
	checkbox_shelf.resize(100, 42);

	CONNECT(&settings_notifier, enable_powermap, &checkbox_enable, &GUI::CheckBox::setChecked);
	CONNECT(&settings_notifier, fixed0_x, &knob0_x, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, fixed0_y, &knob0_y, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, fixed1_x, &knob1_x, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, fixed1_y, &knob1_y, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, fixed2_x, &knob2_x, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, fixed2_y, &knob2_y, &GUI::Knob::setValue);
	CONNECT(&settings_notifier, shelf, &checkbox_shelf, &GUI::CheckBox::setChecked);
}

void PowerWidget::chk_enable(bool v)
{
	settings.enable_powermap.store(v);
}

void PowerWidget::k0_x(float v)
{
	settings.fixed0_x.store(v);
}

void PowerWidget::k0_y(float v)
{
	settings.fixed0_y.store(v);
}

void PowerWidget::k1_x(float v)
{
	settings.fixed1_x.store(v);
}

void PowerWidget::k1_y(float v)
{
	settings.fixed1_y.store(v);
}

void PowerWidget::k2_x(float v)
{
	settings.fixed2_x.store(v);
}

void PowerWidget::k2_y(float v)
{
	settings.fixed2_y.store(v);
}

void PowerWidget::chk_shelf(bool v)
{
	settings.shelf.store(v);
}

void PowerWidget::repaintEvent(GUI::RepaintEvent *repaintEvent)
{
	GUI::Painter p(*this);
	box.setSize(width(), height() / 2);
	p.drawImage(0, 0, box);
}

void PowerWidget::resize(std::size_t width, std::size_t height)
{
	Widget::resize(width, height);
	if(width < 14 || height < 14)
	{
		canvas.resize(1, 1);
		return;
	}
	canvas.resize(width - 14, height / 2 - 14);

	checkbox_enable.move(220, height / 2 + 14);
	knob0_x.move(0,   height / 2 + 14);
	knob0_y.move(0,   height / 2 + 14 + 60);
	knob1_x.move(80,  height / 2 + 14);
	knob1_y.move(80,  height / 2 + 14 + 60);
	knob2_x.move(160, height / 2 + 14);
	knob2_y.move(160, height / 2 + 14 + 60);
	checkbox_shelf.move(220, height / 2 + 14 + 60);
}

PowerWidget::Canvas::Canvas(GUI::Widget* parent,
                            Settings& settings,
                            SettingsNotifier& settings_notifier)
	: GUI::Widget(parent)
	, settings_notifier(settings_notifier)
	, settings(settings)
{
	CONNECT(this, settings_notifier.enable_powermap,
	        this, &PowerWidget::Canvas::parameterChangedBool);
	CONNECT(this, settings_notifier.fixed0_x,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.fixed0_y,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.fixed1_x,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.fixed1_y,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.fixed2_x,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.fixed2_y,
	        this, &PowerWidget::Canvas::parameterChangedFloat);
	CONNECT(this, settings_notifier.shelf,
	        this, &PowerWidget::Canvas::parameterChangedBool);

	parameterChangedFloat(0);
}

void PowerWidget::Canvas::repaintEvent(GUI::RepaintEvent *repaintEvent)
{
	if(width() < 1 || height() < 1)
	{
		return;
	}

	GUI::Painter p(*this);

	p.clear();

	// draw the fixed nodes of the spline
	float rad = radius * width();
	p.setColour(GUI::Colour{0.0f, 0.7f, 0.5f, 1.0f});
	p.drawFilledCircle(settings.fixed0_x.load() * width(),
	                   height() - settings.fixed0_y.load() * height(), rad);
	p.setColour(GUI::Colour{0.5f, 0.7f, 0.0f, 1.0f});
	p.drawFilledCircle(settings.fixed1_x.load() * width(),
	                   height() - settings.fixed1_y.load() * height(), rad);
	p.setColour(GUI::Colour{0.5f, 0.0f, 0.7f, 1.0f});
	p.drawFilledCircle(settings.fixed2_x.load() * width(),
	                   height() - settings.fixed2_y.load() * height(), rad);

	if(enabled)
	{
		// enabled green
		p.setColour(GUI::Colour(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else
	{
		// disabled grey
		p.setColour(GUI::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	}

	// draw 64 line segments across the region
	std::pair<int, int> old{};
	for(std::size_t x = 0; x < width(); x += width() / 64)
	{
		int y = power_map.map((float)x / width()) * height();
		if(x > 0)
		{
			p.drawLine(old.first, old.second, x, height() - y);
		}
		old = { x, height() - y };
	}

	int x = width();
	int y = power_map.map((float)x / width()) * height();
	p.drawLine(old.first, old.second, x, height() - y);
	old = { x, height() - y };
}

void PowerWidget::Canvas::buttonEvent(GUI::ButtonEvent* buttonEvent)
{
	float x0 = (float)buttonEvent->x / width();
	float y0 = (float)(height() - buttonEvent->y) / height();

	switch(buttonEvent->direction)
	{
	case GUI::Direction::up:
		in_point = -1;
		break;
	case GUI::Direction::down:
		if(std::abs(x0 - settings.fixed0_x.load()) < radius * 1.5 &&
		   std::abs(y0 - settings.fixed0_y.load()) < radius * 1.5)
		{
			in_point = 0;
		}

		if(std::abs(x0 - settings.fixed1_x.load()) < radius * 1.5 &&
		   std::abs(y0 - settings.fixed1_y.load()) < radius * 1.5)
		{
			in_point = 1;
		}

		if(std::abs(x0 - settings.fixed2_x.load()) < radius * 1.5 &&
		   std::abs(y0 - settings.fixed2_y.load()) < radius * 1.5)
		{
			in_point = 2;
		}
		break;
	}
}

void PowerWidget::Canvas::mouseMoveEvent(GUI::MouseMoveEvent* mouseMoveEvent)
{
	switch(in_point)
	{
	case 0:
		settings.fixed0_x.store((float)mouseMoveEvent->x / width());
		settings.fixed0_y.store((float)(height() - mouseMoveEvent->y) / height());
		redraw();
		break;
	case 1:
		settings.fixed1_x.store((float)mouseMoveEvent->x / width());
		settings.fixed1_y.store((float)(height() - mouseMoveEvent->y) / height());
		redraw();
		break;
	case 2:
		settings.fixed2_x.store((float)mouseMoveEvent->x / width());
		settings.fixed2_y.store((float)(height() - mouseMoveEvent->y) / height());
		redraw();
		break;
	default:
		break;
	}
}

void PowerWidget::Canvas::mouseLeaveEvent()
{
	//in_point = -1;
}

void PowerWidget::Canvas::parameterChangedFloat(float)
{
	power_map.setFixed0({settings.fixed0_x.load(), settings.fixed0_y.load()});
	power_map.setFixed1({settings.fixed1_x.load(), settings.fixed1_y.load()});
	power_map.setFixed2({settings.fixed2_x.load(), settings.fixed2_y.load()});
	power_map.setShelf(settings.shelf.load());
	enabled = settings.enable_powermap.load();
	redraw();
}

void PowerWidget::Canvas::parameterChangedBool(bool)
{
	parameterChangedFloat(0);
}
