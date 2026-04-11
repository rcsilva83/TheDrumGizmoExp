/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            nativewindow_headless.cc
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
#include "nativewindow_headless.h"

#include "window.h"

namespace dggui
{

NativeWindowHeadless::NativeWindowHeadless(
    void* /*native_window*/, Window& /*window*/)
{
}

NativeWindowHeadless::~NativeWindowHeadless()
{
}

void NativeWindowHeadless::setFixedSize(std::size_t width, std::size_t height)
{
	_width = width;
	_height = height;
}

void NativeWindowHeadless::setAlwaysOnTop(bool /*always_on_top*/)
{
}

void NativeWindowHeadless::resize(std::size_t width, std::size_t height)
{
	_width = width;
	_height = height;
}

std::pair<std::size_t, std::size_t> NativeWindowHeadless::getSize() const
{
	return {_width, _height};
}

void NativeWindowHeadless::move(int x, int y)
{
	_x = x;
	_y = y;
}

std::pair<int, int> NativeWindowHeadless::getPosition() const
{
	return {_x, _y};
}

void NativeWindowHeadless::show()
{
	_visible = true;
}

void NativeWindowHeadless::hide()
{
	_visible = false;
}

bool NativeWindowHeadless::visible() const
{
	return _visible;
}

void NativeWindowHeadless::setCaption(const std::string& /*caption*/)
{
}

void NativeWindowHeadless::redraw(const Rect& /*dirty_rect*/)
{
}

void NativeWindowHeadless::grabMouse(bool /*grab*/)
{
}

EventQueue NativeWindowHeadless::getEvents()
{
	return {};
}

void* NativeWindowHeadless::getNativeWindowHandle() const
{
	return nullptr;
}

Point NativeWindowHeadless::translateToScreen(const Point& point)
{
	return point;
}

} // namespace dggui
