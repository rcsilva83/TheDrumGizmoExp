/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midimapparser.cc
 *
 *  Mon Aug  8 16:55:30 CEST 2011
 *  Copyright 2011 Bent Bisballe Nyeng
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
#include "midimapparser.h"

#include <pugixml.hpp>
#include <hugin.hpp>

bool MidiMapParser::parseFile(const std::string& filename)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.data());
	if(result.status)
	{
		ERR(midimapparser, "XML parse error: %d", (int)result.offset);
		return false;
	}

	pugi::xml_node midimap_node = doc.child("midimap");
	for(pugi::xml_node map_node : midimap_node.children("map"))
	{
		constexpr int default_int   = 10000;
		auto note =  map_node.attribute("note").as_int(default_int);
		auto cc =    map_node.attribute("cc").as_int(default_int);
		auto instr = std::string(map_node.attribute("instr").as_string());
		auto control_min = map_node.attribute("min").as_int(default_int);
		auto control_max = map_node.attribute("max").as_int(default_int);
		auto control_str = std::string(map_node.attribute("control").as_string());
		auto control = (control_str == "openness" ? InstrumentStateKind::Openness :
		                control_str == "position" ? InstrumentStateKind::Position :
						InstrumentStateKind::None);

		bool is_conflict = (note != default_int && cc != default_int);
		bool is_note_play_instrument =
		   (!is_conflict && instr != "" && note != default_int);
		bool is_cc_control =
		   (!is_conflict && instr != "" && control != InstrumentStateKind::None && cc != default_int);

		if (is_note_play_instrument)
		{
			midimap.push_back(MidimapEntry {
				MapFrom::Note,
				MapTo::PlayInstrument,
				note,
				instr,
				InstrumentStateKind::None,
				0, 0
			});
		}
		else if (is_cc_control)
		{
			midimap.push_back(MidimapEntry {
				MapFrom::CC,
				MapTo::InstrumentState,
				cc,
				instr,
				control,
				(control_min != default_int) ? (uint8_t)control_min : (uint8_t)0,
				(control_max != default_int) ? (uint8_t)control_max : (uint8_t)127
			});
		}
	}

	return true;
}
