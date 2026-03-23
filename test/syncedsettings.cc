/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            syncedsettings.cc
 *
 *  Wed Mar 31 09:32:12 CET 2016
 *  Copyright 2016 Christian Glöckner
 *  cgloeckner@freenet.de
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

#include <syncedsettings.h>

struct SyncedSettingsTestFixture
{
	struct TestData
	{
		float foo;
		bool bar;
		std::string msg;
	};
};

TEST_CASE_FIXTURE(SyncedSettingsTestFixture, "SyncedSettingsTest")
{
	SUBCASE("groupCanBeDefaultInitialized")
	{
		Group<TestData> data;
	}

	SUBCASE("groupDataCanBeCopied")
	{
		Group<TestData> data;
		(TestData) data; // copies
	}

	SUBCASE("accessorCanSetFields")
	{
		Group<TestData> data;
		{
			Accessor<TestData> a{data};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		TestData copy = data;
		CHECK_EQ(copy.foo, 3.f);
		CHECK_EQ(copy.bar, false);
		CHECK_EQ(copy.msg, std::string{"hello"});
	}

	SUBCASE("accessorCanGetFields")
	{
		Group<TestData> data;
		{
			Accessor<TestData> a{data};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		// now read
		{
			Accessor<TestData> a{data};
			CHECK_EQ(a.data.foo, 3.f);
			CHECK_EQ(a.data.bar, false);
			CHECK_EQ(a.data.msg, std::string{"hello"});
		}
	}

	SUBCASE("groupHasCopyCtor")
	{
		Group<TestData> tmp;
		{
			Accessor<TestData> a{tmp};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		Group<TestData> data{tmp};
		TestData copy = data;
		CHECK_EQ(copy.foo, 3.f);
		CHECK_EQ(copy.bar, false);
		CHECK_EQ(copy.msg, std::string{"hello"});
	}

	SUBCASE("groupHasMoveCtor")
	{
		Group<TestData> tmp;
		{
			Accessor<TestData> a{tmp};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		Group<TestData> data{std::move(tmp)};
		TestData copy = data;
		CHECK_EQ(copy.foo, 3.f);
		CHECK_EQ(copy.bar, false);
		CHECK_EQ(copy.msg, std::string{"hello"});
	}

	SUBCASE("groupHasCopyAssignOp")
	{
		Group<TestData> tmp;
		{
			Accessor<TestData> a{tmp};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		Group<TestData> data = tmp;
		TestData copy = data;
		CHECK_EQ(copy.foo, 3.f);
		CHECK_EQ(copy.bar, false);
		CHECK_EQ(copy.msg, std::string{"hello"});
	}

	SUBCASE("groupHasMoveAssignOp")
	{
		Group<TestData> tmp;
		{
			Accessor<TestData> a{tmp};
			a.data.foo = 3.f;
			a.data.bar = false;
			a.data.msg = "hello";
		}
		Group<TestData> data = std::move(tmp);
		TestData copy = data;
		CHECK_EQ(copy.foo, 3.f);
		CHECK_EQ(copy.bar, false);
		CHECK_EQ(copy.msg, std::string{"hello"});
	}

	SUBCASE("mimicRealUse")
	{
		struct Settings
		{
			struct Foo
			{
				float a{5};
				float b{3};
				bool enabled{true};
			};
			struct Bar
			{
				std::string label{"empty"};
				float bla{0.f};
			};

			Group<Foo> foo;
			Group<Bar> bar;
		};

		Settings s;

		// set bar settings
		{
			Accessor<Settings::Bar> tmp{s.bar};
			tmp.data.label = "hello world";
			tmp.data.bla = 3.14f;
		}

		// read foo settings
		{
			Accessor<Settings::Foo> tmp{s.foo};
			if(tmp.data.enabled)
			{
				// do some while locked
			}
		}
		// or:
		Settings::Foo copy = s.foo;
		if(copy.enabled)
		{
			// do some stuff without locking
		}
		CHECK(copy.enabled);
	}
}
