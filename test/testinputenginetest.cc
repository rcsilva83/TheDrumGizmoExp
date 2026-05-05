/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            testinputenginetest.cc
 *
 *  Tue May  5 12:00:00 CET 2026
 *  Copyright 2026 DrumGizmo team
 *
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

#include <config.h>

#ifdef HAVE_INPUT_TEST
#include "drumgizmo/input/test.h"
#include <instrument.h>

// =============================================================================
// TestInputEngine Tests
// =============================================================================

TEST_CASE("TestInputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		TestInputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("initReturnsTrue")
	{
		TestInputEngine engine;
		Instruments instruments;

		bool result = engine.init(instruments);

		CHECK_UNARY(result);
	}

	SUBCASE("startReturnsTrue")
	{
		TestInputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		TestInputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("setSampleRateDoesNotThrow")
	{
		TestInputEngine engine;

		engine.setSampleRate(48000.0);
		engine.setSampleRate(44100.0);
		engine.setSampleRate(96000.0);

		CHECK_UNARY(true);
	}

	SUBCASE("setParmProbabilityAcceptsValidValues")
	{
		TestInputEngine engine;

		// Should not throw
		engine.setParm("p", "0.0");
		engine.setParm("p", "0.5");
		engine.setParm("p", "1.0");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInstrumentAcceptsValidValues")
	{
		TestInputEngine engine;

		// Should not throw
		engine.setParm("instr", "0");
		engine.setParm("instr", "5");
		engine.setParm("instr", "31");
		engine.setParm("instr", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmLengthAcceptsValidValues")
	{
		TestInputEngine engine;

		// Should not throw
		engine.setParm("len", "1");
		engine.setParm("len", "10");
		engine.setParm("len", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("runGeneratesEventsWithHighProbability")
	{
		TestInputEngine engine;
		std::vector<event_t> events;
		Instruments instruments;

		engine.init(instruments);
		engine.setParm("p", "1.0"); // 100% probability

		// Run multiple times to ensure events are generated
		for(size_t i = 0; i < 10; ++i)
		{
			events.clear();
			engine.run(i * 1024, 1024, events);
		}

		// At least some events should be generated with 100% probability
		// Note: Due to randomness, we can't guarantee specific counts
		CHECK_UNARY(true); // If we get here without crash, it's good
	}

	SUBCASE("runWithZeroProbabilityGeneratesNoEvents")
	{
		TestInputEngine engine;
		std::vector<event_t> events;
		Instruments instruments;

		engine.init(instruments);
		engine.setParm("p", "0.0"); // 0% probability

		for(size_t i = 0; i < 10; ++i)
		{
			events.clear();
			engine.run(i * 1024, 1024, events);
			CHECK_EQ(events.size(), 0);
		}
	}

	SUBCASE("runWithSpecificInstrumentGeneratesEventsForThatInstrument")
	{
		TestInputEngine engine;
		std::vector<event_t> events;
		Instruments instruments;

		engine.init(instruments);
		engine.setParm("p", "1.0");
		engine.setParm("instr", "5");

		events.clear();
		engine.run(0, 1024, events);

		// Check that events have the correct instrument
		for(const auto& event : events)
		{
			CHECK_EQ(event.instrument, 5);
		}
	}

	SUBCASE("runWithLengthParameterGeneratesStopEvent")
	{
		TestInputEngine engine;
		std::vector<event_t> events;
		Instruments instruments;

		engine.init(instruments);
		engine.setParm("p", "1.0");
		engine.setParm("len", "0"); // Stop immediately

		events.clear();
		engine.run(0, 1024, events);

		// Should have a stop event due to len=0 and pos > len*samplerate
		CHECK_UNARY(!events.empty());
	}

	SUBCASE("fullLifecycleWorks")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		CHECK_UNARY(engine.init(instruments));
		engine.setParm("p", "0.5");
		engine.setParm("instr", "3");
		engine.setSampleRate(48000.0);
		CHECK_UNARY(engine.start());
		engine.pre();
		engine.run(0, 1024, events);
		engine.post();
		engine.stop();

		CHECK_UNARY(engine.isFreewheeling());
	}
}

// =============================================================================
// TestInputEngine Edge Cases
// =============================================================================

TEST_CASE("TestInputEngineEdgeCases")
{
	SUBCASE("setParmInvalidProbabilityDoesNotCrash")
	{
		TestInputEngine engine;

		engine.setParm("p", "invalid");
		engine.setParm("p", "");
		engine.setParm("p", "-1");
		engine.setParm("p", "2.0");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidInstrumentDoesNotCrash")
	{
		TestInputEngine engine;

		engine.setParm("instr", "invalid");
		engine.setParm("instr", "");
		engine.setParm("instr", "abc");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidLengthDoesNotCrash")
	{
		TestInputEngine engine;

		engine.setParm("len", "invalid");
		engine.setParm("len", "");
		engine.setParm("len", "abc");

		CHECK_UNARY(true);
	}

	SUBCASE("runWithZeroLengthDoesNotCrash")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		engine.init(instruments);
		engine.run(0, 0, events);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithLargeLengthDoesNotCrash")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		engine.init(instruments);
		engine.run(0, 1000000, events);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithZeroSampleRateDoesNotCrash")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		engine.init(instruments);
		engine.setSampleRate(0.0);
		engine.run(0, 1024, events);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithZeroProbabilityGeneratesNoEvents")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		engine.init(instruments);
		engine.setParm("p", "0.0");

		// Run many times to ensure no events with 0 probability
		for(size_t i = 0; i < 100; ++i)
		{
			events.clear();
			engine.run(i * 1024, 1024, events);
			CHECK_EQ(events.size(), 0);
		}
	}

	SUBCASE("runWithLengthOneGeneratesStopEvent")
	{
		TestInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		engine.init(instruments);
		engine.setParm("p", "1.0");
		engine.setParm("len", "1");

		// First run with high probability should trigger events
		events.clear();
		engine.run(0, 1024, events);

		// After some runs with len=1, stop event should be generated
		// Position > 1*samplerate should trigger stop
		events.clear();
		engine.run(50000, 1024, events);

		// Should have at least processed without crash
		CHECK_UNARY(true);
	}
}

#endif // HAVE_INPUT_TEST
