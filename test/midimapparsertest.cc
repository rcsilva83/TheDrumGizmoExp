/* -*- Mode: c++ -*- */
/***************************************************************************
 *            midimapparsertest.cc
 *
 *  Wed Jul 25 20:37:23 CEST 2018
 *  Copyright 2018 Bent Bisballe Nyeng
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
#include <uunit.h>

#include <midimapparser.h>
#include <curvemap.h>

#include "scopedfile.h"

class MidimapParserTest
	: public uUnit
{
public:
	MidimapParserTest()
	{
		uTEST(MidimapParserTest::test_basic);
		uTEST(MidimapParserTest::test_curve);
		uTEST(MidimapParserTest::test_invalid);
	}

	void test_basic()
	{
		ScopedFile scoped_file(
			"<?xml version='1.0' encoding='UTF-8'?>\n" \
			"<midimap>\n" \
			"	<map note=\"54\" instr=\"Crash_left_tip\"/>\n" \
			"	<map note=\"60\" instr=\"Crash_left_whisker\"/>\n" \
			"	<map note=\"55\" instr=\"Crash_right_tip\"/>\n" \
			"	<map note=\"62\" instr=\"Crash_right_whisker\"/>\n" \
			"	<map note=\"62\" instr=\"Hihat_closed\"/>\n" \
			"	<map note=\"56\" instr=\"Hihat_closed\"/>\n" \
			"</midimap>");

		MidiMapParser parser;
		uASSERT(parser.parseFile(scoped_file.filename()));

		const auto& midimap = parser.midimap;
		uASSERT_EQUAL(6u, midimap.size());

		uASSERT_EQUAL(54, midimap[0].note_id);
		uASSERT_EQUAL(std::string("Crash_left_tip"), midimap[0].instrument_name);

		uASSERT_EQUAL(60, midimap[1].note_id);
		uASSERT_EQUAL(std::string("Crash_left_whisker"), midimap[1].instrument_name);

		uASSERT_EQUAL(55, midimap[2].note_id);
		uASSERT_EQUAL(std::string("Crash_right_tip"), midimap[2].instrument_name);

		// These next two note numbers are intentionally the same and trigger two
		// different instruments:
		uASSERT_EQUAL(62, midimap[3].note_id);
		uASSERT_EQUAL(std::string("Crash_right_whisker"), midimap[3].instrument_name);

		uASSERT_EQUAL(62, midimap[4].note_id);
		uASSERT_EQUAL(std::string("Hihat_closed"), midimap[4].instrument_name);

		uASSERT_EQUAL(56, midimap[5].note_id);
		uASSERT_EQUAL(std::string("Hihat_closed"), midimap[5].instrument_name);
	}

	void test_curve()
	{
		ScopedFile scoped_file(
			"<?xml version='1.0' encoding='UTF-8'?>\n" \
			"<midimap>\n" \
			"	<map note=\"56\" instr=\"Hihat_closed\"/>\n" \
			"   <map note=\"40\" instr=\"Kick\">\n" \
			"       <curve in0=\"0.1\" out0=\"0.2\" in1=\"0.5\" out1=\"0.6\" in2=\"0.8\" out2=\"0.9\" invert=\"true\" shelf=\"false\"/>\n" \
			"   </map>\n" \
			"   <map note=\"41\" instr=\"Snare\">\n" \
			"       <curve/>\n" \
			"   </map>\n" \
			"</midimap>");

		MidiMapParser parser;
		uASSERT(parser.parseFile(scoped_file.filename()));

		const auto& midimap = parser.midimap;
		uASSERT_EQUAL(3u, midimap.size());

		uASSERT_EQUAL(56, midimap[0].note_id);
		uASSERT(!midimap[0].maybe_curve_map);

		uASSERT_EQUAL(40, midimap[1].note_id);
		uASSERT(midimap[1].maybe_curve_map.get());
		uASSERT_EQUAL(true, midimap[1].maybe_curve_map->getInvert());
		uASSERT_EQUAL(false, midimap[1].maybe_curve_map->getShelf());
		uASSERT_EQUAL(0.1, midimap[1].maybe_curve_map->getFixed0().in);
		uASSERT_EQUAL(0.2, midimap[1].maybe_curve_map->getFixed0().out);
		uASSERT_EQUAL(0.5, midimap[1].maybe_curve_map->getFixed1().in);
		uASSERT_EQUAL(0.6, midimap[1].maybe_curve_map->getFixed1().out);
		uASSERT_EQUAL(0.8, midimap[1].maybe_curve_map->getFixed2().in);
		uASSERT_EQUAL(0.9, midimap[1].maybe_curve_map->getFixed2().out);

		uASSERT_EQUAL(41, midimap[2].note_id);
		CurveMap reference_map;
		uASSERT(midimap[2].maybe_curve_map.get());
		uASSERT(reference_map == *midimap[2].maybe_curve_map);
	}

	void test_invalid()
	{
		ScopedFile scoped_file(
			"<?xml version='1.0' encoding='UTF-8'?>\n" \
			"<midimap\n" \
			"	<map note=\"54\" instr=\"Crash_left_tip\"/>\n" \
			"	<map note=\"60\" instr=\"Crash_left_whisker\"/>\n" \
			"	<map note=\"55\" instr=\"Crash_right_tip\"/>\n" \
			"	<map note=\"62\" instr=\"Crash_right_whisker\"/>\n" \
			"	<map note=\"56\" instr=\"Hihat_closed\"/>\n" \
			"</midimap>");

		MidiMapParser parser;
		uASSERT(!parser.parseFile(scoped_file.filename()));
	}
};

// Registers the fixture into the 'registry'
static MidimapParserTest test;
