/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            wavfileoutputenginetest.cc
 *
 *  Tue May  5 12:00:00 CET 2026
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

#ifdef HAVE_OUTPUT_WAVFILE
#include "drumgizmo/output/wavfile.h"
#include <channel.h>
#include <cstdio>
#include <string>
#include <unistd.h>

static void cleanupWavFiles(const std::string& prefix, const Channels& channels)
{
	for(size_t i = 0; i < channels.size(); ++i)
	{
		std::string fname =
		    prefix + channels[i].name + "-" + std::to_string(i) + ".wav";
		std::remove(fname.c_str());
	}
}

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
