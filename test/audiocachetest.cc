/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            cachemanagertest.cc
 *
 *  Sun Apr 19 10:15:59 CEST 2015
 *  Copyright 2015 Bent Bisballe Nyeng
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
#include <thread>

#include <audiocache.h>
#include <audiofile.h>
#include <settings.h>

#include "drumkit_creator.h"

#define FRAMESIZE 64

static constexpr std::size_t preload_buffer_size{4096};

struct AudioCacheTestFixture
{
	DrumkitCreator drumkit_creator;

	//! Test runner.
	//! \param filename The name of the file to read.
	//! \param channel The channel number to do comparison on.
	//! \param thread Control if this test is running in threaded mode or not.
	//! \param framesize The initial framesize to use.
	void testHelper(const char* filename, int channel, bool threaded,
	    int framesize, int num_channels)
	{
		// Reference file:
		AudioFile audio_file_ref(filename, channel);
		printf("audio_file_ref.load\n");
		audio_file_ref.load(nullptr);

		// Input file:
		AudioFile audio_file(filename, channel);
		printf("audio_file.load\n");
		audio_file.load(nullptr, 4096);

		Settings settings;
		AudioCache audio_cache(settings);
		printf("audio_cache.init\n");
		audio_cache.init(100);
		audio_cache.setAsyncMode(threaded);

		// Set initial (upper limit) framesize
		audio_cache.setFrameSize(framesize);
		audio_cache.updateChunkSize(num_channels);

		cacheid_t id;

		for(size_t initial_samples_needed = 0;
		    initial_samples_needed < (size_t)(framesize + 1);
		    ++initial_samples_needed)
		{

			printf("open: initial_samples_needed: %d\n",
			    (int)initial_samples_needed);
			sample_t* samples = audio_cache.open(
			    audio_file, initial_samples_needed, channel, id);
			size_t size = initial_samples_needed;
			size_t offset = 0;

			// Test pre cache:
			for(size_t i = 0; i < size; ++i)
			{
				CHECK_EQ(audio_file_ref.data[offset], samples[i]);
				++offset;
			}

			// Test the rest
			while(offset < audio_file_ref.size)
			{
				if(threaded)
				{
					// Wait until we are finished reading
					int timeout = 1000;
					while(!audio_cache.isReady(id))
					{
						std::this_thread::sleep_for(
						    std::chrono::milliseconds(1));
						if(--timeout == 0)
						{
							FAIL(""); // timeout
						}
					}
				}

				size = framesize;
				samples = audio_cache.next(id, size);

				CHECK_EQ(std::size_t(0), settings.number_of_underruns.load());

				for(size_t i = 0; (i < size) && (offset < audio_file_ref.size);
				    ++i)
				{
					if(audio_file_ref.data[offset] != samples[i])
					{
						printf("-----> offset: %d, size: %d, diff: %d,"
						       " i: %d, size: %d, block-diff: %d\n",
						    (int)offset, (int)audio_file_ref.size,
						    (int)(audio_file_ref.size - offset), (int)i,
						    (int)size, (int)(size - i));
					}
					CHECK_EQ(audio_file_ref.data[offset], samples[i]);
					++offset;
				}
			}

			audio_cache.close(id);
		}

		printf("done\n");
	}
};

