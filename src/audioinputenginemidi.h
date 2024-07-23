/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            audioinputenginemidi.h
 *
 *  Mon Apr  1 20:13:24 CEST 2013
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
#pragma once

#include <string>
#include <map>

#include "audioinputengine.h"
#include "midimapper.h"
#include "instrument.h"
#include "configfile.h"
#include "instrumentstate.h"

class AudioInputEngineMidi
	: public AudioInputEngine
{
public:
	AudioInputEngineMidi() = default;
	virtual ~AudioInputEngineMidi() = default;

	virtual bool init(const Instruments &instruments) = 0;

	virtual void setParm(const std::string& parm, const std::string& value) = 0;

	virtual bool start() = 0;
	virtual void stop() = 0;

	virtual void pre() = 0;
	virtual void run(size_t pos, size_t len, std::vector<event_t>& events) = 0;
	virtual void post() = 0;

	virtual bool loadMidiMap(const std::string& midimap_file,
	                         const Instruments& i);

	std::string getMidimapFile() const;

	bool isValid() const;

	void processNote(const std::uint8_t* midi_buffer,
	                 std::size_t midi_buffer_length,
	                 std::size_t offset,
	                 std::vector<event_t>& events);

protected:
	MidiMapper mmap;
	std::map<int, InstrumentState> instrument_states;

private:
	std::string midimap;
	bool is_valid{false};

	ConfigFile refs{REFSFILE};
};
