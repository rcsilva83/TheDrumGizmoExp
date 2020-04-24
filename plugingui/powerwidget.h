/* -*- Mode: c++ -*- */
/***************************************************************************
 *            powerwidget.h
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
#pragma once

#include <widget.h>
#include <texturedbox.h>
#include <texture.h>
#include <knob.h>
#include <checkbox.h>
#include <powermap.h>

struct Settings;
class SettingsNotifier;

class PowerWidget
	: public GUI::Widget
{
public:
	PowerWidget(GUI::Widget* parent,
	            Settings& settings,
	            SettingsNotifier& settings_notifier);

	// From Widget:
	virtual void repaintEvent(GUI::RepaintEvent *repaintEvent) override;
	virtual void resize(std::size_t width, std::size_t height) override;

private:
	GUI::TexturedBox box{getImageCache(), ":resources/widget.png",
			0, 0, // atlas offset (x, y)
			7, 1, 7, // dx1, dx2, dx3
			7, 63, 7}; // dy1, dy2, dy3

	class Canvas
		: public GUI::Widget
	{
	public:
		Canvas(GUI::Widget* parent, Settings& settings,
		       SettingsNotifier& settings_notifier);

		// From Widget:
		virtual void repaintEvent(GUI::RepaintEvent *repaintEvent) override;

	private:
		Powermap power_map;

		void parameterChangedFloat(float);
		void parameterChangedBool(bool);

		SettingsNotifier& settings_notifier;
		Settings& settings;

		bool enabled{true};
	};

	void chk_enable(bool v);
	void k0_x(float v);
	void k0_y(float v);
	void k1_x(float v);
	void k1_y(float v);
	void k2_x(float v);
	void k2_y(float v);
	void chk_shelf(bool v);

	Canvas canvas;
	GUI::CheckBox checkbox_enable{this};
	GUI::Knob knob0_x{this};
	GUI::Knob knob0_y{this};
	GUI::Knob knob1_x{this};
	GUI::Knob knob1_y{this};
	GUI::Knob knob2_x{this};
	GUI::Knob knob2_y{this};
	GUI::CheckBox checkbox_shelf{this};

	Settings& settings;
};
