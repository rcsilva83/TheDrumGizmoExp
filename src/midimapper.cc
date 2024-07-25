/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midimapper.cc
 *
 *  Mon Jul 21 15:24:08 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#include "midimapper.h"
#include <set>

#include <cpp11fix.h>

MidimapEntry::MidimapEntry(int note_id,
				           std::string instrument_name,
				           CurveMap *maybe_curve_map) :
	note_id(note_id)
	, instrument_name(instrument_name)
{
	if (maybe_curve_map)
	{
		this->maybe_curve_map = std::make_unique<CurveMap>(*maybe_curve_map);
	}
}

MidimapEntry::MidimapEntry(const MidimapEntry& other)
{
	*this = other;
}

MidimapEntry &MidimapEntry::operator=(const MidimapEntry& other)
{
	note_id = other.note_id;
	instrument_name = other.instrument_name;
	if (other.maybe_curve_map)
	{
		maybe_curve_map = std::make_unique<CurveMap>(*other.maybe_curve_map);
	}

	return *this;
}

int MidiMapper::lookup_instrument(std::string name) {
	const std::lock_guard<std::mutex> guard(mutex);
	auto instrmap_it = instrmap.find(name);
	if(instrmap_it != instrmap.end())
	{
		return instrmap_it->second;
	}
	return -1;
}

std::vector<MidimapEntry> MidiMapper::lookup(int note_id)
{
	std::vector<MidimapEntry> rval;

	const std::lock_guard<std::mutex> guard(mutex);

	for(const auto& map_entry : midimap)
	{
		if(note_id == map_entry.note_id)
		{
			rval.push_back(map_entry);
		}
	}

	return rval;
}

void MidiMapper::swap(instrmap_t& instrmap, midimap_t& midimap)
{
	const std::lock_guard<std::mutex> guard(mutex);

	std::swap(this->instrmap, instrmap);
	std::swap(this->midimap, midimap);
}

const midimap_t& MidiMapper::getMap()
{
	return midimap;
}
