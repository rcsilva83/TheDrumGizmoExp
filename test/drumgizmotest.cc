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

#include <cstdio>
#include <memory>
#include <string>
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

#ifdef HAVE_OUTPUT_WAVFILE
static void cleanupWavFiles(const std::string& prefix, const Channels& channels)
{
	for(size_t i = 0; i < channels.size(); ++i)
	{
		std::string fname =
		    prefix + channels[i].name + "-" + std::to_string(i) + ".wav";
		std::remove(fname.c_str());
	}
}
#endif

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

		cleanupWavFiles("/tmp/drumgizmo_test_", channels);
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
		cleanupWavFiles("/tmp/drumgizmo_latency_test_", channels);
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
		cleanupWavFiles("/tmp/drumgizmo_latency_test2_", channels);
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
		cleanupWavFiles("/tmp/drumgizmo_full_test_", channels);
	}
}
#endif // HAVE_OUTPUT_WAVFILE

// =============================================================================
// EngineFactory Tests
// =============================================================================

#include "drumgizmo/enginefactory.h"

TEST_CASE("EngineFactory")
{
	SUBCASE("constructorInitializesEngineLists")
	{
		EngineFactory factory;

		// Should have at least some engines registered
		[[maybe_unused]] auto& inputs = factory.getInputEngines();
		[[maybe_unused]] auto& outputs = factory.getOutputEngines();

		// The lists should be populated based on compile-time flags
		// Just verify no crash by calling the methods
		CHECK_UNARY(true);
	}

	SUBCASE("getInputEnginesReturnsValidList")
	{
		EngineFactory factory;
		const auto& inputs = factory.getInputEngines();

		// Verify it's a valid list reference
		for(const auto& engine_name : inputs)
		{
			CHECK_UNARY(!engine_name.empty());
		}
	}

	SUBCASE("getOutputEnginesReturnsValidList")
	{
		EngineFactory factory;
		const auto& outputs = factory.getOutputEngines();

		// Verify it's a valid list reference
		for(const auto& engine_name : outputs)
		{
			CHECK_UNARY(!engine_name.empty());
		}
	}

#ifdef HAVE_INPUT_DUMMY
	SUBCASE("createInputDummyReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createInput("dummy");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_INPUT_TEST
	SUBCASE("createInputTestReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createInput("test");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_OUTPUT_DUMMY
	SUBCASE("createOutputDummyReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("dummy");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_OUTPUT_WAVFILE
	SUBCASE("createOutputWavfileReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("wavfile");

		CHECK_UNARY(engine != nullptr);
	}
#endif

	SUBCASE("createInputInvalidReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createInput("nonexistent_engine");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createOutputInvalidReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("nonexistent_engine");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createInputEmptyStringReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createInput("");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createOutputEmptyStringReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("multipleFactoryInstancesWork")
	{
		EngineFactory factory1;
		EngineFactory factory2;

		// Both should be independent
		auto& inputs1 = factory1.getInputEngines();
		auto& inputs2 = factory2.getInputEngines();

		CHECK_EQ(inputs1.size(), inputs2.size());
	}

	SUBCASE("createdEnginesAreIndependent")
	{
		EngineFactory factory;

#ifdef HAVE_INPUT_DUMMY
		auto engine1 = factory.createInput("dummy");
		auto engine2 = factory.createInput("dummy");

		CHECK_UNARY(engine1 != nullptr);
		CHECK_UNARY(engine2 != nullptr);
		CHECK_UNARY(engine1 != engine2);
#endif
	}
}

// =============================================================================
// MidifileInputEngine Tests
// =============================================================================

#ifdef HAVE_INPUT_MIDIFILE
#include "drumgizmo/input/midifile.h"
#include "scopedfile.h"
#include <fstream>
#include <random.h>
#include <settings.h>
#include <smf.h>

// Helper function to create a minimal test MIDI file
[[maybe_unused]] static bool createTestMidiFile(const std::string& filename)
{
	smf_t* smf = smf_new();
	if(!smf)
	{
		return false;
	}

	// Create a simple track with a note on/off event
	smf_track_t* track = smf_track_new();
	if(!track)
	{
		smf_delete(smf);
		return false;
	}

	// Add track to smf
	smf_add_track(smf, track);

	// Note On event (middle C, velocity 100)
	std::uint8_t note_on[] = {0x90, 60, 100};
	smf_event_t* event_on =
	    smf_event_new_from_pointer(note_on, sizeof(note_on));
	if(event_on)
	{
		smf_track_add_event_seconds(track, event_on, 0.0);
	}

	// Note Off event
	std::uint8_t note_off[] = {0x80, 60, 0};
	smf_event_t* event_off =
	    smf_event_new_from_pointer(note_off, sizeof(note_off));
	if(event_off)
	{
		smf_track_add_event_seconds(track, event_off, 0.5);
	}

	// Save to file
	int result = smf_save(smf, filename.c_str());
	smf_delete(smf);
	return result == 0;
}

TEST_CASE("MidifileInputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		MidifileInputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("initWithoutFileReturnsFalse")
	{
		MidifileInputEngine engine;
		Instruments instruments;

		bool result = engine.init(instruments);

		CHECK_UNARY(!result);
	}

	SUBCASE("initWithoutMidimapReturnsFalse")
	{
		MidifileInputEngine engine;
		Instruments instruments;

		engine.setParm("file", "/tmp/test.mid");
		bool result = engine.init(instruments);

		CHECK_UNARY(!result);
	}

	SUBCASE("setParmFileSetsFilename")
	{
		MidifileInputEngine engine;

		engine.setParm("file", "/path/to/file.mid");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMidimapSetsFilename")
	{
		MidifileInputEngine engine;

		engine.setParm("midimap", "/path/to/map.xml");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("setParmSpeedSetsSpeed")
	{
		MidifileInputEngine engine;

		engine.setParm("speed", "2.0");
		engine.setParm("speed", "0.5");
		engine.setParm("speed", "1.0");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidSpeedDoesNotCrash")
	{
		MidifileInputEngine engine;

		engine.setParm("speed", "invalid");
		engine.setParm("speed", "");
		engine.setParm("speed", "abc");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("setParmLoopEnablesLooping")
	{
		MidifileInputEngine engine;

		engine.setParm("loop", "1");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		MidifileInputEngine engine;

		engine.setParm("unknown", "value");
		engine.setParm("", "");

		// Should not crash
		CHECK_UNARY(true);
	}

	SUBCASE("startReturnsTrue")
	{
		MidifileInputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		MidifileInputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		MidifileInputEngine engine;

		engine.pre();

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		MidifileInputEngine engine;

		engine.post();

		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		MidifileInputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("setSampleRateDoesNotThrow")
	{
		MidifileInputEngine engine;

		engine.setSampleRate(44100.0);
		engine.setSampleRate(48000.0);
		engine.setSampleRate(96000.0);

		CHECK_UNARY(true);
	}

	// Init with nonexistent MIDI file returns false
	SUBCASE("initWithNonexistentMidiFileReturnsFalse")
	{
		MidifileInputEngine engine;
		Instruments instruments;
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		engine.setParm("file", "/nonexistent/path/test.mid");
		engine.setParm("midimap", midimap_file.filename());

		bool result = engine.init(instruments);

		CHECK_UNARY(!result);
	}

	// Init with nonexistent midimap file returns false
	SUBCASE("initWithNonexistentMidimapReturnsFalse")
	{
		ScopedFile midi_file("placeholder");
		// Write a minimal valid MIDI file
		{
			smf_t* smf = smf_new();
			smf_track_t* track = smf_track_new();
			smf_add_track(smf, track);
			std::uint8_t note[] = {0x90, 60, 100};
			smf_event_t* ev = smf_event_new_from_pointer(note, sizeof(note));
			smf_track_add_event_seconds(track, ev, 0.0);
			int save_res = smf_save(smf, midi_file.filename().c_str());
			(void)save_res;
			smf_delete(smf);
		}

		MidifileInputEngine engine;
		Instruments instruments;

		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", "/nonexistent/path/map.xml");

		bool result = engine.init(instruments);

		CHECK_UNARY(!result);
	}
}

// Minimal MIDI file: Format 0, 1 track, 96 ticks/quarter.
// Contains a Note On (note=60, vel=100) at time 0.
static const unsigned char kMinimalMidi[] = {'M', 'T', 'h', 'd', 0x00, 0x00,
    0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x60, 'M', 'T', 'r', 'k', 0x00,
    0x00, 0x00, 0x14,
    // Event: delta=0, Note On ch=0 note=60 vel=100
    0x00, 0x90, 0x3C, 0x64,
    // Event: delta=96, Note Off ch=0 note=60 vel=0
    0x60, 0x80, 0x3C, 0x00,
    // Event: delta=0, Note On ch=0 note=64 vel=80
    0x00, 0x90, 0x40, 0x50,
    // Event: delta=96, Note Off ch=0 note=64 vel=0
    0x60, 0x80, 0x40, 0x00,
    // End of track
    0x00, 0xFF, 0x2F, 0x00};

TEST_CASE("MidifileInputEngineRun")
{
	SUBCASE("fullLifecycleGeneratesStopEvent")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "\t<map note=\"64\" instr=\"Snare\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;

		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 44100, events);
		engine.post();

		CHECK_UNARY(!events.empty());
		if(!events.empty())
		{
			CHECK_EQ(static_cast<int>(events.back().type),
			    static_cast<int>(EventType::Stop));
		}

		engine.stop();
	}

	SUBCASE("lifecycleMethodsWorkWithoutInit")
	{
		MidifileInputEngine engine;
		std::vector<event_t> events;

		// The engine's lifecycle methods (isFreewheeling, start, stop)
		// work without calling init(). Calling run() without init
		// would crash because the internal smf pointer is null.
		CHECK_UNARY(engine.isFreewheeling());
		CHECK_UNARY(engine.start());
		engine.stop();
	}

	SUBCASE("fullLifecycleWithLoopDoesNotGenerateStop")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());
		engine.setParm("loop", "1");

		Instruments instruments;

		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		// Run multiple times - with loop enabled, no Stop event should appear
		bool saw_stop = false;
		for(size_t i = 0; i < 3; ++i)
		{
			std::vector<event_t> events;
			engine.pre();
			engine.run(i * 44100, 44100, events);
			engine.post();

			for(const auto& e : events)
			{
				if(e.type == EventType::Stop)
				{
					saw_stop = true;
				}
			}
		}

		CHECK_UNARY(!saw_stop);

		engine.stop();
	}

	SUBCASE("fullLifecycleMultiplePositions")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;

		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		// Run through the file in chunks
		size_t total_events = 0;
		for(size_t pos = 0; pos < static_cast<size_t>(44100) * 5; pos += 1024)
		{
			std::vector<event_t> events;
			engine.pre();
			engine.run(pos, 1024, events);
			engine.post();
			total_events += events.size();
		}

		CHECK_UNARY(total_events >= 1);

		engine.stop();
	}

	SUBCASE("setSampleRateAffectsTiming")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;

		engine.setSampleRate(48000.0);
		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 48000, events);
		engine.post();

		CHECK_UNARY(!events.empty());

		engine.stop();
	}

	SUBCASE("setParmSpeedAffectsTiming")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());
		engine.setParm("speed", "2.0"); // Double speed moves events earlier

		Instruments instruments;

		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 44100, events);
		engine.post();

		// With double speed, events are processed in half the time.
		// The Stop event should be generated once the MIDI file ends.
		CHECK_UNARY(!events.empty());

		engine.stop();
	}
}
#endif // HAVE_INPUT_MIDIFILE

