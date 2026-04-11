/* -*- Mode: c++ -*- */
/***************************************************************************
 *            audiofiletest.cc
 *
 *  Sat Apr 11 2026
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

#include <string>
#include <vector>

#include <audiofile.h>
#include <logger.h>

#include "drumkit_creator.h"

TEST_CASE("AudioFileTest")
{
	DrumkitCreator creator;

	SUBCASE("load_nonexistent_file_returns_without_data_no_logger")
	{
		// Calling load() with a non-existent path and no logger must not
		// crash and must leave the file in an unloaded state.
		AudioFile af("/path/that/does/not/exist.wav", 0);
		af.load(nullptr);

		CHECK(!af.isLoaded());
		CHECK(af.data == nullptr);
	}

	SUBCASE("load_nonexistent_file_calls_logger_with_warning")
	{
		// When a logger is provided and the file cannot be opened, the
		// logger must be called with LogLevel::Warning.
		AudioFile af("/path/that/does/not/exist.wav", 0);

		std::vector<std::string> messages;
		auto logger = [&](LogLevel lvl, const std::string& msg)
		{
			if(lvl == LogLevel::Warning)
			{
				messages.push_back(msg);
			}
		};

		af.load(logger);

		CHECK(!af.isLoaded());
		CHECK(!messages.empty());
	}

	SUBCASE("load_valid_mono_file_populates_data")
	{
		// Loading a valid mono WAV file must mark the file as loaded and
		// populate the data pointer.
		std::string wav_path = creator.createSingleChannelWav("test_mono");
		AudioFile af(wav_path, 0);
		af.load(nullptr);

		CHECK(af.isLoaded());
		CHECK(af.data != nullptr);
		CHECK(af.size > 0u);
	}

	SUBCASE("load_called_twice_is_idempotent")
	{
		// Calling load() a second time when the file is already loaded must
		// return immediately (the "already loaded" early-exit branch).
		std::string wav_path = creator.createSingleChannelWav("test_idempotent");
		AudioFile af(wav_path, 0);
		af.load(nullptr);

		CHECK(af.isLoaded());
		std::size_t first_size = af.size;

		// Second load – must not crash and data must remain valid.
		af.load(nullptr);
		CHECK(af.isLoaded());
		CHECK_EQ(first_size, af.size);
	}

	SUBCASE("load_multi_channel_file_channel_zero")
	{
		// Loading channel 0 from a stereo (2-channel) WAV file must succeed
		// and populate data.
		std::string wav_path = creator.createMultiChannelWav("test_stereo");
		AudioFile af(wav_path, 0);
		af.load(nullptr);

		CHECK(af.isLoaded());
		CHECK(af.data != nullptr);
	}

	SUBCASE("load_multi_channel_file_out_of_range_channel_clamped")
	{
		// When filechannel is >= the number of channels in the file the
		// channel must be clamped to the last valid channel.  With a
		// logger the warning is emitted; without a logger it is still safe.
		std::string wav_path = creator.createMultiChannelWav("test_oor");

		// Use filechannel=99, which is well beyond the number of channels in
		// a typical test WAV file.  This covers the logger branch too.
		AudioFile af(wav_path, 99);

		std::vector<std::string> messages;
		auto logger = [&](LogLevel lvl, const std::string& msg)
		{
			if(lvl == LogLevel::Warning)
			{
				messages.push_back(msg);
			}
		};

		af.load(logger);

		// Even with an out-of-range channel the load must succeed (clamped).
		CHECK(af.isLoaded());

		// The logger must have been called with the channel warning.
		CHECK(!messages.empty());
	}

	SUBCASE("unload_then_reload_succeeds")
	{
		// After unloading, loading again must work without issues.
		std::string wav_path = creator.createSingleChannelWav("test_reload");
		AudioFile af(wav_path, 0);

		af.load(nullptr);
		CHECK(af.isLoaded());

		af.unload();
		CHECK(!af.isLoaded());

		af.load(nullptr);
		CHECK(af.isLoaded());
		CHECK(af.data != nullptr);
	}
}
