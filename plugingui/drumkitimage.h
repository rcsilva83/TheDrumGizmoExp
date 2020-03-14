/* -*- Mode: c++ -*- */
/***************************************************************************
 *            drumkitimage.h
 *
 *  Sun Mar  8 18:30:17 CET 2020
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

#include <cstdint>
#include <memory>

#include "widget.h"
#include "image.h"

namespace GUI
{

class DrumKitImage
	: public Widget
{
public:
	DrumKitImage(Widget* parent);

	void setImages(const std::string& imagefile, const std::string& overlayfile);
	void clearImages();

	// From Widget:
	void resize(std::size_t width, std::size_t height) override;
	void buttonEvent(ButtonEvent* buttonEvent) override;
	void scrollEvent(ScrollEvent* scrollEvent) override;
	void mouseMoveEvent(MouseMoveEvent* mouseMoveEvent) override;
	void mouseLeaveEvent() override;
	void repaintEvent(RepaintEvent* repaintEvent) override;

private:
	class Overlay
		: public Widget
	{
	public:
		Overlay(Widget* parent);

		void setOverlay(const std::string& overlayfile);
		void clearOverlay();

		// From Widget:
		void buttonEvent(ButtonEvent* buttonEvent) override;
		void mouseMoveEvent(MouseMoveEvent* mouseMoveEvent) override;
		void repaintEvent(RepaintEvent* repaintEvent) override;

	private:
		std::unique_ptr<Image> overlay;
	};

	Overlay overlay{this};
	std::unique_ptr<Image> image;
};

} // GUI::
