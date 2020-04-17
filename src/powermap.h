/* -*- Mode: c++ -*- */
/***************************************************************************
 *            powermap.h
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
#pragma once

class Powermap
{
	using Power = float;
	struct PowerPair {
		Power in;
		Power out;
	};

public:
	Powermap() = default;

	Power map(Power in) const;
	void reset();

	// set parameters
	void setMinInput(Power min_input);
	void setMaxInput(Power max_input);
	void setFixed(PowerPair fixed);

private:
	// input parameters
	Power min_input;
	Power max_input;
	PowerPair fixed;

	// spline parameters
	float a1, b1, c1, d1;
	float a2, b2, c2, d2;

	void updateSpline();
};
