/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            ossoutputenginetest.cc
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

#ifdef HAVE_OUTPUT_OSS

#include "../drumgizmo/osswrapper.h"
#include "../drumgizmo/output/oss.h"
#include "mock_wrappers.h"

#include <string>
#include <sys/soundcard.h>
#include <vector>

TEST_CASE("OSSOutputEngineWithMock")
{
	// ---- Constructor and lifecycle ----

	SUBCASE("defaultConstructorCreatesEngine")
	{
		OSSOutputEngine engine;
		CHECK_UNARY(true);
	}

	SUBCASE("injectionConstructorCreatesEngine")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsFalse")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		CHECK_UNARY(!engine.isFreewheeling());
	}

	SUBCASE("startReturnsTrue")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		CHECK_UNARY(engine.start());
	}

	SUBCASE("stopDoesNotThrow")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.stop();
		CHECK_UNARY(true);
	}

	SUBCASE("getSamplerateReturnsDefault")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		CHECK_GT(engine.getSamplerate(), 0u);
	}

	SUBCASE("preSizesData")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.pre(256);
		CHECK_UNARY(true);
	}

	// ---- setParm tests ----

	SUBCASE("setParmDevSetsDevice")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("dev", "/dev/dsp1");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmSrateSetsSamplerate")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("srate", "96000");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidSrateDoesNotCrash")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("srate", "bad");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMaxFragmentsSetsValue")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("max_fragments", "8");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMaxFragmentsClampedToMin2")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("max_fragments", "1");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmMaxFragmentsInvalidDoesNotCrash")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("max_fragments", "xyz");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeSetsValue")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("fragment_size", "16");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeClampedToMin4")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("fragment_size", "2");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeClampedToMax65535")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("fragment_size", "99999");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeInvalidDoesNotCrash")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("fragment_size", "abc");
		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownDoesNotCrash")
	{
		MockOssWrapper oss;
		OSSOutputEngine engine(oss);
		engine.setParm("nonexistent", "value");
		CHECK_UNARY(true);
	}

	// ---- init() tests ----

	SUBCASE("initSucceedsWithMockDevice")
	{
		MockOssWrapper oss;
		oss.open_return = 3;  // valid fd
		oss.ioctl_return = 0; // all ioctls succeed

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		bool result = engine.init(channels);
		CHECK_UNARY(result);
	}

	SUBCASE("initFailsWhenOpenFails")
	{
		MockOssWrapper oss;
		oss.open_return = -1; // open fails

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		bool result = engine.init(channels);
		CHECK_UNARY(!result);
	}

	SUBCASE("initFailsWhenSetFragmentFails")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = -1; // first ioctl (fragments) fails

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		bool result = engine.init(channels);
		CHECK_UNARY(!result);
	}

	SUBCASE("initWithMultipleChannels")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0; // all ioctls succeed

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;
		channels.emplace_back("ch2");
		channels[1].num = 1;

		bool result = engine.init(channels);
		CHECK_UNARY(result);
	}

	// ---- run() tests ----

	SUBCASE("runConvertsFloatToInt32")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(64, 0.5f);
		engine.pre(64);
		engine.run(0, samples.data(), 64);
		CHECK_UNARY(true);
	}

	SUBCASE("runClipsValuesAbove1")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(32, 2.0f); // above 1.0
		engine.pre(32);
		engine.run(0, samples.data(), 32);
		CHECK_UNARY(true);
	}

	SUBCASE("runClipsValuesBelowNeg1")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(32, -3.0f); // below -1.0
		engine.pre(32);
		engine.run(0, samples.data(), 32);
		CHECK_UNARY(true);
	}

	SUBCASE("runWithMultipleChannelsInterleavesData")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;
		channels.emplace_back("ch2");
		channels[1].num = 1;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(64, 0.25f);
		engine.pre(64);
		engine.run(0, samples.data(), 64);
		engine.run(1, samples.data(), 64);
		CHECK_UNARY(true);
	}

	// ---- post() tests ----

	SUBCASE("postWritesData")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;
		oss.write_return = 512;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(128, 0.25f);
		engine.pre(128);
		engine.run(0, samples.data(), 128);
		engine.post(128);
		CHECK_UNARY(true);
	}

	SUBCASE("postHandlesWriteShorterThanExpected")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;
		oss.write_return = 64; // shorter than expected

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));

		std::vector<sample_t> samples(128, 0.25f);
		engine.pre(128);
		engine.run(0, samples.data(), 128);
		engine.post(128);
		CHECK_UNARY(true);
	}

	// ---- Full lifecycle ----

	SUBCASE("fullLifecycleSucceeds")
	{
		MockOssWrapper oss;
		oss.open_return = 3;
		oss.ioctl_return = 0;
		oss.write_return = 512;

		OSSOutputEngine engine(oss);

		Channels channels;
		channels.emplace_back("ch1");
		channels[0].num = 0;

		CHECK_UNARY(engine.init(channels));
		CHECK_UNARY(engine.start());

		std::vector<sample_t> samples(128, 0.5f);
		engine.pre(128);
		engine.run(0, samples.data(), 128);
		engine.post(128);
		engine.stop();

		CHECK_UNARY(true);
	}
}

#endif // HAVE_OUTPUT_OSS
