/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumgizmointegrationtest.cc
 *
 *  Mon May  5 10:00:00 CEST 2026
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

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include <drumgizmo.h>
#include <settings.h>

#include "drumkit_creator.h"

#include "drumgizmo/enginefactory.h"

#ifdef HAVE_INPUT_MIDIFILE
#include "scopedfile.h"
#include <smf.h>
#endif

//! Wait for a drumkit to finish loading (or fail).
static bool waitForDrumkitLoad(Settings& settings, int timeout_ms = 5000)
{
	auto deadline = std::chrono::steady_clock::now() +
	                std::chrono::milliseconds(timeout_ms);

	while(std::chrono::steady_clock::now() < deadline)
	{
		auto status = settings.drumkit_load_status.load();
		if(status == LoadStatus::Done)
		{
			return true;
		}
		if(status == LoadStatus::Error)
		{
			return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return false;
}

//! Get a reasonable buffer size; some engines (e.g. JACK without a server)
//! return 0. Fall back to 1024 in that case.
static std::size_t safeBufferSize(AudioOutputEngine& oe)
{
	auto sz = oe.getBufferSize();
	return (sz > 0) ? sz : 1024;
}

struct DrumGizmoIntegrationFixture
{
	DrumkitCreator drumkit_creator;
	std::string kitfile;

	DrumGizmoIntegrationFixture()
	{
		kitfile = drumkit_creator.createStdKit("int_kit");
	}
};

// =============================================================================
// Integration tests for the full DrumGizmo pipeline with real engines.
// =============================================================================

TEST_CASE_FIXTURE(DrumGizmoIntegrationFixture, "DrumGizmoIntegration")
{
	// ---- Non-hardware engine integration tests ----

#if defined(HAVE_INPUT_DUMMY) && defined(HAVE_OUTPUT_DUMMY)
	SUBCASE("dummyInputDummyOutputFullPipeline")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("dummy");
		auto oe = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		REQUIRE(gizmo.init());
		REQUIRE(ie->start());
		REQUIRE(oe->start());

		std::size_t nsamples = oe->getBufferSize();
		std::vector<sample_t> buf(nsamples, 0.0f);

		// Run a few frames
		bool result = gizmo.run(0, buf.data(), nsamples);
		CHECK_UNARY(result);

		ie->stop();
		oe->stop();
	}
#endif

#if defined(HAVE_INPUT_TEST) && defined(HAVE_OUTPUT_DUMMY)
	SUBCASE("testInputDummyOutputGeneratesActiveEvents")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("test");
		auto oe = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		// Configure test engine: 100% probability, instrument 0, length 1 sec
		ie->setParm("p", "1.0");
		ie->setParm("instr", "0");
		ie->setParm("len", "1");

		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		REQUIRE(gizmo.init());
		REQUIRE(ie->start());
		REQUIRE(oe->start());

		std::size_t nsamples = oe->getBufferSize();
		std::vector<sample_t> buf(nsamples, 0.0f);

		// Test engine generates OnSet events, so run() should find active
		// events
		bool saw_active = false;
		for(int i = 0; i < 10; ++i)
		{
			bool result = gizmo.run(i * nsamples, buf.data(), nsamples);
			if(result)
			{
				saw_active = true;
				break;
			}
		}

		CHECK_UNARY(saw_active);

		ie->stop();
		oe->stop();
	}
#endif

#if defined(HAVE_INPUT_MIDIFILE) && defined(HAVE_OUTPUT_DUMMY)
	SUBCASE("midifileInputDummyOutputFullPipeline")
	{
		// Create a minimal MIDI file with two note-on events
		smf_t* smf = smf_new();
		REQUIRE(smf != nullptr);

		int ppqn_res = smf_set_ppqn(smf, 96);
		(void)ppqn_res;
		smf_track_t* track = smf_track_new();
		REQUIRE(track != nullptr);
		smf_add_track(smf, track);

		// Note On  ch=0 note=60 vel=100
		std::uint8_t note_on_60[] = {0x90, 60, 100};
		smf_event_t* ev1 =
		    smf_event_new_from_pointer(note_on_60, sizeof(note_on_60));
		REQUIRE(ev1 != nullptr);
		smf_track_add_event_seconds(track, ev1, 0.0);

		// Note Off ch=0 note=60 vel=0
		std::uint8_t note_off_60[] = {0x80, 60, 0};
		smf_event_t* ev2 =
		    smf_event_new_from_pointer(note_off_60, sizeof(note_off_60));
		REQUIRE(ev2 != nullptr);
		smf_track_add_event_seconds(track, ev2, 0.1);

		ScopedFile midi_file("placeholder");
		int save_res = smf_save(smf, midi_file.filename().c_str());
		(void)save_res;
		smf_delete(smf);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"instr1\"/>\n"
		                        "</midimap>");

		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("midifile");
		auto oe = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		ie->setParm("file", midi_file.filename());
		ie->setParm("midimap", midimap_file.filename());

		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		REQUIRE(gizmo.init());
		REQUIRE(ie->start());
		REQUIRE(oe->start());

		std::size_t nsamples = oe->getBufferSize();
		std::vector<sample_t> buf(nsamples, 0.0f);

		// The MIDI file contains note-on events, so run() should find active
		// events
		bool saw_active = false;
		for(int i = 0; i < 100; ++i)
		{
			bool result = gizmo.run(i * nsamples, buf.data(), nsamples);
			if(result)
			{
				saw_active = true;
				break;
			}
		}

		CHECK_UNARY(saw_active);

		ie->stop();
		oe->stop();
	}