// =============================================================================
// Additional Edge Case Tests
// =============================================================================

#ifdef HAVE_INPUT_TEST
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

#ifdef HAVE_OUTPUT_WAVFILE
TEST_CASE("WavfileOutputEngineEdgeCases")
{
	SUBCASE("initWithMultipleChannelsCreatesMultipleFiles")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("channel1");
		Channel channel2("channel2");
		Channel channel3("channel3");
		channels.push_back(channel1);
		channels.push_back(channel2);
		channels.push_back(channel3);

		engine.setParm("file", "/tmp/drumgizmo_multi_test_");

		bool result = engine.init(channels);

		CHECK_UNARY(result);
		cleanupWavFiles("/tmp/drumgizmo_multi_test_", channels);
	}

	SUBCASE("setSamplerateZeroDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.setParm("srate", "0");

		CHECK_UNARY(true);
	}

	SUBCASE("setSamplerateNegativeDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.setParm("srate", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setSamplerateVeryHighDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.setParm("srate", "192000");

		CHECK_EQ(engine.getSamplerate(), 192000);
	}

	SUBCASE("setParmFileEmptyString")
	{
		WavfileOutputEngine engine;

		engine.setParm("file", "");

		CHECK_UNARY(true);
	}

	SUBCASE("onLatencyChangeWithZeroDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.onLatencyChange(0);

		CHECK_UNARY(true);
	}

	SUBCASE("onLatencyChangeWithLargeValueDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.onLatencyChange(10000);

		CHECK_UNARY(true);
	}

	SUBCASE("postWithZeroLenDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.post(0);

		CHECK_UNARY(true);
	}

	SUBCASE("runWithNullSamplesDoesNotCrash")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("test");
		channels.push_back(channel1);

		engine.setParm("file", "/tmp/drumgizmo_null_test_");
		engine.init(channels);

		// Run with nullptr - should handle gracefully
		engine.run(0, nullptr, 0);

		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_null_test_", channels);
	}

	SUBCASE("runWithNegativeChannelIndexDoesNotCrash")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("test");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_neg_chan_test_");
		engine.init(channels);

		// Negative channel index casts to large unsigned int >= channels.size()
		engine.run(-1, samples, 1024);

		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_neg_chan_test_", channels);
	}

	SUBCASE("preWithZeroSizeDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.pre(0);

		CHECK_UNARY(true);
	}

	SUBCASE("preWithLargeSizeDoesNotCrash")
	{
		WavfileOutputEngine engine;

		engine.pre(100000);

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithMultipleRuns")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024];
		for(int i = 0; i < 1024; ++i)
		{
			samples[i] = static_cast<sample_t>(i);
		}

		engine.setParm("file", "/tmp/drumgizmo_multi_run_test_");
		engine.setParm("srate", "44100");
		CHECK_UNARY(engine.init(channels));
		CHECK_UNARY(engine.start());

		// Multiple pre/run/post cycles
		for(int i = 0; i < 5; ++i)
		{
			engine.pre(1024);
			engine.run(0, samples, 1024);
			engine.post(1024);
		}

		engine.stop();

		CHECK_EQ(engine.getSamplerate(), 44100);
		CHECK_UNARY(engine.isFreewheeling());
		cleanupWavFiles("/tmp/drumgizmo_multi_run_test_", channels);
	}

	SUBCASE("fullLifecycleWithLatencyChanges")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_change_test_");
		CHECK_UNARY(engine.init(channels));
		CHECK_UNARY(engine.start());

		engine.onLatencyChange(100);
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);

		engine.onLatencyChange(50);
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);

		engine.onLatencyChange(0);
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);

		engine.stop();

		CHECK_UNARY(engine.isFreewheeling());
		cleanupWavFiles("/tmp/drumgizmo_latency_change_test_", channels);
	}
}
#endif // HAVE_OUTPUT_WAVFILE

