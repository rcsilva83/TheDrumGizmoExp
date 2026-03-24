/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            inputfilterstest.cc
 *
 *  Tue Mar 24 2026
 *  Copyright 2026 The DrumGizmo Authors
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

#include <event.h>
#include <latencyfilter.h>
#include <powermapfilter.h>
#include <random.h>
#include <settings.h>
#include <staminafilter.h>
#include <velocityfilter.h>

TEST_CASE("InputFiltersTest")
{
	SUBCASE("latency_filter_disabled_is_noop")
	{
		Settings settings;
		Random random(1234);
		LatencyFilter filter(settings, random);

		event_t event{EventType::OnSet, 0, 12, 0.6f};
		CHECK(filter.filter(event, 128));

		CHECK_EQ(event.offset, 12);
		CHECK_EQ(filter.getLatency(), 0u);
		CHECK_EQ(settings.latency_current.load(), doctest::Approx(0.0f));
	}

	SUBCASE("latency_filter_applies_deterministic_offsets")
	{
		Settings settings;
		settings.enable_latency_modifier.store(true);
		settings.samplerate.store(1000.0f);
		settings.latency_max_ms.store(10.0f);
		settings.latency_laid_back_ms.store(4.0f);
		settings.latency_stddev.store(0.0f);
		settings.latency_regain.store(0.7f);

		Random random(1234);
		LatencyFilter filter(settings, random);

		event_t event{EventType::OnSet, 0, 3, 0.6f};
		CHECK(filter.filter(event, 100));

		CHECK_EQ(event.offset, 17);
		CHECK_EQ(filter.getLatency(), 10u);
		CHECK_EQ(settings.latency_current.load(), doctest::Approx(4.0f));
	}

	SUBCASE("stamina_filter_resets_when_disabled_and_recovers")
	{
		Settings settings;
		settings.samplerate.store(1000.0f);
		settings.velocity_modifier_falloff.store(1.0f);
		settings.velocity_modifier_weight.store(0.5f);

		StaminaFilter filter(settings);

		event_t disabled_event{EventType::OnSet, 2, 0, 0.8f};
		settings.enable_velocity_modifier.store(false);
		CHECK(filter.filter(disabled_event, 100));
		CHECK_EQ(disabled_event.velocity, doctest::Approx(0.8f));

		settings.enable_velocity_modifier.store(true);
		event_t first_event{EventType::OnSet, 2, 0, 1.0f};
		CHECK(filter.filter(first_event, 600));
		CHECK_EQ(first_event.velocity, doctest::Approx(1.0f));

		event_t second_event{EventType::OnSet, 2, 0, 1.0f};
		CHECK(filter.filter(second_event, 600));
		CHECK_EQ(second_event.velocity, doctest::Approx(0.5f));

		event_t third_event{EventType::OnSet, 2, 0, 1.0f};
		CHECK(filter.filter(third_event, 2600));
		CHECK_EQ(third_event.velocity, doctest::Approx(1.0f));
	}

	SUBCASE("velocity_filter_uses_configured_stddev")
	{
		Settings settings;
		settings.enable_velocity_modifier.store(true);
		settings.velocity_stddev.store(0.9f);

		Random filter_random(42);
		Random reference_random(42);
		VelocityFilter filter(settings, filter_random);

		event_t event{EventType::OnSet, 1, 0, 0.7f};
		auto expected_velocity =
		    reference_random.normalDistribution(0.7f, 0.9f / 30.0f);

		CHECK(filter.filter(event, 0));
		CHECK_EQ(event.velocity, doctest::Approx(expected_velocity));
	}

	SUBCASE("powermap_filter_reports_input_and_output")
	{
		Settings settings;
		PowermapFilter filter(settings);

		event_t bypassed_event{EventType::OnSet, 1, 0, 0.25f};
		settings.enable_powermap.store(false);
		CHECK(filter.filter(bypassed_event, 0));
		CHECK_EQ(bypassed_event.velocity, doctest::Approx(0.25f));
		CHECK_EQ(settings.powermap_input.load(), doctest::Approx(0.25f));
		CHECK_EQ(settings.powermap_output.load(), doctest::Approx(0.25f));

		settings.enable_powermap.store(true);
		settings.powermap_fixed0_x.store(0.0001f);
		settings.powermap_fixed0_y.store(0.0001f);
		settings.powermap_fixed1_x.store(0.3f);
		settings.powermap_fixed1_y.store(0.7f);
		settings.powermap_fixed2_x.store(0.9999f);
		settings.powermap_fixed2_y.store(0.9999f);
		settings.powermap_shelf.store(true);

		event_t mapped_event{EventType::OnSet, 1, 0, 0.3f};
		CHECK(filter.filter(mapped_event, 0));
		CHECK_EQ(mapped_event.velocity, doctest::Approx(0.7f));
		CHECK_EQ(settings.powermap_input.load(), doctest::Approx(0.3f));
		CHECK_EQ(settings.powermap_output.load(), doctest::Approx(0.7f));
	}
}
