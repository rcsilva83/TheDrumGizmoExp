/* -*- Mode: c++ -*- */
/***************************************************************************
 *            midimappertest.cc
 *
 *  Sun Aug 8 09:55:13 CEST 2021
 *  Copyright 2021 Bent Bisballe Nyeng
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA. */
#include <uunit.h>

#include <algorithm>
#include <vector>

#include <midimapper.h>

#include "scopedfile.h"

class MidiMapperTest
	: public uUnit
{
public:
	MidiMapperTest()
	{
		uTEST(MidiMapperTest::test);
		uTEST(MidiMapperTest::exceptional);
	}

	void test()
	{
		midimap_t midimap
		{
			MidimapEntry(54, "Crash_left_tip"),
			MidimapEntry(60, "Crash_left_whisker"),
			MidimapEntry(55, "Crash_right_tip"),
			MidimapEntry(62, "Crash_right_whisker"),
			MidimapEntry(62, "Hihat_closed"),
			MidimapEntry(56, "Hihat_closed"),
		};

		instrmap_t instrmap
		{
			{ "Crash_left_tip", 0 },
			{ "Crash_left_whisker", 1 },
			{ "Crash_right_tip", 2 },
			{ "Crash_right_whisker", 3 },
			{ "Hihat_closed", 4 },
		};

		MidiMapper mapper;
		mapper.swap(instrmap, midimap);

		{
			auto es = mapper.lookup(54);
			uASSERT_EQUAL(1u, es.size());

			auto i = mapper.lookup_instrument(es[0].instrument_name);
			uASSERT_EQUAL(0, i);
		}

		{
			auto es = mapper.lookup(60);
			uASSERT_EQUAL(1u, es.size());

			auto i = mapper.lookup_instrument(es[0].instrument_name);
			uASSERT_EQUAL(1, i);
		}

		{
			auto es = mapper.lookup(55);
			uASSERT_EQUAL(1u, es.size());

			auto i = mapper.lookup_instrument(es[0].instrument_name);
			uASSERT_EQUAL(2, i);
		}

		{
			auto es = mapper.lookup(62);
			uASSERT_EQUAL(2u, es.size());

			std::vector<int> is;
			is.push_back(mapper.lookup_instrument(es[0].instrument_name));
			is.push_back(mapper.lookup_instrument(es[1].instrument_name));

			// We don't care about the order, so just count the instances
			uASSERT_EQUAL(1u, std::count(is.begin(), is.end(), 3));
			uASSERT_EQUAL(1u, std::count(is.begin(), is.end(), 4));
		}

		{
			auto es = mapper.lookup(56);
			uASSERT_EQUAL(1u, es.size());

			auto i = mapper.lookup_instrument(es[0].instrument_name);
			uASSERT_EQUAL(4, i);
		}
	}

	void exceptional()
	{
		midimap_t midimap
		{
			MidimapEntry(54, "Crash_left_tip" ),
			MidimapEntry(60, "Crash_left_whisker_MISSING" ),
			MidimapEntry(55, "Crash_right_tip" ),
			MidimapEntry(62, "Crash_right_whisker" ),
			MidimapEntry(62, "Hihat_closed" ),
			MidimapEntry(56, "Hihat_closed" ),
		};

		instrmap_t instrmap
		{
			{ "Crash_left_tip", 0 },
			{ "Crash_left_whisker", 1 },
			{ "Crash_right_tip", 2 },
			{ "Crash_right_whisker", 3 },
			{ "Hihat_closed", 4 },
		};

		MidiMapper mapper;
		mapper.swap(instrmap, midimap);

		// no such note id
		{
			auto es = mapper.lookup(42);
			uASSERT_EQUAL(0u, es.size());
		}

		// no such instrument
		{
			auto es = mapper.lookup(60);
			uASSERT_EQUAL(1u, es.size());

			auto is = mapper.lookup_instrument(es[0].instrument_name);
			uASSERT_EQUAL(-1, is);
		}
	}
};

// Registers the fixture into the 'registry'
static MidiMapperTest test;
