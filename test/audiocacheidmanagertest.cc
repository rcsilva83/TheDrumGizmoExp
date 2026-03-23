/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            audiocacheidmanagertest.cc
 *
 *  Thu Jan  7 15:42:31 CET 2016
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

#include <audiocacheidmanager.h>

class TestableAudioCacheIDManager : public AudioCacheIDManager
{
public:
	int getAvailableIDs()
	{
		return available_ids.size();
	}
};

TEST_CASE("AudioCacheIDManagerTest")
{
	SUBCASE("registerReleaseTest")
	{
		TestableAudioCacheIDManager manager;
		manager.init(2);

		cache_t c1;
		// cppcheck-suppress cstyleCast
		c1.afile = (AudioCacheFile*)1;
		auto id1 = manager.registerID(c1);
		CHECK(id1 != CACHE_DUMMYID);
		CHECK(id1 != CACHE_NOID);
		CHECK_EQ(1, manager.getAvailableIDs());

		cache_t c2;
		// cppcheck-suppress cstyleCast
		c2.afile = (AudioCacheFile*)2;
		auto id2 = manager.registerID(c2);
		CHECK(id2 != CACHE_DUMMYID);
		CHECK(id2 != CACHE_NOID);
		CHECK_EQ(0, manager.getAvailableIDs());

		cache_t c3;
		// cppcheck-suppress cstyleCast
		c3.afile = (AudioCacheFile*)3;
		auto id3 = manager.registerID(c3);
		CHECK(id3 == CACHE_DUMMYID);
		CHECK_EQ(0, manager.getAvailableIDs());

		cache_t& tc1 = manager.getCache(id1);
		CHECK_EQ(c1.afile, tc1.afile);

		cache_t& tc2 = manager.getCache(id2);
		CHECK_EQ(c2.afile, tc2.afile);

		manager.releaseID(id1);
		CHECK_EQ(1, manager.getAvailableIDs());

		cache_t c4;
		// cppcheck-suppress cstyleCast
		c4.afile = (AudioCacheFile*)4;
		auto id4 = manager.registerID(c4);
		CHECK(id4 != CACHE_DUMMYID);
		CHECK(id4 != CACHE_NOID);
		CHECK_EQ(0, manager.getAvailableIDs());

		cache_t& tc4 = manager.getCache(id4);
		CHECK_EQ(c4.afile, tc4.afile);

		manager.releaseID(id2);
		CHECK_EQ(1, manager.getAvailableIDs());

		manager.releaseID(id4);
		CHECK_EQ(2, manager.getAvailableIDs());
	}
}
