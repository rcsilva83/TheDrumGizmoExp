/* -*- Mode: c++ -*- */
/***************************************************************************
 *            inputprocessortest.cc
 *
 *  Tue Mar 24 2026
 *  Copyright 2026 The DrumGizmo Authors
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
#include <doctest/doctest.h>

#include <event.h>
#include <inputprocessor.h>
#include <drumkit.h>
#include <events_ds.h>
#include <random.h>
#include <settings.h>

TEST_CASE("InputProcessorTest")
{
	SUBCASE("empty_event_list_returns_true")
	{
		// Processing an empty event list on an empty kit must succeed (true).
		Settings settings;
		DrumKit kit;
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		std::vector<event_t> events;
		CHECK(proc.process(events, 0, 1.0));
	}

	SUBCASE("processOnset_out_of_range_instrument_skips_event")
	{
		// When the instrument_id in an OnSet event exceeds the number of
		// instruments in the kit, processOnset returns false and the event is
		// skipped.  The call to process() itself still returns true (loop
		// continues for the remaining events).
		Settings settings;
		DrumKit kit; // no instruments loaded
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		// instrument 99 does not exist in an empty kit
		event_t event{EventType::OnSet, 99, 0, 0.8f};
		std::vector<event_t> events{event};
		CHECK(proc.process(events, 0, 1.0));

		// No sample events should have been added
		CHECK_EQ(0u, events_ds.numberOfEvents(0));
	}

	SUBCASE("processChoke_out_of_range_instrument_skips_event")
	{
		// Same contract for a Choke event: missing instrument is skipped and
		// process() returns true.
		Settings settings;
		DrumKit kit;
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		event_t event{EventType::Choke, 99, 0, 0.8f};
		std::vector<event_t> events{event};
		CHECK(proc.process(events, 0, 1.0));
	}

	SUBCASE("processStop_stops_engine_when_no_active_events")
	{
		// A Stop event sets the is_stopping flag.  When there are no active
		// sample events, processStop should return false, which propagates as
		// a false return from process() to signal the engine to halt.
		Settings settings;
		DrumKit kit;
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		event_t stop_event{EventType::Stop, 0, 0, 0.0f};
		std::vector<event_t> events{stop_event};
		// No active events exist, so the engine should signal a stop.
		CHECK(!proc.process(events, 0, 1.0));
	}

	SUBCASE("processStop_keeps_running_with_active_events")
	{
		// When there are active sample events and a Stop event arrives,
		// process() returns true to keep the engine running until the events
		// drain.
		Settings settings;
		DrumKit kit;
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		// Inject a fake active sample event on channel 0 so numberOfEvents > 0.
		events_ds.startAddingNewGroup(0);
		events_ds.emplace<SampleEvent>(0, 0, 1.0, nullptr, "", 0);

		// kit has no channels, so the channel loop in processStop skips all
		// channels, resulting in num_active_events == 0 → still stops.
		// To observe the "keep running" branch we need at least one kit channel
		// whose num < NUM_CHANNELS.
		kit.channels.emplace_back();
		kit.channels.back().name = "ch0";
		kit.channels.back().num = 0;

		event_t stop_event{EventType::Stop, 0, 0, 0.0f};
		std::vector<event_t> events{stop_event};
		// Active event exists on channel 0, so engine should keep running.
		CHECK(proc.process(events, 0, 1.0));
	}

	SUBCASE("getLatency_zero_by_default")
	{
		// With all filters disabled (default settings), combined latency is 0.
		Settings settings;
		DrumKit kit;
		EventsDS events_ds;
		Random random(1234);
		InputProcessor proc(settings, kit, events_ds, random);

		CHECK_EQ(0u, proc.getLatency());
	}
}
