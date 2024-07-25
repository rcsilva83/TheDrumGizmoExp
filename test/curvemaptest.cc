/* -*- Mode: c++ -*- */
/***************************************************************************
 *            curvemaptest.cc
 *
 *  Sun Apr 19 23:23:37 CEST 2020
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
#include <uunit.h>
#include <map>

#include "../src/curvemap.h"
#include <iostream>
#include <iomanip>

class test_curvemaptest
	: public uUnit
{
public:
	test_curvemaptest()
	{
		uUNIT_TEST(test_curvemaptest::check_default);
		uUNIT_TEST(test_curvemaptest::check_default_invert);
		uUNIT_TEST(test_curvemaptest::check_default_disabled_shelf);
		uUNIT_TEST(test_curvemaptest::check_default_disabled_shelf_invert);
		uUNIT_TEST(test_curvemaptest::check_fixed1_075_025);
		uUNIT_TEST(test_curvemaptest::check_fixed1_075_025_invert);
		uUNIT_TEST(test_curvemaptest::check_shelf_060);
		uUNIT_TEST(test_curvemaptest::check_shelf_060_invert);
		// uUNIT_TEST(test_curvemaptest::check_fixed0_075_025_invert);
	}

	static constexpr auto eps = CurveMapTestAccessor::eps;

	const std::map<double, double> default_map = {
		{0.0, eps},
		{eps, eps},
		{0.1, 0.1},
		{0.2, 0.2},
		{0.3, 0.3},
		{0.4, 0.4},
		{0.5, 0.5},
		{0.6, 0.6},
		{0.7, 0.7},
		{0.8, 0.8},
		{0.9, 0.9},
		{1.0 - eps, 1.0 - eps},
		{1.0, 1.0 - eps}
	};

	const std::map<double, double> identity_map = {
		{0.0, 0.0},
		{0.1, 0.1},
		{0.2, 0.2},
		{0.3, 0.3},
		{0.4, 0.4},
		{0.5, 0.5},
		{0.6, 0.6},
		{0.7, 0.7},
		{0.8, 0.8},
		{0.9, 0.9},
		{1.0, 1.0}
	};

	const std::map<double, double> fixed1_075_025_map = {
		{0.0, eps},
		{eps, eps},
		{0, 0.0001},
		{0.1, 0.0295469705015421},
		{0.2, 0.0536915548145771},
		{0.3, 0.0760560110211372},
		{0.4, 0.100195862352848},
		{0.5, 0.129666686058044},
		{0.6, 0.168024003505707},
		{0.7, 0.218823373317719},
		{0.8, 0.325357049703598},
		{0.9, 0.64416378736496},
		{1.0 - eps, 1.0 - eps},
		{1.0, 1.0 - eps}
	};

	const std::map<double, double> shelf_060_map = {
		{0.0, eps},
		{eps, eps},
		{0.1, 0.1},
		{0.2, 0.2},
		{0.3, 0.3},
		{0.4, 0.4},
		{0.5, 0.5},
		{0.6, 0.6},
		{0.7, 0.6},
		{0.8, 0.6},
		{0.9, 0.6},
		{1.0, 0.6}
	};

	void check_default()
	{
		auto dataset = this->default_map;

		CurveMap map;

		for (auto& entry : dataset) {
			auto in = entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_default_invert()
	{
		auto dataset = this->default_map;

		CurveMap map;
		map.setInvert(true);
		
		for (auto& entry : dataset) {
			auto in = 1.0 - entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_default_disabled_shelf()
	{
		auto dataset = this->identity_map;

		CurveMap map;
		map.setShelf(false);
		map.setFixed2({0.6, 0.6});

		for (auto& entry : dataset) {
			auto in = entry.first;
			auto expect = entry.second;
			// std::cout << "{" << in << ", " << std::setprecision (15) << map.map(in) << "}," << std::endl; // FIXME
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_default_disabled_shelf_invert()
	{
		auto dataset = this->identity_map;

		CurveMap map;
		map.setShelf(false);
		map.setFixed2({0.6, 0.6});
		map.setInvert(true);

		for (auto& entry : dataset) {
			auto in = 1.0 - entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_fixed1_075_025()
	{
		auto dataset = this->fixed1_075_025_map;

		CurveMap map;
		map.setFixed1({0.75, 0.25});

		for (auto& entry : dataset) {
			auto in = entry.first;
			auto expect = entry.second;
			// std::cout << "{" << in << ", " << std::setprecision (15) << map.map(in) << "}," << std::endl; // FIXME
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_fixed1_075_025_invert()
	{
		auto dataset = this->fixed1_075_025_map;

		CurveMap map;
		map.setFixed1({0.75, 0.25});
		map.setInvert(true);

		for (auto& entry : dataset) {
			auto in = 1.0 - entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_shelf_060()
	{
		auto dataset = this->shelf_060_map;

		CurveMap map;
		map.setFixed2({0.6, 0.6});
		map.setShelf(true);

		for (auto& entry : dataset) {
			auto in = entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}

	void check_shelf_060_invert()
	{
		auto dataset = this->shelf_060_map;

		CurveMap map;
		map.setFixed2({0.6, 0.6});
		map.setShelf(true);
		map.setInvert(true);

		for (auto& entry : dataset) {
			auto in = 1.0 - entry.first;
			auto expect = entry.second;
			uASSERT_EQUAL(expect, map.map(in));
		}
	}
};

// Registers the fixture into the 'registry'
static test_curvemaptest test;
