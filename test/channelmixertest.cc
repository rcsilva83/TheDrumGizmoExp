/* -*- Mode: c++ -*- */
/***************************************************************************
 *            channelmixertest.cc
 *
 *  Sat Apr 11 00:00:00 CET 2026
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

#include <channel.h>
#include <channelmixer.h>

TEST_CASE("ChannelMixerTest")
{
	Channels channels;
	channels.emplace_back("ch0");
	channels.emplace_back("ch1");
	channels[0].num = 0;
	channels[1].num = 1;

	SUBCASE("nullptr_default_channel_falls_back_to_first_channel")
	{
		// When the default channel is nullptr and channels is non-empty,
		// setDefaults must fall back to channels[0].
		ChannelMixer mixer(channels, nullptr, 1.0f);

		InstrumentChannel ich("ic0");
		MixerSettings& m = mixer.lookup(ich);

		CHECK_EQ(m.output, &channels[0]);
		CHECK_EQ(doctest::Approx(1.0f), m.gain);
	}

	SUBCASE("explicit_default_channel_is_used")
	{
		// When an explicit default channel is provided it must be used.
		ChannelMixer mixer(channels, &channels[1], 0.5f);

		InstrumentChannel ich("ic1");
		MixerSettings& m = mixer.lookup(ich);

		CHECK_EQ(m.output, &channels[1]);
		CHECK_EQ(doctest::Approx(0.5f), m.gain);
	}

	SUBCASE("lookup_returns_same_entry_on_second_call")
	{
		// The second call to lookup() with the same InstrumentChannel must
		// return the previously stored entry (the branch where the key is
		// already present in the map).
		ChannelMixer mixer(channels, &channels[0], 1.0f);

		InstrumentChannel ich("ic2");
		MixerSettings& m1 = mixer.lookup(ich);
		m1.gain = 0.75f;
		m1.output = &channels[1];

		MixerSettings& m2 = mixer.lookup(ich);
		CHECK_EQ(doctest::Approx(0.75f), m2.gain);
		CHECK_EQ(m2.output, &channels[1]);
	}

	SUBCASE("setDefaults_updates_default_channel_and_gain")
	{
		// After setDefaults, new (not yet looked-up) InstrumentChannels should
		// get the updated defaults.
		ChannelMixer mixer(channels, &channels[0], 1.0f);
		mixer.setDefaults(&channels[1], 0.3f);

		InstrumentChannel ich("ic3");
		MixerSettings& m = mixer.lookup(ich);

		CHECK_EQ(m.output, &channels[1]);
		CHECK_EQ(doctest::Approx(0.3f), m.gain);
	}

	SUBCASE("setDefaults_with_nullptr_falls_back_to_first_channel")
	{
		// setDefaults(nullptr, ...) must fall back to channels[0] just like
		// the constructor does.
		ChannelMixer mixer(channels, &channels[1], 1.0f);
		mixer.setDefaults(nullptr, 0.6f);

		InstrumentChannel ich("ic4");
		MixerSettings& m = mixer.lookup(ich);

		CHECK_EQ(m.output, &channels[0]);
		CHECK_EQ(doctest::Approx(0.6f), m.gain);
	}
}