TEST_CASE_FIXTURE(AudioCacheTestFixture, "AudioCacheTest")
{
	SUBCASE("singleChannelNonThreaded")
	{
		printf("\nsinglechannel_nonthreaded()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		// Conduct test
		int channel = 0;
		bool threaded = false;
		int num_channels = 1;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
	}

	SUBCASE("singleChannelThreaded")
	{
		printf("\nsinglechannel_threaded()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		// Conduct test
		int channel = 0;
		bool threaded = true;
		int num_channels = 1;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
	}

	SUBCASE("multiChannelNonThreaded")
	{
		printf("\nmultichannel_nonthreaded()\n");

		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		// Conduct test
		int channel = 0;
		bool threaded = false;
		int num_channels = 13;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
		++channel;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
	}

	SUBCASE("multiChannelThreaded")
	{
		printf("\nmultichannel_threaded()\n");

		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		// Conduct test
		int channel = 0;
		bool threaded = true;
		int num_channels = 13;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
		++channel;
		testHelper(
		    filename.c_str(), channel, threaded, FRAMESIZE, num_channels);
	}

	SUBCASE("nonthreadedThreadedParity")
	{
		printf("\nnonthreaded_threaded_parity()\n");

		auto filename = drumkit_creator.createSingleChannelWav("parity.wav");

		int channel = 0;
		int num_channels = 1;

		// Both modes must produce identical correct output against the
		// reference
		testHelper(filename.c_str(), channel, false, FRAMESIZE, num_channels);
		testHelper(filename.c_str(), channel, true, FRAMESIZE, num_channels);
	}

	SUBCASE("updateChunkSizeWhileEventsQueued")
	{
		printf("\nupdate_chunk_size_while_events_queued()\n");

		// Create a file larger than the preload buffer to force disk streaming
		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr, preload_buffer_size);
		REQUIRE_LT(audio_file.preloadedsize, audio_file.size);

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(100);
		audio_cache.setAsyncMode(true);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// Keep async mode enabled but stop the worker thread so events remain
		// queued until the test explicitly changes state.
		audio_cache.deinit();

		cacheid_t id;
		// Open queues a LoadNext event when file.size > preloadedsize
		audio_cache.open(audio_file, 0, 0, id);
		REQUIRE_NE(CACHE_DUMMYID, id);
		REQUIRE_UNARY(!audio_cache.isReady(id));

		// Change chunk size while the LoadNext event is queued.
		// setChunkSize clears queued load events and disables active IDs.
		audio_cache.updateChunkSize(13);
		CHECK_UNARY(!audio_cache.isReady(id));

		std::size_t size = FRAMESIZE;
		audio_cache.next(id, size);
		CHECK_EQ(std::size_t(1), settings.number_of_underruns.load());

		// Close the entry; the destructor will process the Close event.
		audio_cache.close(id);
	}

	SUBCASE("closeWhileLoadQueued")
	{
		printf("\nclose_while_load_queued()\n");

		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr, preload_buffer_size);
		REQUIRE_LT(audio_file.preloadedsize, audio_file.size);

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(1);
		audio_cache.setAsyncMode(true);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// Keep async mode enabled but stop the worker thread so events remain
		// queued deterministically.
		audio_cache.deinit();

		cacheid_t id;
		// Open queues a LoadNext event
		audio_cache.open(audio_file, 0, 0, id);
		REQUIRE_NE(CACHE_DUMMYID, id);
		REQUIRE_UNARY(!audio_cache.isReady(id));

		// Queue Close while LoadNext is still pending.
		audio_cache.close(id);

		cacheid_t second_id;
		audio_cache.open(audio_file, 0, 0, second_id);
		CHECK_EQ(CACHE_DUMMYID, second_id);
	}

	SUBCASE("dummyIdOperations")
	{
		printf("\ndummy_id_operations()\n");

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(10);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// next() with CACHE_DUMMYID increments underruns and returns nodata
		std::size_t size = FRAMESIZE;
		sample_t* result = audio_cache.next(CACHE_DUMMYID, size);
		CHECK_NE(nullptr, result);
		CHECK_EQ(std::size_t(1), settings.number_of_underruns.load());

		// isReady() with CACHE_DUMMYID returns true (no pending read)
		CHECK_UNARY(audio_cache.isReady(CACHE_DUMMYID));

		// close() with CACHE_DUMMYID is a no-op; no crash
		audio_cache.close(CACHE_DUMMYID);
	}

	SUBCASE("getterCoverage")
	{
		printf("\ngetter_coverage()\n");

		Settings settings;
		AudioCache audio_cache(settings);

		audio_cache.setFrameSize(64);
		CHECK_EQ(std::size_t(64), audio_cache.getFrameSize());

		audio_cache.setAsyncMode(true);
		CHECK_UNARY(audio_cache.isAsyncMode());

		audio_cache.setAsyncMode(false);
		CHECK_UNARY(!audio_cache.isAsyncMode());
	}

	SUBCASE("setFrameSizeGrowsNodataBuffer")
	{
		printf("\nset_frame_size_grows_nodata_buffer()\n");

		Settings settings;
		AudioCache audio_cache(settings);

		// First allocation at 64
		audio_cache.setFrameSize(64);
		CHECK_EQ(std::size_t(64), audio_cache.getFrameSize());

		// Growing triggers nodata_dirty path (old buffer saved, new allocated)
		audio_cache.setFrameSize(256);
		CHECK_EQ(std::size_t(256), audio_cache.getFrameSize());
	}

	SUBCASE("fullyPreloadedFileNoDiscStreaming")
	{
		printf("\nfully_preloaded_file_no_disc_streaming()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		// Load entire file - preloadedsize == size
		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr);
		CHECK_EQ(audio_file.preloadedsize, audio_file.size);

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(10);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		cacheid_t id;
		sample_t* data = audio_cache.open(audio_file, 0, 0, id);
		CHECK_NE(CACHE_DUMMYID, id);
		CHECK_EQ(data, audio_file.data);

		// No LoadNext event queued; no underruns expected
		CHECK_EQ(std::size_t(0), settings.number_of_underruns.load());

		audio_cache.close(id);
	}

	SUBCASE("nullFrontBufferUnderrun")
	{
		printf("\nnull_front_buffer_underrun()\n");

		// Large enough file so preloadedsize < size
		auto filename =
		    drumkit_creator.createMultiChannelWav("multi_channel.wav");

		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr, preload_buffer_size);
		REQUIRE_LT(audio_file.preloadedsize, audio_file.size);

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(100);
		audio_cache.setAsyncMode(true);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);
		// Stop worker so disk reads never complete and front stays null.
		audio_cache.deinit();

		cacheid_t id;
		audio_cache.open(audio_file, 0, 0, id);
		REQUIRE_NE(CACHE_DUMMYID, id);

		// Exhaust the preloaded buffer.
		const std::size_t iters = audio_file.preloadedsize / FRAMESIZE;
		for(std::size_t i = 0; i < iters; ++i)
		{
			std::size_t sz = FRAMESIZE;
			audio_cache.next(id, sz);
		}

		// First next() past preloaded: c.ready=false → underrun #1
		std::size_t sz = FRAMESIZE;
		audio_cache.next(id, sz);
		const auto underruns_after_first = settings.number_of_underruns.load();
		CHECK_GT(underruns_after_first, std::size_t(0));

		// Second next(): cache path with null front buffer → underrun #2
		sz = FRAMESIZE;
		audio_cache.next(id, sz);
		CHECK_GT(settings.number_of_underruns.load(), underruns_after_first);

		audio_cache.close(id);
	}

	SUBCASE("poolExhaustionUnderrunFromOpen")
	{
		printf("\npool_exhaustion_underrun_from_open()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		// Fully preload the file so open() doesn't need streaming.
		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr);
		REQUIRE_EQ(audio_file.preloadedsize, audio_file.size);

		const std::size_t pool_size = 2;
		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(pool_size);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// Fill the pool completely.
		cacheid_t ids[pool_size];
		for(std::size_t i = 0; i < pool_size; ++i)
		{
			audio_cache.open(audio_file, 0, 0, ids[i]);
			REQUIRE_NE(CACHE_DUMMYID, ids[i]);
		}
		CHECK_EQ(std::size_t(0), settings.number_of_underruns.load());

		// One more open() must exhaust the pool and return CACHE_DUMMYID.
		cacheid_t overflow_id;
		audio_cache.open(audio_file, 0, 0, overflow_id);
		CHECK_EQ(CACHE_DUMMYID, overflow_id);
		CHECK_EQ(std::size_t(1), settings.number_of_underruns.load());

		// next() with the resulting CACHE_DUMMYID also increments underruns.
		std::size_t sz = FRAMESIZE;
		sample_t* result = audio_cache.next(CACHE_DUMMYID, sz);
		CHECK_NE(nullptr, result);
		CHECK_EQ(std::size_t(2), settings.number_of_underruns.load());

		for(std::size_t i = 0; i < pool_size; ++i)
		{
			audio_cache.close(ids[i]);
		}
	}

	SUBCASE("unloadedFileNextUnderrun")
	{
		printf("\nunloaded_file_next_underrun()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		// Create AudioFile but do not call load() -- data and size remain zero.
		AudioFile audio_file(filename.c_str(), 0);
		REQUIRE_UNARY(audio_file.isValid());
		REQUIRE_EQ(nullptr, audio_file.data);

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(10);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// open() succeeds (file is valid) but sets preloaded_samples to
		// nullptr.
		cacheid_t id;
		audio_cache.open(audio_file, 0, 0, id);
		REQUIRE_NE(CACHE_DUMMYID, id);
		CHECK_EQ(std::size_t(0), settings.number_of_underruns.load());

		// next() hits the null front-buffer path and triggers an underrun.
		std::size_t sz = FRAMESIZE;
		sample_t* result = audio_cache.next(id, sz);
		CHECK_NE(nullptr, result);
		CHECK_EQ(std::size_t(1), settings.number_of_underruns.load());

		audio_cache.close(id);
	}

	SUBCASE("repeatedDummyIdUnderrunIncrements")
	{
		printf("\nrepeated_dummy_id_underrun_increments()\n");

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(10);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		// Each call to next(CACHE_DUMMYID) must increment the counter by one.
		const std::size_t iterations = 5;
		for(std::size_t i = 0; i < iterations; ++i)
		{
			std::size_t sz = FRAMESIZE;
			sample_t* result = audio_cache.next(CACHE_DUMMYID, sz);
			CHECK_NE(nullptr, result);
			CHECK_EQ(i + 1, settings.number_of_underruns.load());
		}
	}

	SUBCASE("setFrameSizeShrinkNoRealloc")
	{
		printf("\nset_frame_size_shrink_no_realloc()\n");

		Settings settings;
		AudioCache audio_cache(settings);

		// First allocation at 256.
		audio_cache.setFrameSize(256);
		CHECK_EQ(std::size_t(256), audio_cache.getFrameSize());

		// Shrinking below nodata_framesize: nodata is NOT reallocated (covers
		// the else-branch of if(framesize > nodata_framesize)), but
		// this->framesize IS updated to the new value.
		audio_cache.setFrameSize(64);
		CHECK_EQ(std::size_t(64), audio_cache.getFrameSize());
	}

	SUBCASE("destructorWithNullNodata")
	{
		printf("\ndestructor_with_null_nodata()\n");

		Settings settings;
		AudioCache audio_cache(settings);
		// setFrameSize is never called, so nodata remains nullptr.
		// When audio_cache is destroyed, the destructor executes
		// delete[] nullptr which is valid C++ and covers the null-nodata
		// branch of ~AudioCache().
	}

	SUBCASE("openWithInvalidFile")
	{
		printf("\nopen_with_invalid_file()\n");

		auto filename =
		    drumkit_creator.createSingleChannelWav("single_channel.wav");

		AudioFile audio_file(filename.c_str(), 0);
		audio_file.load(nullptr);
		REQUIRE_UNARY(audio_file.isValid());
		audio_file.invalidateForTesting();
		CHECK_UNARY_FALSE(audio_file.isValid());

		Settings settings;
		AudioCache audio_cache(settings);
		audio_cache.init(10);
		audio_cache.setAsyncMode(false);
		audio_cache.setFrameSize(FRAMESIZE);
		audio_cache.updateChunkSize(1);

		cacheid_t id;
		audio_cache.open(audio_file, 0, 0, id);
		CHECK_EQ(CACHE_DUMMYID, id);
		CHECK_EQ(std::size_t(1), settings.number_of_underruns.load());
	}
}
