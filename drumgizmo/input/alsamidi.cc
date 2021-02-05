/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsamidi.cc
 *
 *  Copyright 2021 Volker Fischer (github.com/corrados)
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

struct AlsaMidiInitError
{
	int const code;
	const std::string msg;

	AlsaMidiInitError(int op_code, const std::string& msg)
		: code{op_code}
		, msg{msg}
	{
	}

	static inline void test(int code, const std::string& msg)
	{
		if(code < 0)
		{
			throw AlsaMidiInitError(code, msg);
		}
	}
};

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
	if(seq_handle != nullptr)
	{
		snd_seq_close(seq_handle);
	}
}

bool AlsaMidiInputEngine::init(const Instruments& instruments)
{
	if(!loadMidiMap(midimap_file, instruments))
	{
		std::cerr << "[AlsaMidiInputEngine] Failed to parse midimap '"
		          << midimap_file << "'\n";
		return false;
	}

	// try to initialize alsa MIDI
	try
	{
		// it is not allowed to block in the run() function, therefore we
		// have to use a non-blocking mode
		int value = snd_seq_open(&seq_handle, "default",
		                         SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
		AlsaMidiInitError::test(value, "snd_seq_open");

		value = snd_seq_set_client_name(seq_handle, "drumgizmo");
		AlsaMidiInitError::test(value, "snd_seq_set_client_name");

		in_port =
			snd_seq_create_simple_port(seq_handle, "listen:in",
			                           SND_SEQ_PORT_CAP_WRITE |
			                           SND_SEQ_PORT_CAP_SUBS_WRITE,
			                           SND_SEQ_PORT_TYPE_APPLICATION);
		AlsaMidiInitError::test(in_port, "snd_seq_create_simple_port");
	}
	catch(AlsaMidiInitError const& error)
	{
		std::cerr << "[AlsaMidiInputEngine] " << error.msg
		          << " failed: " << snd_strerror(error.code) << std::endl;
		return false;
	}

	return true;
}

void AlsaMidiInputEngine::setParm(const std::string& parm,
                                  const std::string& value)
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
	snd_seq_event_t* ev = NULL;
	if ( snd_seq_event_input(seq_handle, &ev) >= 0 )
	{
		// TODO Better solution needed: It seems that the very first raw byte
		// of the MIDI message is missing the type information which is coded
		// in the 4 high bits of the first byte. As a quick hack we add this
		// information based on the type information of the sequence event.
		std::uint8_t myd[3];
		for ( int i = 0; i < 3; ++i)
		{
			myd[i] = ev->data.raw8.d[i];
		}
		if(ev->type == SND_SEQ_EVENT_NOTEON)
		{
			myd[0] += 0x90; // NoteOn
		}
		else if(ev->type == SND_SEQ_EVENT_NOTEOFF)
		{
			myd[0] += 0x80; // NoteOff
		}
		else if(ev->type == SND_SEQ_EVENT_KEYPRESS )
		{
			myd[0] += 0xA0; // NoteAftertouch
		}

		// since we do not want to introduce any additional delay for the
		// MIDI processing, we set the offset to zero
		processNote(myd, 3, 0, events);
	}
	snd_seq_free_event(ev);
}

void AlsaMidiInputEngine::post()
{
}

bool AlsaMidiInputEngine::isFreewheeling() const
{
	return true;
}
