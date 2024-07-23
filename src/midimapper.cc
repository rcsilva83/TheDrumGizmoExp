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

int MidiMapper::lookup_instrument(std::string name) {
	const std::lock_guard<std::mutex> guard(mutex);
	auto instrmap_it = instrmap.find(name);
	if(instrmap_it != instrmap.end())
	{
		return instrmap_it->second;
	}
	return -1;
}

std::vector<int> MidiMapper::lookup_instruments(std::vector<MidimapEntry> const& entries) {
	const std::lock_guard<std::mutex> guard(mutex);
	std::set<int> rval;
	for(const auto& entry : entries)
	{
		auto it = instrmap.find(entry.instrument_name);
		if (it != instrmap.end()) {
			rval.insert(it->second);
		}
	}
	return std::vector<int>(rval.begin(), rval.end());
}

std::vector<MidimapEntry> MidiMapper::lookup(
		int     from_id,
		MapFrom from_kind,
		MapTo   to_kind,
		InstrumentStateKind state_kind)
{
	std::vector<MidimapEntry> rval;

	const std::lock_guard<std::mutex> guard(mutex);

	for(const auto& map_entry : midimap)
	{
		bool match = true;
		match = match && (from_id == map_entry.from_id || from_id == -1);
		match = match && (from_kind == MapFrom::None || from_kind == map_entry.from_kind);
		match = match && (to_kind == MapTo::None || to_kind == map_entry.to_kind);
		match = match && (state_kind == InstrumentStateKind::None || state_kind == map_entry.maybe_instrument_state_kind);

		if(match)
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
