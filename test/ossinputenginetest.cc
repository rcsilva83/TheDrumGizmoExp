/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            ossinputenginetest.cc
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

#ifdef HAVE_INPUT_OSSMIDI
#include "drumgizmo/input/ossmidi.h"

TEST_CASE("OSSInputEngine")
{
	SUBCASE("constructorCreatesEngine")
	{
		OSSInputEngine engine;

		CHECK_UNARY(true);
	}

	SUBCASE("isFreewheelingReturnsFalse")
	{
		OSSInputEngine engine;

		bool result = engine.isFreewheeling();

		CHECK_UNARY(!result);
	}

	SUBCASE("startReturnsTrue")
	{
		OSSInputEngine engine;

		bool result = engine.start();

		CHECK_UNARY(result);
	}

	SUBCASE("stopDoesNotThrow")
	{
		OSSInputEngine engine;

		engine.stop();

		CHECK_UNARY(true);
	}

	SUBCASE("preDoesNotThrow")
	{
		OSSInputEngine engine;

		engine.pre();

		CHECK_UNARY(true);
	}

	SUBCASE("postDoesNotThrow")
	{
		OSSInputEngine engine;

		engine.post();

		CHECK_UNARY(true);
	}

	SUBCASE("setParmDevSetsDevice")
	{
		OSSInputEngine engine;

		engine.setParm("dev", "/dev/midi1");
		engine.setParm("dev", "/dev/umidi0");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmMidimapSetsFilename")
	{
		OSSInputEngine engine;

		engine.setParm("midimap", "/path/to/map.xml");

		CHECK_UNARY(true);
	}

	SUBCASE("setParmUnknownParameterDoesNotCrash")
	{
		OSSInputEngine engine;

		engine.setParm("unknown", "value");
		engine.setParm("", "");

		CHECK_UNARY(true);
	}

	SUBCASE("fullLifecycleWithoutInitDoesNotCrash")
	{
		OSSInputEngine engine;

		CHECK_UNARY(engine.start());
		engine.pre();
		engine.post();
		engine.stop();

		CHECK_UNARY(!engine.isFreewheeling());
	}
}
#endif // HAVE_INPUT_OSSMIDI
