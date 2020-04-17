/* -*- Mode: c++ -*- */
/***************************************************************************
 *            powermap.cc
 *
 *  Fri Apr 17 23:06:12 CEST 2020
 *  Copyright 2020 André Nusser
 *  andre.nusser@googlemail.com
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
#include "powermap.h"

#include <cmath>

Power Powermap::map(Power in) const
{
	if (in < fixed_map.in) {
		return a1*pow(in,3) + b1*pow(in,2) + c1*in + d1;
	}
	else {
		return a2*pow(in,3) + b2*pow(in,2) + c2*in + d2;
	}
}

void Powermap::reset()
{
	min_input = 0.;
	max_input = 1.;
	fixed_map = {.5, .5};
}

void Powermap::setMinInput(Power min_input)
{
	this->min_input = min_input;
}

void Powermap::setMaxInput(Power max_input)
{
	this->max_input = max_input;
}

void Powermap::setFixed(PowerPair fixed)
{
	this->fixed = fixed;
}

void Powermap::updateSpline()
{
	// TODO
}
