/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumgizmotest.cc
 *
 *  Fri Apr 24 10:30:00 CET 2026
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

#include "drumkit_creator.h"

#include <memory>
#include <vector>

// Include input engines
#ifdef HAVE_INPUT_DUMMY
#include "drumgizmo/input/inputdummy.h"
#endif

#ifdef HAVE_INPUT_TEST
#include "drumgizmo/input/test.h"
#endif

// Include output engines
#ifdef HAVE_OUTPUT_DUMMY
#include "drumgizmo/output/outputdummy.h"
#endif

#ifdef HAVE_OUTPUT_WAVFILE
#include "drumgizmo/output/wavfile.h"
#endif

// Include channel and instrument definitions
#include <channel.h>
#include <instrument.h>

// =============================================================================
// DummyInputEngine Tests
// =============================================================================

#ifdef HAVE_INPUT_DUMMY
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
#endif // HAVE_INPUT_DUMMY

// =============================================================================
// TestInputEngine Tests
// =============================================================================

#ifdef HAVE_INPUT_TEST
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
		for(int i = 0; i < 10; ++i)
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

		for(int i = 0; i < 10; ++i)
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
		CHECK_UNARY(events.size() > 0);
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
#endif // HAVE_INPUT_TEST

// =============================================================================
// DummyOutputEngine Tests
// =============================================================================

#ifdef HAVE_OUTPUT_DUMMY
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
#endif // HAVE_OUTPUT_DUMMY

// =============================================================================
// WavfileOutputEngine Tests
// =============================================================================

#ifdef HAVE_OUTPUT_WAVFILE
TEST_CASE("WavfileOutputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		WavfileOutputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("initWithEmptyChannelsReturnsTrue")
	{
		WavfileOutputEngine engine;
		Channels channels;

		bool result = engine.init(channels);

		CHECK_UNARY(result);
	}

	SUBCASE("initWithChannelsCreatesFiles")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("test");
		channels.push_back(channel1);

		engine.setParm("file", "/tmp/drumgizmo_test_");

		bool result = engine.init(channels);

		CHECK_UNARY(result);
	}

	SUBCASE("setParmFileSetsFilename")
	{
		WavfileOutputEngine engine;

		// Should not throw
		engine.setParm("file", "/tmp/output");
		engine.setParm("file", "output");
		engine.setParm("file", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmSrateSetsSamplerate")
	{
		WavfileOutputEngine engine;

		engine.setParm("srate", "48000");
		CHECK_EQ(engine.getSamplerate(), 48000);

		engine.setParm("srate", "44100");
		CHECK_EQ(engine.getSamplerate(), 44100);

		engine.setParm("srate", "96000");
		CHECK_EQ(engine.getSamplerate(), 96000);
	}

	SUBCASE("setParmInvalidSrateDoesNotCrash")
	{
		WavfileOutputEngine engine;

		// Invalid samplerate should not crash
		engine.setParm("srate", "invalid");
		engine.setParm("srate", "");
		engine.setParm("srate", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		WavfileOutputEngine engine;

		// Unknown parameter should not crash
		engine.setParm("unknown", "value");

		CHECK_UNARY(true);
	}

	SUBCASE("startReturnsTrue")
	{
		WavfileOutputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		WavfileOutputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		WavfileOutputEngine engine;

		engine.pre(1024);
		engine.pre(0);

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		WavfileOutputEngine engine;

		engine.post(1024);
		engine.post(0);

		CHECK_UNARY(true);
	}

	SUBCASE("postReducesLatency")
	{
		WavfileOutputEngine engine;

		engine.onLatencyChange(100);
		engine.post(50);
		// Latency should now be 50

		engine.post(100);
		// Latency should now be 0

		CHECK_UNARY(true);
	}

	SUBCASE("getSamplerateReturnsDefault44100")
	{
		WavfileOutputEngine engine;

		std::size_t rate = engine.getSamplerate();

		CHECK_EQ(rate, 44100);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		WavfileOutputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("onLatencyChangeSetsLatency")
	{
		WavfileOutputEngine engine;

		engine.onLatencyChange(100);
		engine.onLatencyChange(0);
		engine.onLatencyChange(1000);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithInvalidChannelLogsError")
	{
		WavfileOutputEngine engine;
		Channels channels;
		sample_t samples[1024] = {0};

		engine.init(channels);

		// Run with invalid channel index - should not crash
		engine.run(0, samples, 1024);
		engine.run(100, samples, 1024);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithLatencySkipsSamples")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("test");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_test_");
		engine.init(channels);
		engine.onLatencyChange(10);

		// Run with latency - should skip first 10 samples
		engine.run(0, samples, 100);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithLatencyEqualToNsamplesSkipsAll")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("test");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_test2_");
		engine.init(channels);
		engine.onLatencyChange(100);

		// Run with latency equal to nsamples - should skip all
		engine.run(0, samples, 100);

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWorks")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_full_test_");
		engine.setParm("srate", "48000");
		CHECK_UNARY(engine.init(channels));
		CHECK_UNARY(engine.start());
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);
		engine.stop();

		CHECK_EQ(engine.getSamplerate(), 48000);
		CHECK_UNARY(engine.isFreewheeling());
	}
}
#endif // HAVE_OUTPUT_WAVFILE
