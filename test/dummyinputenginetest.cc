/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            dummyinputenginetest.cc
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

#ifdef HAVE_INPUT_DUMMY
#include "drumgizmo/input/inputdummy.h"
#include <instrument.h>

// =============================================================================
// DummyInputEngine Tests
// =============================================================================

TEST_CASE("DummyInputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		DummyInputEngine engine;

		// Just verify it compiles and constructs
		CHECK_UNARY(true);
	}

	SUBCASE("initReturnsTrue")
	{
		DummyInputEngine engine;
		Instruments instruments;

		bool result = engine.init(instruments);

		CHECK_UNARY(result);
	}

	SUBCASE("setParmDoesNotThrow")
	{
		DummyInputEngine engine;

		// Should not throw or crash
		engine.setParm("any_param", "any_value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("startReturnsTrue")
	{
		DummyInputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		DummyInputEngine engine;

		// Should not throw or crash
		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		DummyInputEngine engine;

		// Should not throw or crash
		engine.pre();

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		DummyInputEngine engine;

		// Should not throw or crash
		engine.post();

		CHECK_UNARY(true);
	}

	SUBCASE("runDoesNotGenerateEvents")
	{
		DummyInputEngine engine;
		std::vector<event_t> events;

		engine.run(0, 1024, events);

		CHECK_EQ(events.size(), 0);
	}

	SUBCASE("runWithDifferentPositionsDoesNotGenerateEvents")
	{
		DummyInputEngine engine;
		std::vector<event_t> events;

		engine.run(0, 512, events);
		CHECK_EQ(events.size(), 0);

		engine.run(512, 512, events);
		CHECK_EQ(events.size(), 0);

		engine.run(1024, 1024, events);
		CHECK_EQ(events.size(), 0);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		DummyInputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("fullLifecycleWorks")
	{
		DummyInputEngine engine;
		Instruments instruments;
		std::vector<event_t> events;

		// Full lifecycle
		CHECK_UNARY(engine.init(instruments));
		engine.setParm("test", "value");
		CHECK_UNARY(engine.start());
		engine.pre();
		engine.run(0, 1024, events);
		engine.post();
		engine.stop();

		CHECK_EQ(events.size(), 0);
		CHECK_UNARY(engine.isFreewheeling());
	}
}

TEST_CASE("DummyInputEngineEdgeCases")
{
	SUBCASE("runMultipleTimesDoesNotGenerateEvents")
	{
		DummyInputEngine engine;
		std::vector<event_t> events;

		// Run multiple times
		for(size_t i = 0; i < 100; ++i)
		{
			events.clear();
			engine.run(i * 1024, 1024, events);
			CHECK_EQ(events.size(), 0);
		}
	}

	SUBCASE("fullLifecycleMultipleTimes")
	{
		DummyInputEngine engine;
		Instruments instruments;

		// Run full lifecycle multiple times
		for(size_t i = 0; i < 5; ++i)
		{
			std::vector<event_t> events;

			CHECK_UNARY(engine.init(instruments));
			CHECK_UNARY(engine.start());
			engine.pre();
			engine.run(i * 1024, 1024, events);
			engine.post();
			engine.stop();

			CHECK_EQ(events.size(), 0);
		}

		CHECK_UNARY(engine.isFreewheeling());
	}
}

#endif // HAVE_INPUT_DUMMY
