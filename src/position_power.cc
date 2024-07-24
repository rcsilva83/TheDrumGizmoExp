/* -*- Mode: c++ -*- */
/***************************************************************************
 *            position_power.cc
 *
 *  Wed Jul 24 15:05:27 CEST 2024
 *  Copyright 2024 Bent Bisballe Nyeng
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
#include "position_power.h"

#include "sample.h"
#include "instrument.h"

#include <set>
#include <algorithm>

Instrument::PowerRange positionPower(const std::vector<Sample*>& samplelist, double position)
{
	if(samplelist.empty())
	{
		return {0.0, 1.0};
	}

	struct PosPower
	{
		double position;
		double power;
		Sample* sample;
	};
	auto dist_cmp =
		[position](const PosPower& a, const PosPower& b)
		{
			auto position_delta =
				std::abs(a.position - position) - std::abs(b.position - position);
			if(position_delta != 0)
			{
				return position_delta < 0.0;
			}
			return a.sample < b.sample;
		};
	std::set<PosPower, decltype(dist_cmp)> sorted_samples(dist_cmp);

	std::for_each(samplelist.begin(), samplelist.end(),
	              [&](Sample* s)
	              {
		              sorted_samples.insert({s->getPosition(), s->getPower(), s});
	              });

	// Find the smallest, closest set in terms of delta-position against the note position
	// and find the contained power range.
	double power_min{std::numeric_limits<double>::max()};
	double power_max{std::numeric_limits<double>::min()};
	auto sample_iter = sorted_samples.begin();
	auto final_position_boundary = sample_iter->position;
	for(std::size_t i = 0; i < std::max(sorted_samples.size() / 4, std::size_t(1)); ++i)
	{
		auto power = sample_iter->power;
		final_position_boundary = sample_iter->position;
		power_min = std::min(power_min, power);
		power_max = std::max(power_max, power);
		++sample_iter;
	}

	// Include upcoming samples from the list as long as their distances are contained in
	// the final position range.
	while(sample_iter != sorted_samples.end())
	{
		if(sample_iter->position != final_position_boundary)
		{
			// Position has left the range - and since the list is sorted; stop.
			break;
		}

		auto power = sample_iter->power;
		power_min = std::min(power_min, power);
		power_max = std::max(power_max, power);
		++sample_iter;
	}

	return {power_min, power_max};
}
