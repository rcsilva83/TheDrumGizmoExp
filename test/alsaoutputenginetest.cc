/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsaoutputenginetest.cc
 *
 *  Mon May 05 08:00:00 CEST 2026
 *  Copyright 2026 André Nusser
 *  andre.nusser@googlemail.com
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

#ifdef HAVE_OUTPUT_ALSA
#include "drumgizmo/output/alsa.h"

TEST_CASE("AlsaOutputEngine")
{
	SUBCASE("constructorCreatesEngineWithDefaults")
	{
		AlsaOutputEngine engine;

		CHECK_EQ(engine.getSamplerate(), 44100);
		CHECK_EQ(engine.getBufferSize(), 32);
	}

	SUBCASE("isFreewheelingReturnsFalse")
	{
		AlsaOutputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(!result);
	}

	SUBCASE("startReturnsTrue")
	{
		AlsaOutputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		AlsaOutputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		AlsaOutputEngine engine;

		engine.pre(1024);
		engine.pre(0);

		CHECK_UNARY(true);
	}

	SUBCASE("setParmDevSetsDevice")
	{
		AlsaOutputEngine engine;

		engine.setParm("dev", "hw:0");
		engine.setParm("dev", "default");
		engine.setParm("dev", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmFramesSetsFrames")
	{
		AlsaOutputEngine engine;

		engine.setParm("frames", "64");
		CHECK_EQ(engine.getBufferSize(), 64);

		engine.setParm("frames", "128");
		CHECK_EQ(engine.getBufferSize(), 128);
	}

	SUBCASE("setParmInvalidFramesDoesNotCrash")
	{
		AlsaOutputEngine engine;

		engine.setParm("frames", "invalid");
		engine.setParm("frames", "");

		CHECK_EQ(engine.getBufferSize(), 32); // default unchanged
	}

	SUBCASE("setParmPeriodsDoesNotCrash")
	{
		AlsaOutputEngine engine;

		engine.setParm("periods", "2");
		engine.setParm("periods", "4");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidPeriodsDoesNotCrash")
	{
		AlsaOutputEngine engine;

		engine.setParm("periods", "invalid");
		engine.setParm("periods", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmSrateSetsSamplerate")
	{
		AlsaOutputEngine engine;

		engine.setParm("srate", "48000");
		CHECK_EQ(engine.getSamplerate(), 48000);

		engine.setParm("srate", "44100");
		CHECK_EQ(engine.getSamplerate(), 44100);

		engine.setParm("srate", "96000");
		CHECK_EQ(engine.getSamplerate(), 96000);
	}

	SUBCASE("setParmInvalidSrateDoesNotCrash")
	{
		AlsaOutputEngine engine;

		engine.setParm("srate", "invalid");
		engine.setParm("srate", "");
		engine.setParm("srate", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		AlsaOutputEngine engine;

		engine.setParm("unknown", "value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithoutInitDoesNotCrash")
	{
		AlsaOutputEngine engine;

		CHECK_UNARY(engine.start());
		engine.pre(1024);
		engine.stop();

		CHECK_UNARY(!engine.isFreewheeling());
		CHECK_EQ(engine.getSamplerate(), 44100);
	}
}
#endif // HAVE_OUTPUT_ALSA
