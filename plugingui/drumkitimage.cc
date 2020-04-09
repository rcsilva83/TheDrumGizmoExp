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
	click_overlay.setVisible(false);
}

void DrumKitImage::setImages(const std::string& imagefile,
                             const std::string& overlayfile)
{
	image = std::make_unique<Image>(imagefile);
	overlay = std::make_unique<Image>(overlayfile);

	hover_overlay.setOverlay(*overlay);
	click_overlay.setOverlay(*overlay);

	scale = (float)width() / image->width();

	redraw();
}

void DrumKitImage::clearImages()
{
	hover_overlay.clearOverlay();
	click_overlay.clearOverlay();
	image.reset();
	overlay.reset();
	redraw();
}

void DrumKitImage::resize(std::size_t width, std::size_t height)
{
	Widget::resize(width, height);
	hover_overlay.resize(width, height);
	click_overlay.resize(width, height);
	scale = (float)width / image->width();
}

void DrumKitImage::buttonEvent(ButtonEvent* buttonEvent)
{
	click_overlay.setVisible(buttonEvent->direction == Direction::down);
}

void DrumKitImage::scrollEvent(ScrollEvent* scrollEvent)
{
}

void DrumKitImage::mouseMoveEvent(MouseMoveEvent* mouseMoveEvent)
{
	if(overlay)
	{
		auto scale = (float)width() / image->width();
		auto colour = overlay->getPixel(mouseMoveEvent->x / scale,
		                                mouseMoveEvent->y / scale);
		hover_overlay.setColour(colour);
		click_overlay.setColour(colour);
	}

	Widget::mouseMoveEvent(mouseMoveEvent);
}

void DrumKitImage::mouseLeaveEvent()
{
}

void DrumKitImage::repaintEvent(RepaintEvent* repaintEvent)
{
	Painter painter(*this);
	painter.clear();

	if(image)
	{
		painter.drawImageStretched(0, 0, *image,
		                           image->width() * scale,
		                           image->height() * scale,
		                           Filter::Linear);
	}
}

DrumKitImage::Overlay::Overlay(Widget* parent)
	: Widget(parent)
{
}

void DrumKitImage::Overlay::setOverlay(const Image& overlay)
{
	this->overlay = &overlay;
	scale = (float)width() / overlay.width();
	needs_repaint = true;
	redraw();
}

void DrumKitImage::Overlay::clearOverlay()
{
	overlay = nullptr;
	redraw();
}

void DrumKitImage::Overlay::setColour(const Colour& colour)
{
	if(highlight_colour != colour)
	{
		highlight_colour = colour;
		highlight_colour.data()[3] = 64;
		needs_repaint = true;
		redraw();
	}
}

void DrumKitImage::Overlay::buttonEvent(ButtonEvent* buttonEvent)
{
	// Propagate event to parent
	parent->buttonEvent(buttonEvent);
}

void DrumKitImage::Overlay::mouseMoveEvent(MouseMoveEvent* mouseMoveEvent)
{
	// Propagate event to parent
	parent->mouseMoveEvent(mouseMoveEvent);
}

void DrumKitImage::Overlay::resize(std::size_t width, std::size_t height)
{
	Widget::resize(width, height);
	scale = (float)width / overlay->width();
	needs_repaint = true;
}

void DrumKitImage::Overlay::repaintEvent(RepaintEvent* repaintEvent)
{
	if(!needs_repaint)
	{
		return;
	}

	Painter painter(*this);
	painter.clear();

	if(overlay)
	{
		painter.drawRestrictedImageStretched(0, 0, highlight_colour, *overlay,
		                                     overlay->width() * scale,
		                                     overlay->height() * scale,
		                                     Filter::Nearest);
	}

	needs_repaint = false;
}

} // GUI::
