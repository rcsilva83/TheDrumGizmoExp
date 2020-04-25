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

	CONNECT(&shelf_checkbox, stateChangedNotifier, this, &PowerWidget::chk_shelf);

	shelf_label.setText("Shelf");
	shelf_label.setAlignment(GUI::TextAlignment::center);
	shelf_label.resize(59, 16);
	shelf_checkbox.resize(59, 40);

	CONNECT(&settings_notifier, shelf, &shelf_checkbox,
	        &GUI::CheckBox::setChecked);
}

void PowerWidget::chk_shelf(bool v)
{
	settings.shelf.store(v);
}

void PowerWidget::repaintEvent(GUI::RepaintEvent *repaintEvent)
{
	GUI::Painter p(*this);
	box.setSize(width() - 59 - 64, height());
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
	canvas.resize(width - 14 - 59 - 64, height - 14);

	shelf_label.move(width - 59 + 5 - 32 , 0);
	shelf_checkbox.move(width - 59 + 5 - 32, 16);
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
	p.setColour(GUI::Colour{0.0f, 1.0f, 0.0f, 0.7f});
	p.drawFilledCircle(settings.fixed0_x.load() * width(),
	                   height() - settings.fixed0_y.load() * height(), rad);
	p.drawCircle(power_map.getFixed0().in * width(),
	             height() - power_map.getFixed0().out * height(), rad + 2);

	p.setColour(GUI::Colour{1.0f, 1.0f, 0.0f, 0.7f});
	p.drawFilledCircle(settings.fixed1_x.load() * width(),
	                   height() - settings.fixed1_y.load() * height(), rad);
	p.drawCircle(power_map.getFixed1().in * width(),
	             height() - power_map.getFixed1().out * height(), rad + 2);

	p.setColour(GUI::Colour{1.0f, 0.0f, 0.0f, 0.7f});
	p.drawFilledCircle(settings.fixed2_x.load() * width(),
	                   height() - settings.fixed2_y.load() * height(), rad);
	p.drawCircle(power_map.getFixed2().in * width(),
	             height() - power_map.getFixed2().out * height(), rad + 2);

	if(enabled)
	{
		// draw 1:1 line in grey in the background to indicate where 1:1 is
		p.setColour(GUI::Colour(0.5));
		p.drawLine(0, height(), width(), 0);
	}

	if(enabled)
	{
		// enabled green
		p.setColour(GUI::Colour(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else
	{
		// disabled grey
		p.setColour(GUI::Colour(0.5f));
	}

	// Draw very short line segments across the region
	std::pair<int, int> old{};
	for(std::size_t x = 0; x < width(); ++x)
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

	if(!enabled)
	{
		// draw 1:1 line in green
		p.setColour(GUI::Colour(0.0f, 1.0f, 0.0f, 1.0f));
		p.drawLine(0, height(), width(), 0);
	}
}

void PowerWidget::Canvas::buttonEvent(GUI::ButtonEvent* buttonEvent)
{
	float x0 = (float)buttonEvent->x / width();
	float y0 = (float)(height() - buttonEvent->y) / height();

	float radius_x = radius * 2;
	float radius_y = radius * width() / height() * 2;

	switch(buttonEvent->direction)
	{
	case GUI::Direction::up:
		in_point = -1;
		break;
	case GUI::Direction::down:
		if(std::abs(x0 - settings.fixed0_x.load()) < radius_x &&
		   std::abs(y0 - settings.fixed0_y.load()) < radius_y)
		{
			in_point = 0;
		}

		if(std::abs(x0 - settings.fixed1_x.load()) < radius_x &&
		   std::abs(y0 - settings.fixed1_y.load()) < radius_y)
		{
			in_point = 1;
		}

		if(std::abs(x0 - settings.fixed2_x.load()) < radius_x &&
		   std::abs(y0 - settings.fixed2_y.load()) < radius_y)
		{
			in_point = 2;
		}
		break;
	}
}

namespace
{
float clamp(float val)
{
	return std::max(0.0f, std::min(1.0f, val));
}
}

void PowerWidget::Canvas::mouseMoveEvent(GUI::MouseMoveEvent* mouseMoveEvent)
{
	switch(in_point)
	{
	case 0:
		settings.fixed0_x.store(clamp((float)mouseMoveEvent->x / width()));
		settings.fixed0_y.store(1.0f - clamp((float)mouseMoveEvent->y / height()));
		redraw();
		break;
	case 1:
		settings.fixed1_x.store(clamp((float)mouseMoveEvent->x / width()));
		settings.fixed1_y.store(1.0f - clamp((float)mouseMoveEvent->y / height()));
		redraw();
		break;
	case 2:
		settings.fixed2_x.store(clamp((float)mouseMoveEvent->x / width()));
		settings.fixed2_y.store(1.0f - clamp((float)mouseMoveEvent->y / height()));
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