#endif

#if defined(HAVE_INPUT_TEST) && defined(HAVE_OUTPUT_WAVFILE)
	SUBCASE("testInputWavfileOutputWritesFiles")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("test");
		auto oe = factory.createOutput("wavfile");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		std::string wav_prefix = "/tmp/dg_inttest_wavfile";
		oe->setParm("file", wav_prefix);

		ie->setParm("p", "1.0");
		ie->setParm("instr", "0");
		ie->setParm("len", "1");

		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		REQUIRE(gizmo.init());
		REQUIRE(ie->start());
		REQUIRE(oe->start());

		std::size_t nsamples = oe->getBufferSize();
		std::vector<sample_t> buf(nsamples, 0.0f);

		// Run a few frames to produce output
		for(int i = 0; i < 10; ++i)
		{
			if(!gizmo.run(i * nsamples, buf.data(), nsamples))
			{
				break;
			}
		}

		ie->stop();
		oe->stop();

		// The std kit has 4 channels; wavfile engine writes one file per
		// channel
		std::ifstream f0(wav_prefix + "ch0-0.wav");
		std::ifstream f1(wav_prefix + "ch1-1.wav");
		std::ifstream f2(wav_prefix + "ch2-2.wav");
		std::ifstream f3(wav_prefix + "ch3-3.wav");

		// At least one file should have been created and contain data
		bool any_file_nonempty =
		    (f0.good() && f0.peek() != std::ifstream::traits_type::eof()) ||
		    (f1.good() && f1.peek() != std::ifstream::traits_type::eof()) ||
		    (f2.good() && f2.peek() != std::ifstream::traits_type::eof()) ||
		    (f3.good() && f3.peek() != std::ifstream::traits_type::eof());

		CHECK_UNARY(any_file_nonempty);

		// Cleanup
		unlink((wav_prefix + "ch0-0.wav").c_str());
		unlink((wav_prefix + "ch1-1.wav").c_str());
		unlink((wav_prefix + "ch2-2.wav").c_str());
		unlink((wav_prefix + "ch3-3.wav").c_str());
	}
#endif