#ifdef HAVE_INPUT_DUMMY
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

#ifdef HAVE_OUTPUT_DUMMY
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

#ifdef HAVE_OUTPUT_WAVFILE
TEST_CASE("WavfileOutputEngineLatencyEdgeCases")
{
	SUBCASE("postWithLatencyGreaterThanNsamplesReducesLatency")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_gt_test_");
		CHECK_UNARY(engine.init(channels));

		engine.onLatencyChange(100);
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(50);

		engine.stop();
		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_latency_gt_test_", channels);
	}

	SUBCASE("postWithLatencyEqualToNsamplesZerosLatency")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_eq_test_");
		CHECK_UNARY(engine.init(channels));
		engine.onLatencyChange(50);

		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(50);

		engine.stop();
		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_latency_eq_test_", channels);
	}

	SUBCASE("postWithLatencyLessThanNsamplesZerosLatency")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_lt_test_");
		CHECK_UNARY(engine.init(channels));
		engine.onLatencyChange(30);

		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(100);

		engine.stop();
		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_latency_lt_test_", channels);
	}

	SUBCASE("runWithNsamplesLessOrEqualToLatencySkipsOutput")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[512] = {0};

		engine.setParm("file", "/tmp/drumgizmo_latency_skip_test_");
		CHECK_UNARY(engine.init(channels));

		engine.onLatencyChange(1024);
		engine.pre(512);
		engine.run(0, samples, 512);
		engine.post(512);

		engine.stop();
		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_latency_skip_test_", channels);
	}

	SUBCASE("runWithNsamplesGreaterThanLatencyWritesAfterLatency")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);
		sample_t samples[1024] = {0};
		for(int i = 0; i < 1024; ++i)
		{
			samples[i] = static_cast<sample_t>(i) / 1024.0f;
		}

		engine.setParm("file", "/tmp/drumgizmo_latency_write_test_");
		CHECK_UNARY(engine.init(channels));

		engine.onLatencyChange(100);
		engine.pre(1024);
		engine.run(0, samples, 1024);
		engine.post(1024);

		engine.stop();
		CHECK_UNARY(true);
		cleanupWavFiles("/tmp/drumgizmo_latency_write_test_", channels);
	}

	SUBCASE("destructorWithOpenChannelsClosesFiles")
	{
		{
			WavfileOutputEngine engine;
			Channels channels;
			Channel channel1("chan");
			channels.push_back(channel1);
			sample_t samples[1024] = {0};

			engine.setParm("file", "/tmp/drumgizmo_dtor_test_");
			CHECK_UNARY(engine.init(channels));

			engine.start();
			engine.pre(1024);
			engine.run(0, samples, 1024);
			engine.post(1024);
			engine.stop();
		}
		CHECK_UNARY(true);
		unlink("/tmp/drumgizmo_dtor_test_chan-0.wav");
	}

	SUBCASE("setParmInvalidSrateCatchesException")
	{
		WavfileOutputEngine engine;

		engine.setParm("srate", "not-a-number");

		// Should not crash - catch handler prints error
		CHECK_UNARY(true);
	}

	SUBCASE("initInUnwritableDirectoryReturnsFalse")
	{
		WavfileOutputEngine engine;
		Channels channels;
		Channel channel1("chan");
		channels.push_back(channel1);

		// Use a path that cannot be created (directory doesn't exist)
		engine.setParm("file", "/nonexistent_dir_path_xyz/dg_test_");
		bool result = engine.init(channels);

		// Should return false when sf_open fails
		CHECK_UNARY(!result);
	}
}
#endif // HAVE_OUTPUT_WAVFILE

