/* -*- Mode: c++ -*- */
/***************************************************************************
 *            doubletriggerfilter.cc
 *
 *  Fri Jun  5 18:26:32 CEST 2020
 *  Copyright 2020 Bent Bisballe Nyeng
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
#include "doubletriggerfilter.h"

#include "latencyfilter.h"

#include <cmath>
#include <hugin.hpp>

#include "settings.h"
#include "random.h"

DoubleTriggerFilter::DoubleTriggerFilter(Settings& settings)
	: settings(settings)
{
	lastStrokes.resize(127, 0.0); // initialise to all zeros
}

template<typename T1, typename T2>
static T1 getDistanceSamples(T1 min_dist_ms, T2 samplerate)
{
	return min_dist_ms * samplerate / 1000.;
}

bool DoubleTriggerFilter::filter(event_t& event, std::size_t pos)
{
	auto enabled = true;//settings.enable_latency_modifier.load();
	auto min_dist_ms = 25.0;//settings.latency_max_ms.load();
	auto samplerate = settings.samplerate.load();

	if(!enabled)
	{
		return true;
	}

	auto min_dist = getDistanceSamples(min_dist_ms, samplerate);
	double note_pos = pos + event.offset;
	if(event.instrument >= lastStrokes.size())
	{
		lastStrokes.resize(event.instrument, 0.0); // expand and initialise to zeros
	}

	auto lastStroke = lastStrokes[event.instrument];
	if((note_pos - lastStroke) > min_dist)
	{
		lastStrokes[event.instrument] = note_pos;
		return true;
	}

	return false;
}
