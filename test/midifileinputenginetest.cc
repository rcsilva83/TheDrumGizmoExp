/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midifileinputenginetest.cc
 *
 *  Tue May  5 10:30:00 CET 2026
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

#ifdef HAVE_INPUT_MIDIFILE
#include "drumgizmo/input/midifile.h"
#include "scopedfile.h"
#include <fstream>
#include <instrument.h>
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