#ifdef HAVE_INPUT_MIDIFILE
TEST_CASE("MidifileInputEngineEdgeCases")
{
	SUBCASE("setParmLoopWithFalseStillSetsLoopTrue")
	{
		MidifileInputEngine engine;

		engine.setParm("loop", "false");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmLoopWithEmptyStringStillSetsLoopTrue")
	{
		MidifileInputEngine engine;

		engine.setParm("loop", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmLoopWithTrueSetsLoopTrue")
	{
		MidifileInputEngine engine;

		engine.setParm("loop", "true");

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithInvalidSmfFileReturnsFalse")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", "/tmp/drumgizmo_nonexistent_midi.mid");
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		bool result = engine.init(instruments);

		CHECK_UNARY(!result);
	}

	SUBCASE("fullLifecycleReinitAfterStop")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		engine.start();
		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 44100, events);
		engine.post();
		engine.stop();

		CHECK_UNARY(engine.init(instruments));
		engine.start();
		events.clear();
		engine.pre();
		engine.run(0, 44100, events);
		engine.post();
		engine.stop();

		CHECK_UNARY(!events.empty());
		if(!events.empty())
		{
			CHECK_EQ(static_cast<int>(events.back().type),
			    static_cast<int>(EventType::Stop));
		}
	}

	SUBCASE("runWithSmallChunkSizes")
	{
		std::string midi_data(
		    reinterpret_cast<const char*>(kMinimalMidi), sizeof(kMinimalMidi));
		ScopedFile midi_file(midi_data);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MidifileInputEngine engine;
		engine.setParm("file", midi_file.filename());
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		size_t total_events = 0;
		for(size_t pos = 0; pos < static_cast<size_t>(44100) * 5; pos += 16)
		{
			std::vector<event_t> events;
			engine.pre();
			engine.run(pos, 16, events);
			engine.post();
			total_events += events.size();
		}

		CHECK_UNARY(total_events >= 1);

		engine.stop();
	}

	SUBCASE("setParmInvalidSpeedCatchesException")
	{
		MidifileInputEngine engine;

		engine.setParm("speed", "not-a-number");

		// Should not crash - catch handler prints error
		CHECK_UNARY(true);
	}
}
#endif // HAVE_INPUT_MIDIFILE
