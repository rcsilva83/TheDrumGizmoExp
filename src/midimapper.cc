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

#include <cpp11fix.h>

MidimapEntry::MidimapEntry(MapFrom from_kind,
	             int from_id,
				 MapTo to_kind,
				 std::string instrument_name,
				 InstrumentStateKind maybe_instrument_state_kind
				 ) :
	from_kind(from_kind)
	, from_id(from_id)
	, to_kind(to_kind)
	, instrument_name(instrument_name)
	, maybe_instrument_state_kind(maybe_instrument_state_kind)
{}

MidimapEntry::MidimapEntry(const MidimapEntry& other)
{
	*this = other;
}

MidimapEntry &MidimapEntry::operator=(const MidimapEntry& other)
{
	from_kind = other.from_kind;
	from_id = other.from_id;
	to_kind = other.to_kind;
	instrument_name = other.instrument_name;
	maybe_instrument_state_kind = other.maybe_instrument_state_kind;

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
		match = match && (from_id == -1 || from_id == map_entry.from_id);
		match = match && (from_kind == MapFrom::NoneOrAny || from_kind == map_entry.from_kind);
		match = match && (to_kind == MapTo::NoneOrAny || to_kind == map_entry.to_kind);
		match = match && (state_kind == InstrumentStateKind::NoneOrAny || state_kind == map_entry.maybe_instrument_state_kind);

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
