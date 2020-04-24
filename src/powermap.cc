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

#include <cassert>
#include <cmath>

namespace
{

using Power = Powermap::Power;
using PowerPair = Powermap::PowerPair;

Power h00(Power x) { return (1+2*x)*pow(1-x,2); }
Power h10(Power x) { return x*pow(1-x,2); }
Power h01(Power x) { return x*x*(3-2*x); }
Power h11(Power x) { return x*x*(x-1); }

Power computeValue(
	Power const x, PowerPair const& P0, PowerPair const& P1, Power const m0, Power const m1)
{
	auto const x0 = P0.in;
	auto const x1 = P1.in;
	auto const y0 = P0.out;
	auto const y1 = P1.out;
	auto const dx = x1 - x0;
	auto const x_prime = (x - x0)/dx;

	return h00(x_prime)*y0 + h10(x_prime)*dx*m0 + h01(x_prime)*y1 + h11(x_prime)*dx*m1;
}

} // end anonymous namespace

Powermap::Powermap()
{
	reset();
}

Power Powermap::map(Power in)
{
	if (spline_needs_update) {
		updateSpline();
	}

	if (in < fixed[0].in) {
		return shelf ? fixed[0].out : computeValue(in, {0.,0.}, fixed[0], m[0], m[1]);
	}
	else if (in < fixed[1].in) {
		return computeValue(in, fixed[0], fixed[1], m[1], m[2]);
	}
	else if (in < fixed[2].in) {
		return computeValue(in, fixed[1], fixed[2], m[2], m[3]);
	}
	else { // in >= fixed[2].in
		return shelf ? fixed[2].out : computeValue(in, fixed[2], {1.,1.}, m[3], m[4]);
	}
}

void Powermap::reset()
{
	fixed[0] = {0., 0.};
	fixed[1] = {.5, .5};
	fixed[2] = {1., 1.};
	// FIXME: better false?
	shelf = true;

	updateSpline();
}

void Powermap::setFixed0(PowerPair new_value)
{
	if (fixed[0] != new_value) {
		spline_needs_update = true;
		this->fixed[0] = new_value;
	}
}

void Powermap::setFixed1(PowerPair new_value)
{
	if (fixed[1] != new_value) {
		spline_needs_update = true;
		this->fixed[1] = new_value;
	}
}

void Powermap::setFixed2(PowerPair new_value)
{
	if (fixed[2] != new_value) {
		spline_needs_update = true;
		this->fixed[2] = new_value;
	}
}

void Powermap::setShelf(bool enable)
{
	if (shelf != enable) {
		spline_needs_update = true;
		this->shelf = enable;
	}
}

PowerPair Powermap::getFixed0() const
{
	return fixed[0];
}

PowerPair Powermap::getFixed1() const
{
	return fixed[1];
}

PowerPair Powermap::getFixed2() const
{
	return fixed[2];
}

// This mostly followes the wikipedia article for monotone cubic splines:
// https://en.wikipedia.org/wiki/Monotone_cubic_interpolation
void Powermap::updateSpline()
{
	assert(0. <= fixed[0].in && fixed[0].in < fixed[1].in &&
	       fixed[1].in < fixed[2].in && fixed[2].in <= 1.);
	assert(0. <= fixed[0].out && fixed[0].out <= fixed[1].out &&
	       fixed[1].out <= fixed[2].out && fixed[2].out <= 1.);

	// TODO: What to do if fixed[0] is (0,0) or fixed[2] is (1,1)??
	Powers X = shelf ? Powers{fixed[0].in, fixed[1].in, fixed[2].in}
	                 : Powers{0., fixed[0].in, fixed[1].in, fixed[2].in, 1.};
	Powers P = shelf ? Powers{fixed[0].out, fixed[1].out, fixed[2].out}
	                 : Powers{0., fixed[0].out, fixed[1].out, fixed[2].out, 1.};

	Powers deltas(X.size()-1);
	Powers m(X.size());

	// 1
	for (std::size_t i = 0; i < deltas.size(); ++i) {
		deltas[i] = (P[i+1]-P[i])/(X[i+1]-X[i]);
	}

	// 2
	m[0] = deltas[0];
	for (std::size_t i = 1; i < deltas.size(); ++i) {
		m[i] =  (deltas[i-1] + deltas[i])/2.;
	}
	m.back() = deltas.back();

	// 3
	std::vector<bool> ignore(deltas.size(), false);
	for (std::size_t i = 0; i < deltas.size(); ++i) {
		if (deltas[i] == 0) {
			m[i] = 0;
			m[i+1] = 0;
			ignore[i] = true;
		}
	}

	for (std::size_t i = 0; i < deltas.size(); ++i) {
		if (ignore[i]) {
			continue;
		}

		// 4
		auto alpha = m[i]/deltas[i];
		auto beta = m[i+1]/deltas[i];
		assert(alpha >= 0.);
		assert(beta >= 0.);

		// 5
		// TODO: expose this parameter for testing both
		bool const option1 = true;
		if (option1) {
			if (alpha > 3 || beta > 3) {
				m[i] = 3*deltas[i];
			}
		}
		else {
			auto const a2b2 = alpha*alpha + beta*beta;
			if (a2b2 > 9) {
				auto const tau = 3./sqrt(a2b2);
				m[i] = tau*alpha*deltas[i];
				m[i+1] = tau*alpha*deltas[i];
			}
		}
	}

	if (shelf) {
		assert(m.size() == 3);
		this->m[1] = m[0];
		this->m[2] = m[1];
		this->m[3] = m[2];
	}
	else {
		assert(m.size() == 5);
		for (std::size_t i = 0; i < m.size(); ++i) {
			this->m[i] = m[i];
		}
	}

	spline_needs_update = false;
}
