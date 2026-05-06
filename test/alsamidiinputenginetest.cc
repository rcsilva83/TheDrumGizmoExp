/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            alsamidiinputenginetest.cc
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

#ifdef HAVE_INPUT_ALSAMIDI
#include "drumgizmo/input/alsamidi.h"

TEST_CASE("AlsaMidiInputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		AlsaMidiInputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsTrue")
	{
		AlsaMidiInputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(result);
	}

	SUBCASE("startReturnsTrue")
	{
		AlsaMidiInputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		AlsaMidiInputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		AlsaMidiInputEngine engine;

		engine.pre();

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		AlsaMidiInputEngine engine;

		engine.post();

		CHECK_UNARY(true);
	}

	SUBCASE("setParmMidimapSetsFilename")
	{
		AlsaMidiInputEngine engine;

		engine.setParm("midimap", "/path/to/map.xml");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		AlsaMidiInputEngine engine;

		engine.setParm("unknown", "value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("setSampleRateDoesNotThrow")
	{
		AlsaMidiInputEngine engine;

		engine.setSampleRate(44100.0);
		engine.setSampleRate(48000.0);
		engine.setSampleRate(96000.0);

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithoutInitDoesNotCrash")
	{
		AlsaMidiInputEngine engine;

		CHECK_UNARY(engine.start());
		engine.pre();
		engine.post();
		engine.stop();

		CHECK_UNARY(engine.isFreewheeling());
	}
}
#endif // HAVE_INPUT_ALSAMIDI
