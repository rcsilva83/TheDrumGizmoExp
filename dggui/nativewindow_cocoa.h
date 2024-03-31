/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            nativewindow_cocoa.h
 *
 *  Sun Dec  4 15:55:14 CET 2016
 *  Copyright 2016 Bent Bisballe Nyeng
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

#include <memory>

#include "nativewindow.h"

namespace dggui
{

class Window;
class NativeWindowCocoa
	: public NativeWindow
{
public:
	NativeWindowCocoa(void* native_window, Window& window);
	~NativeWindowCocoa();

	// From NativeWindow:
	void setFixedSize(std::size_t width, std::size_t height) override;
	void setAlwaysOnTop(bool always_on_top) override;
	void resize(std::size_t width, std::size_t height) override;
	std::pair<std::size_t, std::size_t> getSize() const override;
	bool isHiDPI() override;
	void move(int x, int y) override;
	std::pair<int, int> getPosition() const override;
	void show() override;
	void hide() override;
	bool visible() const override;
	void setCaption(const std::string &caption) override;
	void redraw(const Rect& dirty_rect) override;
	void grabMouse(bool grab) override;
	EventQueue getEvents() override;
	void* getNativeWindowHandle() const override;
	Point translateToScreen(const Point& point) override;

	// Expose friend members of Window to ObjC++ implementation.
	class Window& getWindow();
	class PixelBuffer& getWindowPixbuf();
	void resized();
	void pushBackEvent(std::shared_ptr<Event> event);

private:
	void updateLayerOffset();

	Window& window;
	std::unique_ptr<struct priv> priv;
	EventQueue event_queue;
	void* native_window{nullptr};
	bool first{true};
	float scale{1.0};
};

} // dggui::
