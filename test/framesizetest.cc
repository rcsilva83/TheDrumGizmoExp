/* -*- Mode: c++ -*- */
/***************************************************************************
 *            framesizetest.cc
 *
 *  Thu Aug  2 12:16:27 CEST 2018
 *  Copyright 2018 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DrumGizmo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DrumGizmo; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include <doctest/doctest.h>

#include <audiocache.h>
#include <settings.h>

TEST_CASE("AudioCacheFrameSize")
{
	SUBCASE("initial_framesize_is_zero")
	{
		Settings settings;
		AudioCache cache(settings);
		CHECK_EQ(std::size_t(0), cache.getFrameSize());
	}

	SUBCASE("set_and_get_framesize_roundtrip")
	{
		Settings settings;
		AudioCache cache(settings);
		cache.setFrameSize(64);
		CHECK_EQ(std::size_t(64), cache.getFrameSize());
	}

	SUBCASE("increase_then_decrease_framesize")
	{
		// Growing the framesize reallocates the nodata buffer; shrinking it
		// keeps the larger allocation but must still report the current size.
		Settings settings;
		AudioCache cache(settings);
		cache.setFrameSize(128);
		CHECK_EQ(std::size_t(128), cache.getFrameSize());
		cache.setFrameSize(64);
		CHECK_EQ(std::size_t(64), cache.getFrameSize());
	}

	SUBCASE("repeated_set_same_framesize")
	{
		Settings settings;
		AudioCache cache(settings);
		cache.setFrameSize(256);
		cache.setFrameSize(256);
		CHECK_EQ(std::size_t(256), cache.getFrameSize());
	}
}
