/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            filebrowser.cc
 *
 *  Mon Feb 25 21:09:44 CET 2013
 *  Copyright 2013 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "tabs.h"

#include <hugin.hpp>

namespace GUI {

Tabs::Tabs(Widget *parent)
	: Widget(parent)
{
}

Tabs::~Tabs()
{
}

void Tabs::addTab(Widget &w, std::string title)
{
	w.setVisible(false);

	TabItem tab_item;
//	tab_item.widget = w;
//	tab_item.title = title;

	tabs.push_back(w);
}

void Tabs::showTab(unsigned int i)
{
	tabs.at(current_tab).setVisible(false);
	tabs.at(i).setVisible(true);	
	current_tab = i;
	repaintEvent(nullptr);
}

void Tabs::resize(int w, int h)
{
}

void Tabs::repaintEvent(RepaintEvent* repaintEvent)
{
	Painter p(*this);
}

} // GUI::
