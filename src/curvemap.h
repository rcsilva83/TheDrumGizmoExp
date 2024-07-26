/* -*- Mode: c++ -*- */
/***************************************************************************
 *            curvemap.h
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

#include <array>
#include <vector>

class CurveMap
{
public:
	using CurveValue = float;
	using CurveValues = std::vector<CurveValue>;

	bool operator==(const CurveMap& other) const;

	struct CurveValuePair
	{
		CurveValue in;
		CurveValue out;

		bool operator==(const CurveValuePair& other)
		{
			return in == other.in || out == other.out;
		}
		bool operator!=(const CurveValuePair& other)
		{
			return !(*this == other);
		}
	};

	CurveValue map(CurveValue in);
	void reset();

	void setFixed0(CurveValuePair new_value);
	void setFixed1(CurveValuePair new_value);
	void setFixed2(CurveValuePair new_value);
	void setShelf(bool enable);

	//! If enabled, inversion inverts (1 - x) the input value before mapping
	//! it through the curve.
	void setInvert(bool enable);

	CurveValuePair getFixed0() const;
	CurveValuePair getFixed1() const;
	CurveValuePair getFixed2() const;
	bool getShelf() const;
	bool getInvert() const;

private:
	static constexpr CurveValue eps = 1e-4;
	static constexpr std::array<CurveValuePair, 3> default_fixed {
		CurveValuePair {eps, eps},
		CurveValuePair {.5, .5},
		CurveValuePair {1 - eps, 1 - eps}
	};

	// input parameters (state of this class)
	std::array<CurveValuePair, 3> fixed { default_fixed };
	bool shelf { true };
	bool invert { false };

	// spline parameters (deterministically computed from the input parameters)
	bool spline_needs_update { true };
	std::array<float, 5> m { 0, 0, 0, 0, 0 };

	void updateSpline();
	std::vector<float> calcSlopes(const CurveValues& X, const CurveValues& P);

	CurveValue clamp(CurveValue in, CurveValue min, CurveValue max) const;
};