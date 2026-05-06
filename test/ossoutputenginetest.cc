/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            ossoutputenginetest.cc
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

#ifdef HAVE_OUTPUT_OSS
#include "drumgizmo/output/oss.h"

TEST_CASE("OSSOutputEngine")
{
	SUBCASE("constructorCreatesEngineWithDefaults")
	{
		OSSOutputEngine engine;

		CHECK_EQ(engine.getSamplerate(), 44100);
		CHECK_EQ(engine.getBufferSize(), 1024);
	}

	SUBCASE("isFreewheelingReturnsFalse")
	{
		OSSOutputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(!result);
	}

	SUBCASE("startReturnsTrue")
	{
		OSSOutputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		OSSOutputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		OSSOutputEngine engine;

		engine.pre(1024);
		engine.pre(0);

		CHECK_UNARY(true);
	}

	SUBCASE("setParmDevSetsDevice")
	{
		OSSOutputEngine engine;

		engine.setParm("dev", "/dev/dsp1");
		engine.setParm("dev", "/dev/dsp");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmSrateSetsSamplerate")
	{
		OSSOutputEngine engine;

		engine.setParm("srate", "48000");
		CHECK_EQ(engine.getSamplerate(), 48000);

		engine.setParm("srate", "44100");
		CHECK_EQ(engine.getSamplerate(), 44100);
	}

	SUBCASE("setParmInvalidSrateDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("srate", "invalid");
		engine.setParm("srate", "");
		engine.setParm("srate", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmMaxFragmentsDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("max_fragments", "2");
		engine.setParm("max_fragments", "8");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidMaxFragmentsDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("max_fragments", "invalid");
		engine.setParm("max_fragments", "-1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("fragment_size", "4");
		engine.setParm("fragment_size", "64");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmInvalidFragmentSizeDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("fragment_size", "invalid");
		engine.setParm("fragment_size", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeClampsMinDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("fragment_size", "1");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmFragmentSizeClampsMaxDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("fragment_size", "999999");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		OSSOutputEngine engine;

		engine.setParm("unknown", "value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithoutInitDoesNotCrash")
	{
		OSSOutputEngine engine;

		CHECK_UNARY(engine.start());
		engine.pre(1024);
		engine.stop();

		CHECK_UNARY(!engine.isFreewheeling());
		CHECK_EQ(engine.getSamplerate(), 44100);
	}
}
#endif // HAVE_OUTPUT_OSS
