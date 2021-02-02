/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsamidi.cc
 *
 *  Copyright 2021 corrados
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
#include <iostream>
#include <cassert>

#include "cpp11fix.h" // required for c++11
#include "alsamidi.h"

AlsaMidiInputEngine::AlsaMidiInputEngine()
	: AudioInputEngineMidi{}
	, in_port(0)
	, seq_handle{nullptr}
	, pos{0u}
	, events{}
{
}

AlsaMidiInputEngine::~AlsaMidiInputEngine()
{
}

bool AlsaMidiInputEngine::init(const Instruments& instruments)
{
	if(!loadMidiMap(midimap_file, instruments))
	{
		std::cerr << "[MidifileInputEngine] Failed to parse midimap '"
		          << midimap_file << "'\n";
		return false;
	}

	snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0);

	snd_seq_set_client_name(seq_handle, "Midi Listener");

	in_port =
		snd_seq_create_simple_port(seq_handle, "listen:in",
		                           SND_SEQ_PORT_CAP_WRITE |
		                           SND_SEQ_PORT_CAP_SUBS_WRITE,
		                           SND_SEQ_PORT_TYPE_APPLICATION);

	return true;
}

void AlsaMidiInputEngine::setParm(const std::string& parm, const std::string& value)
{
	if(parm == "midimap")
	{
		// apply midimap filename
		midimap_file = value;
	}
	else
	{
		std::cerr << "[AlsaMidiInputEngine] Unsupported parameter '" << parm
		          << "'\n";
	}
}

bool AlsaMidiInputEngine::start()
{
	return true;
}

void AlsaMidiInputEngine::stop()
{
}

void AlsaMidiInputEngine::pre()
{
}

void AlsaMidiInputEngine::run(size_t pos, size_t len,
                              std::vector<event_t>& events)
{
	assert(events.empty());

	snd_seq_event_t* ev = nullptr;
	snd_seq_event_input(seq_handle, &ev);

	processNote(ev->data.raw8.d, 12, ev->data.time.tick, events);
}

void AlsaMidiInputEngine::post()
{
}

bool AlsaMidiInputEngine::isFreewheeling() const
{
	return true;
}
