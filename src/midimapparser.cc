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
#include "parsecurvemap.h"

#include <pugixml.hpp>
#include <hugin.hpp>

#include <cpp11fix.h>

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
		auto instr = std::string(map_node.attribute("instr").as_string());

		std::unique_ptr<CurveMap> maybe_curve;
		auto maybe_curve_node = map_node.child("curve");
		if (maybe_curve_node) {
			CurveMap curve;
			if (parse_curve_map (maybe_curve_node, curve)) {
				maybe_curve = std::make_unique<CurveMap>();
				*maybe_curve = curve;
			}
		}

		if(std::string(instr) == "" || note == default_int)
		{
			continue;
		}

		MidimapEntry entry(note, instr, maybe_curve.get());
		midimap.push_back(entry);
	}

	return true;
}
