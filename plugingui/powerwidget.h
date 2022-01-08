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

#include <dggui/widget.h>
#include <dggui/texturedbox.h>
#include <dggui/texture.h>
#include <dggui/knob.h>
#include <dggui/checkbox.h>
#include <dggui/label.h>
#include <dggui/font.h>

#include <powermap.h>

struct Settings;
class SettingsNotifier;

namespace GUI
{

class PowerWidget
	: public dggui::Widget
{
public:
	PowerWidget(dggui::Widget* parent,
	            Settings& settings,
	            SettingsNotifier& settings_notifier);

	// From Widget:
	void repaintEvent(const dggui::RepaintEvent& repaintEvent) override;
	void resize(std::size_t width, std::size_t height) override;

private:
	dggui::TexturedBox box{getImageCache(), ":resources/widget.png",
			0, 0, // atlas offset (x, y)
			7, 1, 7, // dx1, dx2, dx3
			7, 63, 7}; // dy1, dy2, dy3

	class Canvas
		: public dggui::Widget
	{
	public:
		Canvas(dggui::Widget* parent, Settings& settings,
		       SettingsNotifier& settings_notifier);

		// From Widget:
		bool catchMouse() override { return true; }
		void repaintEvent(const dggui::RepaintEvent& repaintEvent) override;
		void buttonEvent(const dggui::ButtonEvent& buttonEvent) override;
		void mouseMoveEvent(const dggui::MouseMoveEvent& mouseMoveEvent) override;
		void mouseLeaveEvent() override;

	private:
		Powermap power_map;

		void parameterChangedFloat(float);
		void parameterChangedBool(bool);

		SettingsNotifier& settings_notifier;
		Settings& settings;

		bool enabled{true};

		int in_point{-1};
		const float radius = 0.02f;
		const float brd = 6.0f;
		dggui::Font font{":resources/fontemboss.png"};
	};

	void chk_shelf(bool v);

	Canvas canvas;
	dggui::Label shelf_label{this};
	dggui::CheckBox shelf_checkbox{this};

	Settings& settings;
};

} // ::GUI
