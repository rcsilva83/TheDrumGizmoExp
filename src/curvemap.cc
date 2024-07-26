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
#include "curvemap.h"

#include <cassert>
#include <cmath>

namespace
{

using CurveValue = CurveMap::CurveValue;
using CurveValuePair = CurveMap::CurveValuePair;

CurveValue h00(CurveValue x)
{
	return (1 + 2 * x) * pow(1 - x, 2);
}

CurveValue h10(CurveValue x)
{
	return x * pow(1 - x, 2);
}

CurveValue h01(CurveValue x)
{
	return x * x * (3 - 2 * x);
}

CurveValue h11(CurveValue x)
{
	return x * x * (x - 1);
}

CurveValue computeValue(const CurveValue x, const CurveValuePair& P0, const CurveValuePair& P1,
                        const CurveValue m0, const CurveValue m1)
{
	const auto x0 = P0.in;
	const auto x1 = P1.in;
	const auto y0 = P0.out;
	const auto y1 = P1.out;
	const auto dx = x1 - x0;
	const auto x_prime = (x - x0)/dx;

	return
		h00(x_prime) * y0 +
		h10(x_prime) * dx * m0 +
		h01(x_prime) * y1 +
		h11(x_prime) * dx * m1;
}

} // end anonymous namespace

constexpr std::array<CurveValuePair, 3> CurveMap::default_fixed;

CurveValue CurveMap::map(CurveValue in)
{
	assert(in >= 0. && in <= 1.);
	if (invert)
	{
		in = 1.0 - in;
	}

	if (spline_needs_update)
	{
		updateSpline();
	}

	CurveValue out;
	if (in < fixed[0].in)
	{
		out = shelf ? fixed[0].out
		            : computeValue(in, {0.,0.}, fixed[0], m[0], m[1]);
	}
	else if (in < fixed[1].in)
	{
		out = computeValue(in, fixed[0], fixed[1], m[1], m[2]);
	}
	else if (in < fixed[2].in)
	{
		out = computeValue(in, fixed[1], fixed[2], m[2], m[3]);
	}
	else
	{
		// in >= fixed[2].in
		out = shelf ? fixed[2].out
		            : computeValue(in, fixed[2], {1.,1.}, m[3], m[4]);
	}

	assert(out >= 0. && out <= 1.);
	return out;
}

void CurveMap::reset()
{
	*this = CurveMap{};

	updateSpline();
}

void CurveMap::setFixed0(CurveValuePair new_value)
{
	auto prev = fixed[0];
	fixed[0].in = clamp(new_value.in, eps, fixed[1].in - eps);
	fixed[0].out = clamp(new_value.out, eps, fixed[1].out - eps);
	if (fixed[0] != prev)
	{
		spline_needs_update = true;
	}
}

void CurveMap::setFixed1(CurveValuePair new_value)
{
	auto prev = fixed[1];
	fixed[1].in = clamp(new_value.in, fixed[0].in + eps, fixed[2].in - eps);
	fixed[1].out = clamp(new_value.out, fixed[0].out + eps, fixed[2].out - eps);
	if (fixed[1] != prev)
	{
		spline_needs_update = true;
	}
}

void CurveMap::setFixed2(CurveValuePair new_value)
{
	auto prev = fixed[2];
	fixed[2].in = clamp(new_value.in, fixed[1].in + eps, 1 - eps);
	fixed[2].out = clamp(new_value.out, fixed[1].out + eps, 1 - eps);
	if (fixed[2] != prev)
	{
		spline_needs_update = true;
	}
}

void CurveMap::setInvert(bool enable)
{
	if (invert != enable)
	{
		spline_needs_update = true;
		invert = enable;
	}
}

void CurveMap::setShelf(bool enable)
{
	if (shelf != enable)
	{
		spline_needs_update = true;
		shelf = enable;
	}
}

CurveValuePair CurveMap::getFixed0() const
{
	return fixed[0];
}

CurveValuePair CurveMap::getFixed1() const
{
	return fixed[1];
}

CurveValuePair CurveMap::getFixed2() const
{
	return fixed[2];
}

bool CurveMap::getInvert() const {
	return invert;
}

bool CurveMap::getShelf() const {
	return shelf;
}

// This mostly followes the wikipedia article for monotone cubic splines:
// https://en.wikipedia.org/wiki/Monotone_cubic_interpolation
void CurveMap::updateSpline()
{
	assert(0. <= fixed[0].in && fixed[0].in < fixed[1].in &&
	       fixed[1].in < fixed[2].in && fixed[2].in <= 1.);
	assert(0. <= fixed[0].out && fixed[0].out <= fixed[1].out &&
	       fixed[1].out <= fixed[2].out && fixed[2].out <= 1.);

	CurveValues X = shelf ? CurveValues{fixed[0].in, fixed[1].in, fixed[2].in}
	                 : CurveValues{0., fixed[0].in, fixed[1].in, fixed[2].in, 1.};
	CurveValues Y = shelf ? CurveValues{fixed[0].out, fixed[1].out, fixed[2].out}
	                 : CurveValues{0., fixed[0].out, fixed[1].out, fixed[2].out, 1.};

	auto slopes = calcSlopes(X, Y);

	if (shelf)
	{
		assert(slopes.size() == 3);
		this->m[1] = slopes[0];
		this->m[2] = slopes[1];
		this->m[3] = slopes[2];
	}
	else
	{
		assert(slopes.size() == 5);
		for (std::size_t i = 0; i < m.size(); ++i)
		{
			this->m[i] = slopes[i];
		}
	}

	spline_needs_update = false;
}

// This follows the monotone cubic spline algorithm of Steffen, from:
// "A Simple Method for Monotonic Interpolation in One Dimension"
std::vector<float> CurveMap::calcSlopes(const CurveValues& X, const CurveValues& Y)
{
	CurveValues m(X.size());

	CurveValues d(X.size() - 1);
	CurveValues h(X.size() - 1);
	for (std::size_t i = 0; i < d.size(); ++i)
	{
		h[i] = X[i + 1] - X[i];
		d[i] = (Y[i + 1] - Y[i]) / h[i];
	}

	m.front() = d.front();
	for (std::size_t i = 1; i < m.size() - 1; ++i)
	{
		m[i] = (d[i - 1] + d[i]) / 2.;
	}
	m.back() = d.back();

	for (std::size_t i = 1; i < m.size() - 1; ++i)
	{
		const auto min_d = 2*std::min(d[i - 1], d[i]);
		m[i] =
			std::min<float>(min_d,
			                (h[i] * d[i - 1] + h[i - 1] * d[i]) / (h[i - 1] + h[i]));
	}

	return m;
}

CurveValue CurveMap::clamp(CurveValue in, CurveValue min, CurveValue max) const
{
	return std::max(min, std::min(in, max));
}

bool CurveMap::operator==(const CurveMap& other) const
{
	return getFixed0() == other.getFixed0() &&
		getFixed1() == other.getFixed1() &&
		getFixed2() == other.getFixed2() &&
		getShelf() == other.getShelf() &&
		getInvert() == other.getInvert();
}
