/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            audioinputenginemidi.cc
 *
 *  Mon Apr  1 20:13:25 CEST 2013
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
#include "audioinputenginemidi.h"

#include "instrument.h"
#include "midimapparser.h"

#include <cassert>

#include <hugin.hpp>

bool AudioInputEngineMidi::loadMidiMap(const std::string& midimap_file,
                                       const Instruments& instruments)
{
	std::string file = midimap_file;

	if(refs.load())
	{
		if(file.size() > 1 && file[0] == '@')
		{
			file = refs.getValue(file.substr(1));
		}
	}
	else
	{
		ERR(drumkitparser, "Error reading refs.conf");
	}

	midimap = "";
	is_valid = false;

	DEBUG(mmap, "loadMidiMap(%s, i.size() == %d)\n", file.c_str(),
	      (int)instruments.size());

	if(file.empty())
	{
		return false;
	}

	MidiMapParser midimap_parser;
	if(!midimap_parser.parseFile(file))
	{
		return false;
	}

	instrmap_t instrmap;
	for(size_t i = 0; i < instruments.size(); i++)
	{
		instrmap[instruments[i]->getName()] = static_cast<int>(i);
		instrument_states[i] = InstrumentState{};
	}

	mmap.swap(instrmap, midimap_parser.midimap);

	midimap = file;
	is_valid = true;

	return true;
}

std::string AudioInputEngineMidi::getMidimapFile() const
{
	return midimap;
}

bool AudioInputEngineMidi::isValid() const
{
	return is_valid;
}

// Note types:
constexpr std::uint8_t NoteOff{0x80};
constexpr std::uint8_t NoteOn{0x90};
constexpr std::uint8_t NoteAftertouch{0xA0};
constexpr std::uint8_t ControlChange{0xB0};

// Note type mask:
constexpr std::uint8_t NoteMask{0xF0};

void AudioInputEngineMidi::processNote(const std::uint8_t* midi_buffer,
                                       std::size_t midi_buffer_length,
                                       std::size_t offset,
                                       std::vector<event_t>& events)
{
	if(midi_buffer_length < 3)
	{
		return;
	}

	switch(midi_buffer[0] & NoteMask) // NOLINT - span
	{
	case NoteOff:
		// Ignore for now
		break;

	case NoteOn:
		{
			auto key = midi_buffer[1]; // NOLINT - span
			auto velocity = midi_buffer[2]; // NOLINT - span
			auto map_entries = mmap.lookup(key, MapFrom::Note, MapTo::PlayInstrument);
			auto instruments = mmap.lookup_instruments(map_entries);
			for(const auto& instrument_idx : instruments)
			{
				if(velocity != 0)
				{
					constexpr float lower_offset{0.5f};
					constexpr float midi_velocity_max{127.0f};
					// maps velocities to [.5/127, 126.5/127]
					assert(velocity <= 127); // MIDI only support up to 127
					auto centered_velocity =
						(static_cast<float>(velocity) - lower_offset) / midi_velocity_max;
					float position = 0.0f;
					float openness = 0.0f; // TODO
					auto instr_it = instrument_states.find(instrument_idx);
					if(instr_it != instrument_states.end())
					{
						position = instr_it->second.position;
						openness = instr_it->second.openness;
					}
					events.push_back({EventType::OnSet, (std::size_t)instrument_idx,
					                  offset, centered_velocity, position, openness});
				}
			}
		}
		break;

	case NoteAftertouch:
		{
			auto key = midi_buffer[1]; // NOLINT - span
			auto velocity = midi_buffer[2]; // NOLINT - span
			auto map_entries = mmap.lookup(key, MapFrom::Note, MapTo::PlayInstrument);
			auto instruments = mmap.lookup_instruments(map_entries);
			for(const auto& instrument_idx : instruments)
			{
				if(velocity > 0)
				{
					events.push_back({EventType::Choke, (std::size_t)instrument_idx,
					                  offset, .0f, .0f, .0f});
				}
			}
		}
		break;

	case ControlChange:
		{
			auto controller_number = midi_buffer[1]; // NOLINT - span
			auto value = midi_buffer[2]; // NOLINT - span

			// TODO: cross-map from cc to play, etc.
			auto map_entries = mmap.lookup(controller_number,
			                               MapFrom::CC,
										   MapTo::InstrumentState);
			for(const auto& entry : map_entries)
			{
				auto instrument_idx = mmap.lookup_instrument(entry.instrument_name);
				if (instrument_idx >= 0) {
					auto state_it = instrument_states.find(instrument_idx);
					if (state_it != instrument_states.end()) {
						InstrumentState &state = state_it->second;
						auto const max = (float) entry.state_max;
						auto const min = (float) entry.state_min;
						auto const in_clamped = std::min(std::max((float)value, std::min(min, max)), std::max(min, max));
						float fvalue = (in_clamped - min) / (max - min);
						if (entry.maybe_instrument_state_kind == InstrumentStateKind::Openness) {
							state_it->second.openness = fvalue;
						}
						else if (entry.maybe_instrument_state_kind == InstrumentStateKind::Position) {
							state_it->second.position = fvalue;
						}
					}
				}
			}

			// TODO: the old version deleted the position cache(s) here
		}
	}
}
