/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midimapper.h
 *
 *  Mon Jul 21 15:24:07 CEST 2008
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
#pragma once

#include <map>
#include <string>
#include <mutex>
#include <vector>
#include <memory>

#include "instrumentstate.h"

enum class MapFrom {
	Note,
	CC,
	NoneOrAny
};

enum class MapTo {
	PlayInstrument,
	InstrumentState,
	NoneOrAny
};

struct MidimapEntry
{
	MapFrom from_kind; // What kind of input is mapped from
	int     from_id;   // note or CC number
	MapTo   to_kind;   // What kind of action is mapped to

	std::string instrument_name;

	// For mapping to control state changes
	InstrumentStateKind maybe_instrument_state_kind;

	MidimapEntry &operator=(const MidimapEntry& other);
	MidimapEntry(const MidimapEntry& other);
	MidimapEntry(MapFrom from_kind,
	             int from_id,
				 MapTo to_kind,
				 std::string instrument_name,
				 InstrumentStateKind maybe_instrument_state_kind);
};

using midimap_t = std::vector<MidimapEntry>;
using instrmap_t = std::map<std::string, int>;

class MidiMapper
{
public:
	//! Lookup midi map entries matching the given query.
	std::vector<MidimapEntry> lookup(
		int     from_id = -1,              // note or cc #. -1 matches all notes/cc's
		MapFrom from_kind = MapFrom::NoneOrAny, // NoneOrAny will return both CC and note maps
		MapTo   to_kind = MapTo::NoneOrAny,     // NoneOrAny will return both instrument hits and controls
		InstrumentStateKind state_kind = InstrumentStateKind::NoneOrAny // NoneOrAny maps all state control kinds
		);

	int lookup_instrument(std::string name);

	//! Set new map sets.
	void swap(instrmap_t& instrmap, midimap_t& midimap);

	const midimap_t& getMap();

private:
	instrmap_t instrmap;
	midimap_t midimap;

	std::mutex mutex;
};
