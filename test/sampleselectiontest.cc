/* -*- Mode: c++ -*- */
/***************************************************************************
 *            sampleselectiontest.cc
 *
 *  Tue Mar 24 2026
 *  Copyright 2026
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

#include "../src/powerlist.h"
#include "../src/random.h"
#include "../src/sample.h"
#include "../src/sample_selection.h"
#include "../src/settings.h"

TEST_CASE("test_sampleselectiontest")
{
	SUBCASE("regression_0fcb75e8_two_samples_choose_closest_power")
	{
		// Regression for 0fcb75e8: two-sample selection used to short-circuit
		// and always return index 0 before evaluating candidates.
		Settings settings;
		settings.sample_selection_f_close.store(1.0f);
		settings.sample_selection_f_diverse.store(0.0f);
		settings.sample_selection_f_random.store(0.0f);

		Random random(1337);
		PowerList powerlist;
		Sample low_power_sample("low", 0.10);
		Sample high_power_sample("high", 0.90);

		powerlist.add(&high_power_sample);
		powerlist.add(&low_power_sample);
		powerlist.finalise();

		SampleSelection selection(settings, random, powerlist);
		selection.finalise();

		auto chosen = selection.get(0.85f, 64);
		CHECK_EQ(chosen, static_cast<const Sample*>(&high_power_sample));
	}

	SUBCASE("two_samples_low_velocity_prefers_lower_power_sample")
	{
		Settings settings;
		settings.sample_selection_f_close.store(1.0f);
		settings.sample_selection_f_diverse.store(0.0f);
		settings.sample_selection_f_random.store(0.0f);

		Random random(1337);
		PowerList powerlist;
		Sample low_power_sample("low", 0.10);
		Sample high_power_sample("high", 0.90);

		powerlist.add(&high_power_sample);
		powerlist.add(&low_power_sample);
		powerlist.finalise();

		SampleSelection selection(settings, random, powerlist);
		selection.finalise();

		auto chosen = selection.get(0.15f, 64);
		CHECK_EQ(chosen, static_cast<const Sample*>(&low_power_sample));
	}

	SUBCASE("regression_0_9_18_single_sample_always_returned")
	{
		// Regression for v0.9.18: sample selection with a single-sample
		// instrument must return that sample regardless of the requested
		// power level.  The old algorithm could misbehave on small sets.
		Settings settings;
		settings.sample_selection_f_close.store(1.0f);
		settings.sample_selection_f_diverse.store(0.0f);
		settings.sample_selection_f_random.store(0.0f);

		Random random(42);
		PowerList powerlist;
		Sample only_sample("only", 0.50);

		powerlist.add(&only_sample);
		powerlist.finalise();

		SampleSelection selection(settings, random, powerlist);
		selection.finalise();

		CHECK_EQ(
		    selection.get(0.0f, 0), static_cast<const Sample*>(&only_sample));
		CHECK_EQ(
		    selection.get(0.5f, 0), static_cast<const Sample*>(&only_sample));
		CHECK_EQ(
		    selection.get(1.0f, 0), static_cast<const Sample*>(&only_sample));
	}

	SUBCASE("regression_equal_power_samples_no_crash")
	{
		// Regression: when all samples share the same power value the
		// power_range calculation yields 0.  The engine guards against
		// division by zero by substituting 1.0, so selection must still
		// return a valid sample without crashing.
		Settings settings;
		settings.sample_selection_f_close.store(1.0f);
		settings.sample_selection_f_diverse.store(0.0f);
		settings.sample_selection_f_random.store(0.0f);

		Random random(7);
		PowerList powerlist;
		Sample s1("s1", 0.5);
		Sample s2("s2", 0.5);
		Sample s3("s3", 0.5);

		powerlist.add(&s1);
		powerlist.add(&s2);
		powerlist.add(&s3);
		powerlist.finalise();

		SampleSelection selection(settings, random, powerlist);
		selection.finalise();

		auto chosen = selection.get(0.5f, 0);
		CHECK_UNARY(chosen == static_cast<const Sample*>(&s1) ||
		            chosen == static_cast<const Sample*>(&s2) ||
		            chosen == static_cast<const Sample*>(&s3));
	}

	SUBCASE("regression_0_9_19_diverse_factor_avoids_immediate_repetition")
	{
		// Regression for v0.9.19: "Sample selection default values improved".
		// With the diverse factor active, the engine must avoid picking the
		// same sample over and over.  After using a sample at position P, the
		// next call at position P + samplerate should favour the OTHER sample
		// because the diverse factor penalises recently-used samples (a sample
		// used at time T has a higher — worse — diverse score at time T+delta
		// than one that has not been used recently).
		Settings settings;
		settings.samplerate.store(44100.0f);
		settings.sample_selection_f_close.store(0.0f);
		settings.sample_selection_f_diverse.store(1.0f);
		settings.sample_selection_f_random.store(0.0f);

		Random random(0);
		PowerList powerlist;
		Sample s1("s1", 0.5);
		Sample s2("s2", 0.5);

		powerlist.add(&s1);
		powerlist.add(&s2);
		powerlist.finalise();

		SampleSelection selection(settings, random, powerlist);
		selection.finalise();

		// First hit: both samples equally "cold" so the first (s1) wins the
		// strict-less-than tie-break.
		const Sample* first = selection.get(0.5f, 44100);
		CHECK_EQ(first, static_cast<const Sample*>(&s1));

		// Second hit one samplerate-period later: s1 was just used at 44100,
		// so at pos=88200 it has a higher (worse) diverse score than s2 which
		// has last==0.  The engine should therefore choose s2.
		const Sample* second = selection.get(0.5f, 88200);
		CHECK_EQ(second, static_cast<const Sample*>(&s2));
	}
}
