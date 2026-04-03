/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            audiocacheeventhandlertest.cc
 *
 *  Thu Jan  7 15:44:14 CET 2016
 *  Copyright 2016 Bent Bisballe Nyeng
 *  deva@aasimon.org
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

#include <chrono>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include <audiocacheeventhandler.h>
#include <audiocacheidmanager.h>

#include "drumkit_creator.h"

class TestableAudioCacheEventHandler : public AudioCacheEventHandler
{
public:
	using AudioCacheEventHandler::AudioCacheEventHandler;

	size_t pendingEventCount()
	{
		std::lock_guard<std::mutex> lock(mutex);
		return eventqueue.size();
	}
};

TEST_CASE("AudioCacheEventHandlerTest")
{
	SUBCASE("default_state")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(10);

		AudioCacheEventHandler event_handler(id_manager);

		// Not threaded by default
		CHECK_UNARY(!event_handler.isThreaded());
		// Default chunk size is 1024
		CHECK_EQ(1024u, event_handler.getChunkSize());
	}

	SUBCASE("setChunkSize_changes_getChunkSize")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(10);

		AudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(2048);
		CHECK_EQ(2048u, event_handler.getChunkSize());
	}

	SUBCASE("setThreaded_changes_isThreaded")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(10);

		AudioCacheEventHandler event_handler(id_manager);
		CHECK_UNARY(!event_handler.isThreaded());

		event_handler.setThreaded(true);
		CHECK_UNARY(event_handler.isThreaded());

		event_handler.setThreaded(false);
		CHECK_UNARY(!event_handler.isThreaded());
	}

	SUBCASE("setChunkSize_same_value_is_noop")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		// Register an active ID to verify it is not disabled by the no-op call
		cacheid_t id = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id);

		// setChunkSize with the current default value - must be a no-op
		event_handler.setChunkSize(1024);
		CHECK_EQ(1024u, event_handler.getChunkSize());

		// Active ID should NOT be disabled (localpos stays 0, not max)
		cache_t& cache = id_manager.getCache(id);
		CHECK_NE(std::numeric_limits<size_t>::max(), cache.localpos);
	}

	SUBCASE("setChunkSize_different_value_disables_active_ids")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		// Register an active ID
		cacheid_t id = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id);

		// setChunkSize with a different value clears events and disables active
		// IDs
		event_handler.setChunkSize(512);
		CHECK_EQ(512u, event_handler.getChunkSize());

		// The active ID should now be disabled: localpos at max and ready false
		cache_t& cache = id_manager.getCache(id);
		CHECK_EQ(std::numeric_limits<size_t>::max(), cache.localpos);
		CHECK_EQ(false, (bool)cache.ready);
	}

	SUBCASE("close_event_nonthreaded_releases_id")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		// Fill the pool
		cacheid_t id0 = id_manager.registerID({});
		cacheid_t id1 = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id0);
		REQUIRE_NE(CACHE_DUMMYID, id1);

		// Pool is full: next registration returns the dummy id
		CHECK_EQ(CACHE_DUMMYID, id_manager.registerID({}));

		// Close id0 in non-threaded mode (event is handled synchronously)
		event_handler.pushCloseEvent(id0);

		// Pool now has room for one more id
		cacheid_t id2 = id_manager.registerID({});
		CHECK_NE(CACHE_DUMMYID, id2);
	}

	SUBCASE("stop_when_not_running_is_noop")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		// Thread was never started; stop() must return without blocking
		event_handler.stop();
		CHECK_UNARY(!event_handler.isThreaded());
	}

	SUBCASE("start_when_already_running_is_noop")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		event_handler.start();
		event_handler.start(); // Second call must be a no-op
		event_handler.stop();
	}

	SUBCASE("lock_and_unlock")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		AudioCacheEventHandler event_handler(id_manager);

		// lock/unlock are used via std::lock_guard; verify they are callable
		event_handler.lock();
		event_handler.unlock();
	}

	SUBCASE("setChunkSize_with_queued_close_event_clears_queue")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(2);

		TestableAudioCacheEventHandler event_handler(id_manager);

		cacheid_t id = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id);

		// Enable threaded mode WITHOUT starting the thread so the Close event
		// stays queued.
		event_handler.setThreaded(true);
		event_handler.pushCloseEvent(id);
		CHECK_EQ(1u, event_handler.pendingEventCount());

		// setChunkSize calls clearEvents() which processes the Close event.
		event_handler.setChunkSize(512);
		CHECK_EQ(0u, event_handler.pendingEventCount());

		// ID was released during clearEvents - pool has room again.
		cacheid_t id2 = id_manager.registerID({});
		CHECK_NE(CACHE_DUMMYID, id2);
	}
}

