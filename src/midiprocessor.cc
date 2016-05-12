/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midiprocessor.cc
 *
 *  Thu May 12 09:41:25 CEST 2016
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
#include "midiprocessor.h"

MidiProcessor::MidiProcessor(MidiMapper& midi_mapper)
	: midi_mapper{midi_mapper} {
}

bool MidiProcessor::operator()(std::size_t size, unsigned char const * buffer, event_t& out) {
	if (size != 3u) {
		// not enough data
		return false;
	}
	
	// determine event type
	switch (buffer[0] & 0xF0) {
		case 0x90:
			out.type = TYPE_ONSET;
			break;
			
		default:
			// yet unsupported type
			return false;
	}
	
	// lookup instrument
	int instr = midi_mapper.lookup(buffer[1]);
	if (instr == -1) {
		// key does not map to an instrument
		return false;
	}
	
	out.instrument = (std::size_t)instr;
	out.velocity = buffer[2] / 127.0;
	return true;
}
