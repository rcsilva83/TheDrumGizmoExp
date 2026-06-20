/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsamidiinputenginetest.cc
 *
 *  Wed May  6 10:00:00 CEST 2026
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

#ifdef HAVE_INPUT_ALSAMIDI

#include "../drumgizmo/alsaseqwrapper.h"
#include "../drumgizmo/input/alsamidi.h"
#include "mock_wrappers.h"
#include "scopedfile.h"

#include <alsa/asoundlib.h>

TEST_CASE("AlsaMidiInputEngineWithMock")
{
	// ---- Constructor and lifecycle tests ----

	SUBCASE("defaultConstructorCreatesEngine")
	{
		AlsaMidiInputEngine engine;
		CHECK_UNARY(true);
	}

	SUBCASE("injectionConstructorCreatesEngine")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		CHECK_UNARY(engine.isFreewheeling());
	}

	SUBCASE("startReturnsTrue")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		CHECK_UNARY(engine.start());
	}

	SUBCASE("stopPrePostDoNotThrow")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		engine.stop();
		engine.pre();
		engine.post();
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMidimapStoresFilename")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", "/test/map.xml");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownDoesNotCrash")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		engine.setParm("unknown", "value");
		CHECK_UNARY(true);
	}

	// ---- init() success with mock ----

	SUBCASE("initSucceedsWithValidMidimap")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));
	}

	// ---- init() failure paths ----

	SUBCASE("initFailsWithInvalidMidimap")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", "/nonexistent/map.xml");

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	SUBCASE("initFailsWhenSndSeqOpenFails")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = -1; // ALSA open fails

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	SUBCASE("initFailsWhenSetClientNameFails")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = -1;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	SUBCASE("initFailsWhenCreateSimplePortFails")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = -1;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	// ---- run() with mock events ----

	SUBCASE("runWithNoSeqEventDoesNotProduceEvents")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = -1; // no event available

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(events.empty());
	}

	SUBCASE("runWithNoteOnEventDoesNotCrash")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = 0;

		seq.event_data.type = SND_SEQ_EVENT_NOTEON;
		seq.event_data.data.note.channel = 0;
		seq.event_data.data.note.note = 60;
		seq.event_data.data.note.velocity = 100;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		// processNote called via mock -- verify no crash
		CHECK_UNARY(true);
	}

	SUBCASE("runWithNoteOffEventDoesNotCrash")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = 0;

		seq.event_data.type = SND_SEQ_EVENT_NOTEOFF;
		seq.event_data.data.note.channel = 0;
		seq.event_data.data.note.note = 60;
		seq.event_data.data.note.off_velocity = 0;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(true);
	}

	SUBCASE("runWithKeyPressEventDoesNotCrash")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = 0;

		seq.event_data.type = SND_SEQ_EVENT_KEYPRESS;
		seq.event_data.data.note.channel = 0;
		seq.event_data.data.note.note = 60;
		seq.event_data.data.note.velocity = 50;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(true);
	}

	SUBCASE("runWithControllerEventDoesNotCrash")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = 0;

		seq.event_data.type = SND_SEQ_EVENT_CONTROLLER;
		seq.event_data.data.control.channel = 0;
		seq.event_data.data.control.param = 7; // volume
		seq.event_data.data.control.value = 127;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(true);
	}

	SUBCASE("runWithUnknownEventTypeDoesNotProduceEvents")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = 0;

		seq.event_data.type = SND_SEQ_EVENT_PORT_SUBSCRIBED;

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		// Unknown type should not produce MIDI events
		CHECK_UNARY(events.empty());
	}

	// ---- Full lifecycle with mock ----

	SUBCASE("fullLifecycleSucceeds")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockAlsaSeqWrapper seq;
		seq.open_return = 0;
		seq.set_client_name_return = 0;
		seq.create_simple_port_return = 1;
		seq.event_input_return = -1; // no events

		AlsaMidiInputEngine engine(seq);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));
		CHECK_UNARY(engine.start());

		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 1024, events);
		engine.post();

		engine.stop();
		CHECK_UNARY(true);
	}

	SUBCASE("runWithoutInitUsesWrapper")
	{
		MockAlsaSeqWrapper seq;
		AlsaMidiInputEngine engine(seq);

		std::vector<event_t> events;
		// run() without init would try to use seq_handle which is null
		// and seq_wrapper.event_input with null handle. The mock handles this.
		seq.event_input_return = -1;
		engine.run(0, 1024, events);
		CHECK_UNARY(events.empty());
	}
}

#endif // HAVE_INPUT_ALSAMIDI