struct AudioCacheEventHandlerFileFixture
{
	DrumkitCreator drumkit_creator;
	static constexpr size_t test_chunk_size{512};
	static constexpr int load_timeout_ms{1000};
};

TEST_CASE_FIXTURE(
    AudioCacheEventHandlerFileFixture, "AudioCacheEventHandlerFileTest")
{
	SUBCASE("nonthreaded_load_event_executes_immediately")
	{
		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioCacheIDManager id_manager;
		id_manager.init(10);

		AudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(test_chunk_size);

		// Non-threaded is the default
		CHECK_UNARY(!event_handler.isThreaded());

		AudioCacheFile& afile = event_handler.openFile(filename);

		std::vector<sample_t> buf(test_chunk_size, 0.0f);
		volatile bool ready{false};

		// In non-threaded mode pushLoadNextEvent is handled synchronously
		event_handler.pushLoadNextEvent(&afile, 0, 0, buf.data(), &ready);

		// No waiting needed - the flag must be set before the call returns
		CHECK_UNARY(ready);
	}

	SUBCASE("deduplicate_load_events_threaded_both_channels_ready")
	{
		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioCacheIDManager id_manager;
		id_manager.init(10);

		TestableAudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(test_chunk_size);
		event_handler.setThreaded(true);

		AudioCacheFile& afile = event_handler.openFile(filename);

		std::vector<sample_t> buf0(test_chunk_size, 0.0f);
		std::vector<sample_t> buf1(test_chunk_size, 0.0f);
		volatile bool ready0{false};
		volatile bool ready1{false};

		// Push two load events for the same file and position but different
		// channels.
		event_handler.pushLoadNextEvent(&afile, 0, 0, buf0.data(), &ready0);
		event_handler.pushLoadNextEvent(&afile, 1, 0, buf1.data(), &ready1);

		// Verify deduplication happened in the queue before processing starts.
		CHECK_EQ(1u, event_handler.pendingEventCount());

		event_handler.start();

		// Wait for both channels to be loaded
		int timeout = load_timeout_ms;
		while((!ready0 || !ready1) && timeout > 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			--timeout;
		}

		CHECK_UNARY(ready0);
		CHECK_UNARY(ready1);

		event_handler.stop();
	}

	SUBCASE("destructor_releases_file_for_active_id_with_afile")
	{
		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		{
			AudioCacheIDManager id_manager;
			id_manager.init(2);

			AudioCacheEventHandler event_handler(id_manager);

			// Open the file - increments the internal ref count.
			AudioCacheFile& afile = event_handler.openFile(filename);

			// Register an active ID that refers to the open file.
			cache_t c;
			c.afile = &afile;
			cacheid_t id = id_manager.registerID(c);
			REQUIRE_NE(CACHE_DUMMYID, id);

			// When event_handler destructs it calls handleCloseCache(id)
			// which releases the file ref via files.releaseFile() (line 221).
		}
	}

	SUBCASE("deduplicate_different_pos_creates_separate_events")
	{
		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioCacheIDManager id_manager;
		id_manager.init(10);

		TestableAudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(test_chunk_size);
		// Threaded but NOT started → events stay queued.
		event_handler.setThreaded(true);

		AudioCacheFile& afile = event_handler.openFile(filename);

		std::vector<sample_t> buf0(test_chunk_size, 0.0f);
		std::vector<sample_t> buf1(test_chunk_size, 0.0f);
		volatile bool ready0{false};
		volatile bool ready1{false};

		// Push two LoadNext events for the same file but different positions.
		// The dedup loop finds a position mismatch → no merge → two events.
		event_handler.pushLoadNextEvent(&afile, 0, 0, buf0.data(), &ready0);
		event_handler.pushLoadNextEvent(
		    &afile, 0, test_chunk_size, buf1.data(), &ready1);

		CHECK_EQ(2u, event_handler.pendingEventCount());
	}

	SUBCASE("deduplicate_skips_non_loadnext_queued_event")
	{
		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		AudioCacheIDManager id_manager;
		id_manager.init(10);

		TestableAudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(test_chunk_size);
		// Threaded but NOT started → events stay queued.
		event_handler.setThreaded(true);

		AudioCacheFile& afile = event_handler.openFile(filename);

		cacheid_t id = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id);

		// Put a Close event in the queue first.
		event_handler.pushCloseEvent(id);
		CHECK_EQ(1u, event_handler.pendingEventCount());

		// Push a LoadNext event. The dedup loop iterates the queue, encounters
		// the Close event (type != LoadNext) → skips it → no match found →
		// appends new LoadNext event.
		std::vector<sample_t> buf(test_chunk_size, 0.0f);
		volatile bool ready{false};
		event_handler.pushLoadNextEvent(&afile, 0, 0, buf.data(), &ready);

		// Queue contains the original Close event plus the new LoadNext event.
		CHECK_EQ(2u, event_handler.pendingEventCount());
	}

	SUBCASE("deduplicate_different_file_creates_separate_event")
	{
		auto filename_a = drumkit_creator.createSingleChannelWav("file_a.wav");
		auto filename_b = drumkit_creator.createSingleChannelWav("file_b.wav");

		AudioCacheIDManager id_manager;
		id_manager.init(10);

		TestableAudioCacheEventHandler event_handler(id_manager);
		event_handler.setChunkSize(test_chunk_size);
		// Threaded but NOT started → events stay queued.
		event_handler.setThreaded(true);

		AudioCacheFile& afile_a = event_handler.openFile(filename_a);
		AudioCacheFile& afile_b = event_handler.openFile(filename_b);

		std::vector<sample_t> buf0(test_chunk_size, 0.0f);
		std::vector<sample_t> buf1(test_chunk_size, 0.0f);
		volatile bool ready0{false};
		volatile bool ready1{false};

		// Push LoadNext events for two DIFFERENT files at the same position.
		// The dedup loop finds the filename mismatch → no merge → two events.
		event_handler.pushLoadNextEvent(&afile_a, 0, 0, buf0.data(), &ready0);
		event_handler.pushLoadNextEvent(&afile_b, 0, 0, buf1.data(), &ready1);

		CHECK_EQ(2u, event_handler.pendingEventCount());
	}

	SUBCASE("threaded_close_event_releases_id_after_processing")
	{
		AudioCacheIDManager id_manager;
		id_manager.init(1);

		AudioCacheEventHandler event_handler(id_manager);
		event_handler.setThreaded(true);
		event_handler.start();

		cacheid_t id = id_manager.registerID({});
		REQUIRE_NE(CACHE_DUMMYID, id);

		event_handler.pushCloseEvent(id);

		cacheid_t replacement_id = CACHE_DUMMYID;
		int timeout = load_timeout_ms;
		while(timeout > 0)
		{
			replacement_id = id_manager.registerID({});
			if(replacement_id != CACHE_DUMMYID)
			{
				break;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			--timeout;
		}

		CHECK_NE(CACHE_DUMMYID, replacement_id);

		event_handler.stop();
	}

	SUBCASE("threaded_and_nonthreaded_load_parity")
	{
		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel_parity.wav");

		constexpr size_t chunk_size = 64;

		std::vector<sample_t> nonthreaded_buf(chunk_size, 0.0f);
		std::vector<sample_t> threaded_buf(chunk_size, 0.0f);
		volatile bool nonthreaded_ready{false};
		volatile bool threaded_ready{false};

		{
			AudioCacheIDManager id_manager;
			id_manager.init(2);

			AudioCacheEventHandler event_handler(id_manager);
			event_handler.setChunkSize(chunk_size);

			AudioCacheFile& afile = event_handler.openFile(filename);
			event_handler.pushLoadNextEvent(
			    &afile, 0, 0, nonthreaded_buf.data(), &nonthreaded_ready);
		}

		{
			AudioCacheIDManager id_manager;
			id_manager.init(2);

			AudioCacheEventHandler event_handler(id_manager);
			event_handler.setChunkSize(chunk_size);
			event_handler.setThreaded(true);
			event_handler.start();

			AudioCacheFile& afile = event_handler.openFile(filename);
			event_handler.pushLoadNextEvent(
			    &afile, 0, 0, threaded_buf.data(), &threaded_ready);

			int timeout = load_timeout_ms;
			while(!threaded_ready && timeout > 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				--timeout;
			}

			event_handler.stop();
		}

		CHECK_UNARY(nonthreaded_ready);
		CHECK_UNARY(threaded_ready);

		for(size_t i = 0; i < chunk_size; ++i)
		{
			CHECK_EQ(nonthreaded_buf[i], threaded_buf[i]);
		}
	}
}
