/* -*- Mode: c++ -*- */
/***************************************************************************
 *            audioinputenginemiditest.cc
 *
 *  Wed Mar 25 2026
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

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

#include <event.h>
#include <midimapper.h>

#include "audioinputenginemidi.h"

// Concrete subclass used only for testing processNote().
// All pure virtuals are stubbed out since only processNote() is exercised.
class TestMidiEngine : public AudioInputEngineMidi
{
public:
	bool init(const Instruments&) override
	{
		return true;
	}
	void setParm(const std::string&, const std::string&) override
	{
	}
	bool start() override
	{
		return true;
	}
	void stop() override
	{
	}
	void pre() override
	{
	}
	void run(size_t, size_t, std::vector<event_t>&) override
	{
	}
	void post() override
	{
	}
	bool isFreewheeling() const override
	{
		return false;
	}

	// Expose the protected mmap for test setup.
	void setupMapping(
	    int note, const std::string& instrument_name, std::size_t instrument_id)
	{
		midimap_t mm{{note, instrument_name}};
		instrmap_t im{{instrument_name, instrument_id}};
		mmap.swap(im, mm);
	}

	// Set up multiple instruments mapped to the same note (fanout scenario).
	void setupMultiMapping(int note,
	    const std::vector<std::pair<std::string, std::size_t>>& mappings)
	{
		midimap_t mm;
		instrmap_t im;
		for(const auto& m : mappings)
		{
			mm.push_back({note, m.first});
			im[m.first] = static_cast<int>(m.second);
		}
		mmap.swap(im, mm);
	}
};

static const std::uint8_t NOTE_OFF = 0x80;
static const std::uint8_t NOTE_ON = 0x90;
static const std::uint8_t NOTE_AFTERTOUCH = 0xA0;

TEST_CASE("AudioInputEngineMidiTest")
{
	SUBCASE("regression_0_9_19_note_on_velocity_zero_generates_no_event")
	{
		// Regression for v0.9.19: "Notes with velocity 0 is now ignored."
		// Before the fix a NoteOn with velocity=0 (which is the MIDI standard
		// equivalent of a NoteOff) was incorrectly treated as a drum hit and
		// produced an OnSet event.  The fix added the check `if(velocity != 0)`
		// in AudioInputEngineMidi::processNote().
		TestMidiEngine engine;
		engine.setupMapping(60, "Kick", 0);

		std::uint8_t note_on_velocity_zero[] = {NOTE_ON, 60, 0};
		std::vector<event_t> events;

		engine.processNote(note_on_velocity_zero, 3, 0, events);

		CHECK_EQ(0u, events.size());
	}

	SUBCASE("note_on_nonzero_velocity_generates_onset_event")
	{
		// Confirm that a regular NoteOn with velocity > 0 still generates
		// an OnSet event (sanity-check for the regression above).
		TestMidiEngine engine;
		engine.setupMapping(60, "Kick", 0);

		std::uint8_t note_on[] = {NOTE_ON, 60, 64};
		std::vector<event_t> events;

		engine.processNote(note_on, 3, 0, events);

		CHECK_EQ(1u, events.size());
		CHECK_EQ(EventType::OnSet, events[0].type);
		CHECK_EQ(0u, events[0].instrument);
	}

	SUBCASE("regression_0_9_20_aftertouch_velocity_gt_zero_generates_choke")
	{
		// Regression for v0.9.20: "Make aftertouch choke if velocities > 0
		// instead of == 0 as this seem to be the vdrum vendor consensus."
		// Before the fix the condition was `velocity == 0`, so no choke was
		// produced when the player applied pressure (velocity > 0).  The fix
		// changed the check to `velocity > 0`.
		TestMidiEngine engine;
		engine.setupMapping(49, "Crash", 1);

		std::uint8_t aftertouch_positive[] = {NOTE_AFTERTOUCH, 49, 100};
		std::vector<event_t> events;

		engine.processNote(aftertouch_positive, 3, 0, events);

		CHECK_EQ(1u, events.size());
		CHECK_EQ(EventType::Choke, events[0].type);
		CHECK_EQ(1u, events[0].instrument);
	}

	SUBCASE("regression_0_9_20_aftertouch_velocity_zero_generates_no_choke")
	{
		// Counterpart to the regression above: aftertouch with velocity=0
		// must NOT generate a Choke event.  With the old (buggy) `== 0` check
		// only velocity=0 would choke; with the fixed `> 0` check it must
		// not choke on velocity=0.
		TestMidiEngine engine;
		engine.setupMapping(49, "Crash", 1);

		std::uint8_t aftertouch_zero[] = {NOTE_AFTERTOUCH, 49, 0};
		std::vector<event_t> events;

		engine.processNote(aftertouch_zero, 3, 0, events);

		CHECK_EQ(0u, events.size());
	}

	SUBCASE("short_buffer_generates_no_event")
	{
		// processNote() must handle truncated MIDI data gracefully without
		// producing any events.
		TestMidiEngine engine;
		engine.setupMapping(60, "Kick", 0);

		std::uint8_t short_buf[] = {NOTE_ON, 60};
		std::vector<event_t> events;

		engine.processNote(short_buf, 2, 0, events);

		CHECK_EQ(0u, events.size());
	}

	SUBCASE("note_off_generates_no_event")
	{
		// NoteOff messages are currently ignored by the engine.
		TestMidiEngine engine;
		engine.setupMapping(60, "Kick", 0);

		std::uint8_t note_off[] = {NOTE_OFF, 60, 64};
		std::vector<event_t> events;

		engine.processNote(note_off, 3, 0, events);

		CHECK_EQ(0u, events.size());
	}

	SUBCASE("one_note_mapped_to_multiple_instruments_generates_multiple_events")
	{
		// Fanout: a single MIDI note mapped to two different instruments must
		// produce one OnSet event per instrument.
		TestMidiEngine engine;
		engine.setupMultiMapping(60, {{"Kick", 0}, {"Snare", 1}});

		std::uint8_t note_on[] = {NOTE_ON, 60, 64};
		std::vector<event_t> events;

		engine.processNote(note_on, 3, 0, events);

		CHECK_EQ(2u, events.size());
		CHECK_EQ(EventType::OnSet, events[0].type);
		CHECK_EQ(EventType::OnSet, events[1].type);
		// Both mapped instruments must appear, regardless of iteration order.
		std::vector<std::size_t> instruments{
		    events[0].instrument, events[1].instrument};
		std::sort(instruments.begin(), instruments.end());
		CHECK_EQ(0u, instruments[0]);
		CHECK_EQ(1u, instruments[1]);
	}
}
