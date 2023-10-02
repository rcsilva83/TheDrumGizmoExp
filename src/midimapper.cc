/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            midimapper.cc
 *
 *  Mon Jul 21 15:24:08 CEST 2008
 *  Copyright 2008 Bent Bisballe Nyeng
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
#include "midimapper.h"

int MidiMapper::lookup(int note, int controller)
{
	std::lock_guard<std::mutex> guard(mutex);

	auto midimap_it = midimap.find(note);
	if(midimap_it == midimap.end())
	{
		return -1;
	}

	auto instrmap_it = instrmap.find(midimap_it->second);
	if(instrmap_it == instrmap.end())
	{
		return -1;
	}

	if(controller >= 0 && !controlthreshmap[note].empty())
	{
		// find instrument where controller is above threshold with smallest distance to threshold
		int diff = 10000;
		std::string instr = controlthreshmap[note].begin()->first;
		for(auto& c : controlthreshmap[note])
		{
			int cur_diff = controller - c.second;
			if(cur_diff >= 0 && cur_diff < diff)
			{
				diff = cur_diff;
				instr = c.first;
			}
		}
		instrmap_it = instrmap.find(instr);
	}

	return instrmap_it->second;
}

void MidiMapper::swap(instrmap_t& instrmap, midimap_t& midimap, controlthreshmap_t& controlthreshmap)
{
	std::lock_guard<std::mutex> guard(mutex);

	std::swap(this->instrmap, instrmap);
	std::swap(this->midimap, midimap);
	std::swap(this->controlthreshmap, controlthreshmap);
}

const midimap_t& MidiMapper::getMap()
{
	return midimap;
}
