/* -*- Mode: c++ -*- */
/***************************************************************************
 *            drumkitimage.cc
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
#include "drumkitimage.h"

#include "painter.h"
#include <cpp11fix.h>

namespace GUI
{

DrumKitImage::DrumKitImage(Widget* parent)
	: Widget(parent)
{
}

void DrumKitImage::setImages(const std::string& imagefile,
                             const std::string& overlayfile)
{
	overlay.setOverlay(overlayfile);
	image = std::make_unique<Image>(imagefile);
	redraw();
}

void DrumKitImage::clearImages()
{
	overlay.clearOverlay();
	image.reset();
	redraw();
}

void DrumKitImage::resize(std::size_t width, std::size_t height)
{
	Widget::resize(width, height);
	overlay.resize(width, height);
}

void DrumKitImage::buttonEvent(ButtonEvent* buttonEvent)
{
}

void DrumKitImage::scrollEvent(ScrollEvent* scrollEvent)
{
}

void DrumKitImage::mouseMoveEvent(MouseMoveEvent* mouseMoveEvent)
{
	Widget::mouseMoveEvent(mouseMoveEvent);
	//redraw();
}

void DrumKitImage::mouseLeaveEvent()
{
}

void DrumKitImage::repaintEvent(RepaintEvent* repaintEvent)
{
	Painter painter(*this);
	painter.clear();

	auto drumkit_scale = (float)width() / image->width();

	if(image)
	{
		painter.drawImageStretched(0, 0, *image,
		                           image->width() * drumkit_scale,
		                           image->height() * drumkit_scale,
		                           Filter::Linear);
	}
}

DrumKitImage::Overlay::Overlay(Widget* parent)
	: Widget(parent)
{
}

void DrumKitImage::Overlay::setOverlay(const std::string& overlayfile)
{
	overlay = std::make_unique<Image>(overlayfile);
	redraw();
}

void DrumKitImage::Overlay::clearOverlay()
{
	overlay.reset();
	redraw();
}

static auto has_highlight_colour = false;

void DrumKitImage::Overlay::buttonEvent(ButtonEvent* buttonEvent)
{
	has_highlight_colour = !has_highlight_colour;
	redraw();
}

void DrumKitImage::Overlay::mouseMoveEvent(MouseMoveEvent* mouseMoveEvent)
{
}

void DrumKitImage::Overlay::repaintEvent(RepaintEvent* repaintEvent)
{
	Painter painter(*this);
	painter.clear();

	auto drumkit_scale = (float)width() / overlay->width();
	auto colour = Colour(0.0f, 0, 0, .8f);

	if(overlay && has_highlight_colour)
	{
		painter.drawRestrictedImageStretched(0, 0, colour, *overlay,
		                                     overlay->width() * drumkit_scale,
		                                     overlay->height() * drumkit_scale,
		                                     Filter::Nearest);
	}
}

} // GUI::