#if defined(HAVE_INPUT_MIDIFILE) && defined(HAVE_OUTPUT_WAVFILE)
	SUBCASE("midifileInputWavfileOutputWritesFiles")
	{
		smf_t* smf = smf_new();
		REQUIRE(smf != nullptr);

		int ppqn_res = smf_set_ppqn(smf, 96);
		(void)ppqn_res;
		smf_track_t* track = smf_track_new();
		REQUIRE(track != nullptr);
		smf_add_track(smf, track);

		std::uint8_t note_on[] = {0x90, 60, 100};
		smf_event_t* ev1 = smf_event_new_from_pointer(note_on, sizeof(note_on));
		REQUIRE(ev1 != nullptr);
		smf_track_add_event_seconds(track, ev1, 0.0);

		ScopedFile midi_file("placeholder");
		int save_res = smf_save(smf, midi_file.filename().c_str());
		(void)save_res;
		smf_delete(smf);

		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"instr1\"/>\n"
		                        "</midimap>");

		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("midifile");
		auto oe = factory.createOutput("wavfile");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		std::string wav_prefix = "/tmp/dg_inttest_midiwav";
		oe->setParm("file", wav_prefix);

		ie->setParm("file", midi_file.filename());
		ie->setParm("midimap", midimap_file.filename());

		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		REQUIRE(gizmo.init());
		REQUIRE(ie->start());
		REQUIRE(oe->start());

		std::size_t nsamples = oe->getBufferSize();
		std::vector<sample_t> buf(nsamples, 0.0f);

		for(int i = 0; i < 100; ++i)
		{
			if(!gizmo.run(i * nsamples, buf.data(), nsamples))
			{
				break;
			}
		}

		ie->stop();
		oe->stop();

		std::ifstream f0(wav_prefix + "ch0-0.wav");
		std::ifstream f1(wav_prefix + "ch1-1.wav");

		bool any_file_nonempty =
		    (f0.good() && f0.peek() != std::ifstream::traits_type::eof()) ||
		    (f1.good() && f1.peek() != std::ifstream::traits_type::eof());

		CHECK_UNARY(any_file_nonempty);

		unlink((wav_prefix + "ch0-0.wav").c_str());
		unlink((wav_prefix + "ch1-1.wav").c_str());
		unlink((wav_prefix + "ch2-2.wav").c_str());
		unlink((wav_prefix + "ch3-3.wav").c_str());
	}
#endif

	// ---- Hardware-dependent engine integration tests ----
	// Verify that wiring a hardware engine into the DrumGizmo pipeline does
	// not crash even when the hardware is unavailable (init() fails cleanly).

#ifdef HAVE_INPUT_ALSAMIDI
	SUBCASE("alsamidiInputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("alsamidi");
		auto oe_c = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe_c != nullptr);

		// Provide a valid midimap so the failure window is at the ALSA
		// hardware open, not at midimap parsing.
		auto midimapfile = drumkit_creator.createStdMidimap("alsa_int_mm");
		ie->setParm("midimap", midimapfile);

		DrumGizmo gizmo(settings, *oe_c, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe_c));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe_c->getSamplerate()));

		// init() should return false because ALSA hardware is unavailable
		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif

#ifdef HAVE_OUTPUT_ALSA
	SUBCASE("alsaOutputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("dummy");
		auto oe = factory.createOutput("alsa");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		// ALSA hardware not present → init should fail, not crash
		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif

#ifdef HAVE_INPUT_OSSMIDI
	SUBCASE("ossmidiInputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("ossmidi");
		auto oe_c = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe_c != nullptr);

		// Provide a valid midimap so the failure window is at the OSS
		// device open, not at midimap parsing.
		auto midimapfile = drumkit_creator.createStdMidimap("oss_int_mm");
		ie->setParm("midimap", midimapfile);

		DrumGizmo gizmo(settings, *oe_c, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe_c));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe_c->getSamplerate()));

		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif

#ifdef HAVE_OUTPUT_OSS
	SUBCASE("ossOutputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("dummy");
		auto oe = factory.createOutput("oss");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		// OSS hardware not present → init should fail, not crash
		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif

#ifdef HAVE_INPUT_JACKMIDI
	SUBCASE("jackmidiInputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("jackmidi");
		auto oe_c = factory.createOutput("dummy");

		REQUIRE(ie != nullptr);
		REQUIRE(oe_c != nullptr);

		// JACK server not present → init should fail, not crash
		DrumGizmo gizmo(settings, *oe_c, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe_c));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe_c->getSamplerate()));

		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif

#ifdef HAVE_OUTPUT_JACKAUDIO
	SUBCASE("jackaudioOutputInitFailsWithoutHardware")
	{
		EngineFactory factory;
		Settings settings;

		auto ie = factory.createInput("dummy");
		auto oe = factory.createOutput("jackaudio");

		REQUIRE(ie != nullptr);
		REQUIRE(oe != nullptr);

		// JACK server not present → init should fail, not crash
		DrumGizmo gizmo(settings, *oe, *ie);

		gizmo.setFrameSize(safeBufferSize(*oe));

		settings.drumkit_file.store(kitfile);
		settings.reload_counter++;

		REQUIRE(waitForDrumkitLoad(settings));

		gizmo.setSamplerate(static_cast<float>(oe->getSamplerate()));

		bool init_ok = gizmo.init();
		CHECK_UNARY(!init_ok);
	}
#endif
}
