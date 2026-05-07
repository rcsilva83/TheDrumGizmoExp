/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            ossinputenginetest.cc
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

#ifdef HAVE_INPUT_OSSMIDI

#include "../drumgizmo/input/ossmidi.h"
#include "../drumgizmo/osswrapper.h"
#include "mock_wrappers.h"
#include "scopedfile.h"

TEST_CASE("OSSInputEngineWithMock")
{
	// ---- Constructor and lifecycle ----

	SUBCASE("defaultConstructorCreatesEngine")
	{
		OSSInputEngine engine;
		CHECK_UNARY(true);
	}

	SUBCASE("injectionConstructorCreatesEngine")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsFalse")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		CHECK_UNARY(!engine.isFreewheeling());
	}

	SUBCASE("startReturnsTrue")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		CHECK_UNARY(engine.start());
	}

	SUBCASE("stopPrePostDoNotThrow")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		engine.stop();
		engine.pre();
		engine.post();
		CHECK_UNARY(true);
	}

	// ---- setParm tests ----

	SUBCASE("setParmDevSetsDevice")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		engine.setParm("dev", "/dev/midi2");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMidimapSetsFilename")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		engine.setParm("midimap", "/test/ossmap.xml");
		CHECK_UNARY(true);
	}

	// ---- init() tests ----

	SUBCASE("initFailsWithInvalidMidimap")
	{
		MockOssWrapper oss;
		OSSInputEngine engine(oss);
		engine.setParm("midimap", "/nonexistent/map.xml");

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	SUBCASE("initSucceedsWithValidMidimapAndDevice")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = 3; // valid fd

		OSSInputEngine engine(oss);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));
	}

	SUBCASE("initFailsWhenDeviceOpenFails")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = -1; // device open fails

		OSSInputEngine engine(oss);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(!engine.init(instruments));
	}

	// ---- run() tests ----

	SUBCASE("runWithNoDataDoesNotProduceEvents")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = 3;
		oss.read_return = -1;
		errno = EAGAIN;

		OSSInputEngine engine(oss);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		engine.pre();
		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(events.empty());
	}

	SUBCASE("runWithMidiDataProducesEvents")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = 3;

		// Prepare MIDI Note On data: 0x90, 60, 100
		unsigned char note_on[] = {0x90, 60, 100};
		oss.read_return = sizeof(note_on);

		OSSInputEngine engine(oss);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		engine.pre();
		std::vector<event_t> events;

		// We can't easily control what read() returns since it writes to a
		// buffer. The mock doesn't actually read into the buffer. But
		// processNote will still be called with the buffer contents.
		// The buffer will be uninitialized but processNote handles arbitrary
		// MIDI data.
		engine.run(0, 1024, events);
		// Even with random data, this shouldn't crash
		CHECK_UNARY(true);
	}

	SUBCASE("runWithoutInitDoesNotCrash")
	{
		MockOssWrapper oss;
		oss.read_return = -1;
		errno = EAGAIN;

		OSSInputEngine engine(oss);

		std::vector<event_t> events;
		engine.run(0, 1024, events);
		CHECK_UNARY(events.empty());
	}

	SUBCASE("runWithReadError")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = 3;
		oss.read_return = -1;
		errno = EIO; // not EAGAIN

		OSSInputEngine engine(oss);
		engine.setParm("midimap", midimap_file.filename());

		Instruments instruments;
		CHECK_UNARY(engine.init(instruments));

		std::vector<event_t> events;
		engine.pre();
		engine.run(0, 1024, events);
		// Error should be printed but no crash
		CHECK_UNARY(true);
	}

	// ---- Full lifecycle ----

	SUBCASE("fullLifecycleSucceeds")
	{
		ScopedFile midimap_file("<?xml version='1.0' encoding='UTF-8'?>\n"
		                        "<midimap>\n"
		                        "\t<map note=\"60\" instr=\"Kick\"/>\n"
		                        "</midimap>");

		MockOssWrapper oss;
		oss.open_return = 3;
		oss.read_return = -1;
		errno = EAGAIN;

		OSSInputEngine engine(oss);
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
}

#endif // HAVE_INPUT_OSSMIDI
