/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            nativewindow_headless.h
 *
 *  Sat Apr 11 00:00:00 CET 2026
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
#pragma once

#include "nativewindow.h"

namespace dggui
{

class Window;

//! Headless (no-display) NativeWindow implementation for unit testing.
//! All display operations are no-ops; getEvents() always returns an empty queue.
class NativeWindowHeadless
	: public NativeWindow
{
public:
	NativeWindowHeadless(void* native_window, Window& window);
	~NativeWindowHeadless() override;

	// From NativeWindow:
	void setFixedSize(std::size_t width, std::size_t height) override;
	void setAlwaysOnTop(bool always_on_top) override;
	void resize(std::size_t width, std::size_t height) override;
	std::pair<std::size_t, std::size_t> getSize() const override;
	void move(int x, int y) override;
	std::pair<int, int> getPosition() const override;
	void show() override;
	void hide() override;
	bool visible() const override;
	void setCaption(const std::string& caption) override;
	void redraw(const Rect& dirty_rect) override;
	void grabMouse(bool grab) override;
	EventQueue getEvents() override;
	void* getNativeWindowHandle() const override;
	Point translateToScreen(const Point& point) override;

private:
	std::size_t _width{1};
	std::size_t _height{1};
	int _x{0};
	int _y{0};
	bool _visible{false};
};

} // dggui::
