/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            resource_test.cc
 *
 *  Fri Nov 13 18:50:52 CET 2015
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

#include "../dggui/resource.h"

#include "drumkit_creator.h"

class ResourceTester : public dggui::Resource
{
public:
	ResourceTester(const std::string& name) : dggui::Resource(name)
	{
	}

	bool probeIsInternal()
	{
		return isInternal;
	}
};

struct ResourceTestFixture
{
	DrumkitCreator drumkit_creator;
};

TEST_CASE_FIXTURE(ResourceTestFixture, "ResourceTest")
{
	SUBCASE("externalReadTest")
	{
		auto filename = drumkit_creator.create0000Wav("0000.wav");

		ResourceTester rc(filename);
		CHECK(!rc.probeIsInternal());
		CHECK(rc.valid());
		CHECK_EQ((size_t)46, rc.size());
	}

	SUBCASE("internalReadTest")
	{
		ResourceTester rc(":resources/bg.png");
		CHECK(rc.probeIsInternal());
		CHECK(rc.valid());
		CHECK_EQ((size_t)1123, rc.size());
	}

	SUBCASE("failTest")
	{
		{
			ResourceTester rc("/tmp/");
			CHECK(!rc.valid());
		}

		{
			ResourceTester rc("no_such_file");
			CHECK(!rc.valid());
		}

		{
			ResourceTester rc(":no_such_file");
			CHECK(!rc.valid());
		}
	}
}
