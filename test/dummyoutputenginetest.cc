/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            dummyoutputenginetest.cc
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

#ifdef HAVE_OUTPUT_DUMMY
#include "drumgizmo/output/outputdummy.h"
#include <channel.h>

// =============================================================================
// DummyOutputEngine Tests
// =============================================================================

TEST_CASE("DummyOutputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		DummyOutputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("initReturnsTrue")
	{
		DummyOutputEngine engine;
		Channels channels;

		bool result = engine.init(channels);

		CHECK_UNARY(result);
	}

	SUBCASE("initWithChannelsReturnsTrue")
	{
		DummyOutputEngine engine;
		Channels channels;
		Channel channel1("channel1");
		channels.push_back(channel1);

		bool result = engine.init(channels);

		CHECK_UNARY(result);
	}

	SUBCASE("setParmDoesNotThrow")
	{
		DummyOutputEngine engine;

		// Should not throw or crash
		engine.setParm("any_param", "any_value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("startReturnsTrue")
	{
		DummyOutputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		DummyOutputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		DummyOutputEngine engine;

		engine.pre(1024);
		engine.pre(512);
		engine.pre(0);

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		DummyOutputEngine engine;

		engine.post(1024);
		engine.post(512);
		engine.post(0);

		CHECK_UNARY(true);
	}

	SUBCASE("runDoesNotThrow")
	{
		DummyOutputEngine engine;
		sample_t samples[1024] = {0};

		engine.run(0, samples, 1024);
		engine.run(1, samples, 512);

		CHECK_UNARY(true);
	}

	SUBCASE("getSamplerateReturns44100")
	{
		DummyOutputEngine engine;

		std::size_t rate = engine.getSamplerate();

		CHECK_EQ(rate, 44100);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		DummyOutputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("getBufferReturnsNull")
	{
		DummyOutputEngine engine;

		sample_t* buffer = engine.getBuffer(0);

		CHECK_UNARY(buffer == nullptr);
	}

	SUBCASE("getBufferSizeReturns1024")
	{
		DummyOutputEngine engine;

		std::size_t size = engine.getBufferSize();

		CHECK_EQ(size, 1024);
	}

	SUBCASE("fullLifecycleWorks")
	{
		DummyOutputEngine engine;
		Channels channels;
		sample_t samples[1024] = {0};

		CHECK_UNARY(engine.init(channels));
		engine.setParm("test", "value");
		CHECK_UNARY(engine.start());
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);
		engine.stop();

		CHECK_EQ(engine.getSamplerate(), 44100);
		CHECK_UNARY(engine.isFreewheeling());
	}
}

// =============================================================================
// DummyOutputEngine Edge Cases
// =============================================================================

TEST_CASE("DummyOutputEngineEdgeCases")
{
	SUBCASE("fullLifecycleMultipleTimes")
	{
		DummyOutputEngine engine;
		Channels channels;
		sample_t samples[1024] = {0};

		for(int i = 0; i < 5; ++i)
		{
			CHECK_UNARY(engine.init(channels));
			CHECK_UNARY(engine.start());
			engine.pre(1024);
			engine.run(0, samples, 1024);
			engine.post(1024);
			engine.stop();
		}

		CHECK_EQ(engine.getSamplerate(), 44100);
	}

	SUBCASE("getBufferWithVariousIndicesReturnsNull")
	{
		DummyOutputEngine engine;

		CHECK_UNARY(engine.getBuffer(0) == nullptr);
		CHECK_UNARY(engine.getBuffer(1) == nullptr);
		CHECK_UNARY(engine.getBuffer(100) == nullptr);
	}
}

#endif // HAVE_OUTPUT_DUMMY
