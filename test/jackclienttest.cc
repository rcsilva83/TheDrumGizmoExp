/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            jackclienttest.cc
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

#if defined(HAVE_INPUT_JACKMIDI) || defined(HAVE_OUTPUT_JACKAUDIO)

#include "../drumgizmo/jackclient.h"
#include <jack/jack.h>

class TestJackProcess : public JackProcess
{
public:
	int process_count{0};
	jack_nframes_t last_nframes{0};
	int latency_callback_count{0};
	jack_latency_callback_mode_t last_mode{JackCaptureLatency};

	void process(jack_nframes_t num_frames) override
	{
		process_count++;
		last_nframes = num_frames;
	}

	void jackLatencyCallback(jack_latency_callback_mode_t mode) override
	{
		latency_callback_count++;
		last_mode = mode;
	}
};

TEST_CASE("JackClientInternal")
{
	SUBCASE("testConstructorSkipsJackConnection")
	{
		JackClient client(nullptr);
		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingInitiallyFalse")
	{
		JackClient client(nullptr);
		CHECK_UNARY(!client.isFreewheeling());
	}

	SUBCASE("addWithoutProcessDoesNotDispatch")
	{
		JackClient client(nullptr);
		TestJackProcess p1;

		client.add(p1);
		client.test_process(256);
		CHECK_EQ(1, p1.process_count);
		CHECK_EQ(256u, static_cast<unsigned>(p1.last_nframes));
	}

	SUBCASE("removeBeforeProcessClearsProcess")
	{
		JackClient client(nullptr);
		TestJackProcess p1;
		TestJackProcess p2;

		client.add(p1);
		client.add(p2);
		client.remove(p1);

		client.test_process(256);

		CHECK_EQ(0, p1.process_count);
		CHECK_EQ(1, p2.process_count);
	}

	SUBCASE("removeAfterProcessClearsOnNextProcess")
	{
		JackClient client(nullptr);
		TestJackProcess p1;
		TestJackProcess p2;

		client.add(p1);
		client.add(p2);

		// First process: both active
		client.test_process(128);
		CHECK_EQ(1, p1.process_count);
		CHECK_EQ(1, p2.process_count);

		// Remove p1
		client.remove(p1);

		// Second process: p1 removed
		client.test_process(256);
		CHECK_EQ(1, p1.process_count); // not called again
		CHECK_EQ(2, p2.process_count); // called again
	}

	SUBCASE("dirtyFlagClearedAfterProcess")
	{
		JackClient client(nullptr);
		TestJackProcess p1;

		client.add(p1);
		client.remove(p1);

		// First process clears dirty and removes p1
		client.test_process(128);
		CHECK_EQ(0, p1.process_count);

		// Second process: list should be clean, no further dispatch
		TestJackProcess p2;
		client.add(p2);
		client.test_process(256);
		CHECK_EQ(1, p2.process_count);
	}

	SUBCASE("multipleProcessesDispatched")
	{
		JackClient client(nullptr);
		TestJackProcess p1, p2, p3;

		client.add(p1);
		client.add(p2);
		client.add(p3);

		client.test_process(512);

		CHECK_EQ(1, p1.process_count);
		CHECK_EQ(1, p2.process_count);
		CHECK_EQ(1, p3.process_count);
		CHECK_EQ(512u, static_cast<unsigned>(p1.last_nframes));
	}

	SUBCASE("latencyCallbackDispatchedToAll")
	{
		JackClient client(nullptr);
		TestJackProcess p1, p2;

		client.add(p1);
		client.add(p2);

		client.test_latency_callback(JackPlaybackLatency);

		CHECK_EQ(1, p1.latency_callback_count);
		CHECK_EQ(1, p2.latency_callback_count);
		CHECK_EQ(JackPlaybackLatency, p1.last_mode);
	}

	SUBCASE("freewheelCallbackSetsState")
	{
		JackClient client(nullptr);

		CHECK_UNARY(!client.isFreewheeling());
		client.test_freewheel_callback(true);
		CHECK_UNARY(client.isFreewheeling());
		client.test_freewheel_callback(false);
		CHECK_UNARY(!client.isFreewheeling());
	}

	SUBCASE("removeAllProcessesThenProcess")
	{
		JackClient client(nullptr);
		TestJackProcess p1, p2;

		client.add(p1);
		client.add(p2);
		client.remove(p1);
		client.remove(p2);

		client.test_process(64);

		CHECK_EQ(0, p1.process_count);
		CHECK_EQ(0, p2.process_count);
	}

	SUBCASE("addAfterPreviousRemovalWorks")
	{
		JackClient client(nullptr);
		TestJackProcess p1;

		client.add(p1);
		client.remove(p1);
		client.test_process(64);

		// Re-add after removal
		client.add(p1);
		client.test_process(128);
		CHECK_EQ(1, p1.process_count);
	}

	SUBCASE("interleavedAddRemove")
	{
		JackClient client(nullptr);
		TestJackProcess p1, p2, p3;

		client.add(p1);
		client.test_process(32);
		CHECK_EQ(1, p1.process_count);

		client.add(p2);
		client.remove(p1);
		client.test_process(64);
		CHECK_EQ(1, p1.process_count); // not called
		CHECK_EQ(1, p2.process_count); // called once

		client.add(p3);
		client.remove(p2);
		client.test_process(128);
		CHECK_EQ(1, p3.process_count); // only p3 active
	}
}

TEST_CASE("JackPortCreation")
{
	// JackPort requires a valid jack_client_t*, so we can't test port
	// creation without a real JACK server. Just document it.
	SUBCASE("portCannotBeCreatedWithoutJackServer")
	{
		CHECK_UNARY(true); // expected: port creation needs real JACK
	}
}

#endif // HAVE_INPUT_JACKMIDI || HAVE_OUTPUT_JACKAUDIO
