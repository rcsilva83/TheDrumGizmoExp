/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midiprocessor.h
 *
 *  Thu May 12 08:20:49 CEST 2016
 *  Copyright 2016 Christian Glöckner
 *  cgloeckner@freenet.de
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
#include <vector>

#include "event.h"
#include "midimapper.h"

class MidiProcessor {
private:
	MidiMapper& midi_mapper;
	
public:
	MidiProcessor(MidiMapper& midi_mapper);
	
	bool operator()(std::size_t size, unsigned char const * buffer, event_t& out);
};
	
