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
}
