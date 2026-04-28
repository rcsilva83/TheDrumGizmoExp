/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumkittest.cc
 *
 *  Mon Apr 28 12:00:00 UTC 2026
 *  Copyright 2026 DrumGizmo contributors
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

#include <drumkit.h>
#include <channel.h>
#include <versionstr.h>

TEST_CASE("drumkit_construction")
{
	SUBCASE("constructor_is_valid")
	{
		DrumKit kit;
		CHECK_UNARY(kit.isValid());
	}

	SUBCASE("default_samplerate_is_44100")
	{
		DrumKit kit;
		CHECK_EQ(44100.0f, doctest::Approx(kit.getSamplerate()));
	}

	SUBCASE("default_name_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(std::string(""), kit.getName());
	}

	SUBCASE("default_description_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(std::string(""), kit.getDescription());
	}

	SUBCASE("default_version_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(VersionStr(""), kit.getVersion());
	}

	SUBCASE("default_file_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(std::string(""), kit.getFile());
	}

	SUBCASE("default_number_of_files_is_zero")
	{
		DrumKit kit;
		CHECK_EQ(0u, kit.getNumberOfFiles());
	}

	SUBCASE("default_instruments_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(0u, kit.instruments.size());
	}

	SUBCASE("default_channels_is_empty")
	{
		DrumKit kit;
		CHECK_EQ(0u, kit.channels.size());
	}
}

TEST_CASE("drumkit_clear")
{
	SUBCASE("clear_resets_samplerate_to_default")
	{
		DrumKit kit;
		// samplerate defaults to 44100 and clear() resets it
		kit.clear();
		CHECK_EQ(44100.0f, doctest::Approx(kit.getSamplerate()));
	}

	SUBCASE("clear_empties_instruments")
	{
		DrumKit kit;
		// instruments should already be empty
		kit.clear();
		CHECK_EQ(0u, kit.instruments.size());
	}

	SUBCASE("clear_empties_channels")
	{
		DrumKit kit;
		kit.channels.emplace_back("test");
		kit.clear();
		CHECK_EQ(0u, kit.channels.size());
	}

	SUBCASE("clear_resets_number_of_files")
	{
		DrumKit kit;
		kit.clear();
		CHECK_EQ(0u, kit.getNumberOfFiles());
	}

	SUBCASE("clear_resets_name")
	{
		DrumKit kit;
		kit.clear();
		CHECK_EQ(std::string(""), kit.getName());
	}

	SUBCASE("clear_resets_description")
	{
		DrumKit kit;
		kit.clear();
		CHECK_EQ(std::string(""), kit.getDescription());
	}
}

TEST_CASE("drumkit_lifecycle")
{
	SUBCASE("constructor_isValid")
	{
		DrumKit kit;
		CHECK_UNARY(kit.isValid());
	}

	SUBCASE("two_instances_have_independent_validity")
	{
		DrumKit kit1;
		DrumKit kit2;
		CHECK_UNARY(kit1.isValid());
		CHECK_UNARY(kit2.isValid());
	}

	SUBCASE("destructor_invalidates")
	{
		auto* raw = new DrumKit();
		CHECK_UNARY(raw->isValid());
		delete raw;
		// After destruction, the magic pointer is set to NULL in ~DrumKit,
		// but we can't test that since the object is destroyed.
		// This test just verifies construction and cleanup don't crash.
	}
}

TEST_CASE("channel_construction")
{
	SUBCASE("default_constructor")
	{
		Channel ch;
		CHECK_EQ(std::string(""), ch.name);
		CHECK_EQ(NO_CHANNEL, ch.num);
	}

	SUBCASE("named_constructor")
	{
		Channel ch("kick");
		CHECK_EQ(std::string("kick"), ch.name);
		CHECK_EQ(NO_CHANNEL, ch.num);
	}
}

TEST_CASE("instrument_channel_construction")
{
	SUBCASE("default_constructor")
	{
		InstrumentChannel ch;
		CHECK_EQ(std::string(""), ch.name);
		CHECK_EQ(NO_CHANNEL, ch.num);
		CHECK_EQ(main_state_t::unset, ch.main);
	}

	SUBCASE("named_constructor")
	{
		InstrumentChannel ch("snare");
		CHECK_EQ(std::string("snare"), ch.name);
		CHECK_EQ(NO_CHANNEL, ch.num);
		CHECK_EQ(main_state_t::unset, ch.main);
	}
}
