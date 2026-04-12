/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            combobox.cc
 *
 *  Sun Mar 10 19:04:50 CET 2013
 *  Copyright 2013 Bent Bisballe Nyeng
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
#include "combobox.h"

#include "font.h"
#include "painter.h"

#include <stdio.h>

#define BORDER 10

namespace dggui
{

void ComboBox::listboxSelectHandler()
{
	ButtonEvent ev{};
	ev.button = MouseButton::left;
	ev.doubleClick = false;
	ev.direction = Direction::down;
	this->buttonEvent(&ev);
}

ComboBox::ComboBox(Widget* parent) : Widget(parent), listbox(parent)
{
	CONNECT(&listbox, selectionNotifier, this, &ComboBox::listboxSelectHandler);
	CONNECT(&listbox, clickNotifier, this, &ComboBox::listboxSelectHandler);

	listbox.hide();
}

ComboBox::~ComboBox()
{
}

void ComboBox::addItem(const std::string& name, const std::string& value)
{
	listbox.addItem(name, value);
}

void ComboBox::clear()
{
	listbox.clear();
	redraw();
}

bool ComboBox::selectItem(int index)
{
	listbox.selectItem(index);
	redraw();
	return true;
}

std::string ComboBox::selectedName()
{
	return listbox.selectedName();
}

std::string ComboBox::selectedValue()
{
	return listbox.selectedValue();
}

static void drawArrow(Painter& p, int x, int y, int w, int h)
{
	p.drawLine(x, y, x + (w / 2), y + h);
	p.drawLine(x + (w / 2), y + h, x + w, y);

	y++;
	p.drawLine(x, y, x + (w / 2), y + h);
	p.drawLine(x + (w / 2), y + h, x + w, y);
}

void ComboBox::repaintEvent(RepaintEvent* repaintEvent)
{
	Painter p(*this);

	std::string _text = selectedName();

	int w = static_cast<int>(width());
	int h = static_cast<int>(height());
	if(w == 0 || h == 0)
	{
		return;
	}

	box.setSize(w, h);
	p.drawImage(0, 0, box);

	p.setColour(Colour(183.0f / 255.0f, 219.0f / 255.0f, 255.0 / 255.0f, 1.0f));
	p.drawText(BORDER - 4 + 3, static_cast<int>(height()) / 2 + 5 + 1 + 1,
	    font, _text);
	{
		int arrow_w = 10;
		int arrow_h = 6;
		drawArrow(p, static_cast<int>(width()) - 6 - 4 - arrow_w,
		    (static_cast<int>(height()) - arrow_h) / 2, arrow_w, arrow_h);
		p.drawLine(static_cast<int>(width()) - 6 - 4 - arrow_w - 4, 7,
		    static_cast<int>(width()) - 6 - 4 - arrow_w - 4,
		    static_cast<int>(height()) - 8);
	}
}

void ComboBox::keyEvent(KeyEvent* keyEvent)
{
	if(keyEvent->direction != Direction::up)
	{
		return;
	}
}

void ComboBox::buttonEvent(ButtonEvent* buttonEvent)
{
	// Ignore everything except left clicks.
	if(buttonEvent->button != MouseButton::left)
	{
		return;
	}

	if(buttonEvent->direction != Direction::down)
	{
		return;
	}

	if(!listbox.visible())
	{
		listbox.resize(width() - 10, 100);
		listbox.move(x() + 5, y() + height() - 7);
	}
	else
	{
		valueChangedNotifier(listbox.selectedName(), listbox.selectedValue());
	}

	listbox.setVisible(!listbox.visible());
}

} // namespace dggui
