/* -*- Mode: c++ -*- */
/***************************************************************************
 *            position_power_test.cc
 *
 *  Wed Jul 24 15:24:53 CEST 2024
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
#include <uunit.h>

#include "../src/position_power.h"

class test_position_powertest
	: public uUnit
{
public:
	test_position_powertest()
	{
		uTEST(test_position_powertest::empty);
		uTEST(test_position_powertest::boundary);
	}

	void empty()
	{
		std::vector<Sample*> samplelist;
		auto res = positionPower(samplelist, 0);
		uASSERT_EQUAL(0.0, res.min);
		uASSERT_EQUAL(1.0, res.max);
	}

	void boundary()
	{
		// Sig:    nam, pwr, pos
		Sample s1_1{{}, 1.1, 1.0};
		Sample s2_1{{}, 2.1, 1.0};
		Sample s3_1{{}, 3.1, 1.0};
		Sample s4_1{{}, 4.1, 1.0};
		Sample s5_1{{}, 5.1, 1.0};

		Sample s1_2{{}, 1.2, 2.0};
		Sample s2_2{{}, 2.2, 2.0};
		Sample s3_2{{}, 3.2, 2.0};
		Sample s4_2{{}, 4.2, 2.0};
		Sample s5_2{{}, 5.2, 2.0};
		Sample s6_2{{}, 6.2, 2.0};
		Sample s7_2{{}, 7.2, 2.0};

		Sample s1_3{{}, 1.3, 3.0};
		Sample s2_3{{}, 2.3, 3.0};
		Sample s3_3{{}, 3.3, 3.0};
		Sample s4_3{{}, 4.3, 3.0};
		Sample s5_3{{}, 5.3, 3.0};
		Sample s6_3{{}, 6.3, 3.0};
		Sample s7_3{{}, 7.3, 3.0};

		{ // one [s1_1, s1_1]
			std::vector<Sample*> samplelist{&s1_1};
			auto res = positionPower(samplelist, 1.0);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(1.1, res.max);
		}

		{ // two with same position [s1_1, s2_1]
			std::vector<Sample*> samplelist{&s1_1, &s2_1};
			auto res = positionPower(samplelist, 1.0);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(2.1, res.max);
		}

		{ // two with different position [s1_1, s2_2]
			std::vector<Sample*> samplelist{&s1_1, &s2_2};
			auto res = positionPower(samplelist, 1.0);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(1.1, res.max);
		}

		{ // three [s1, (s2), s3] - one "hidden" inside range
			std::vector<Sample*> samplelist{&s1_1, &s2_1, &s3_1};
			auto res = positionPower(samplelist, 1.0);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(3.1, res.max);
		}

		{ // six [s1, (s2), s3] - one "hidden" inside range and three ouside boundary
			std::vector<Sample*> samplelist{&s1_1, &s2_1, &s3_1, &s1_2, &s2_2, &s3_2};
			auto res = positionPower(samplelist, 1.0);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(3.1, res.max);
		}

		{ // six [s1, (s2), s3] - one "hidden" inside range and three ouside boundary
			std::vector<Sample*> samplelist{&s1_1, &s2_1, &s3_1, &s1_2, &s2_2, &s3_2};
			auto res = positionPower(samplelist, 2.0);
			uASSERT_EQUAL(1.2, res.min);
			uASSERT_EQUAL(3.2, res.max);
		}

		{ // again, six in two position groups (1 and 2), lower three is the closest to 1.49
			std::vector<Sample*> samplelist{&s1_1, &s2_1, &s3_1, &s1_2, &s2_2, &s3_2};
			auto res = positionPower(samplelist, 1.49);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(3.1, res.max);
		}

		{ // again, six in two position groups (1 and 2), upper three is the closest to 1.51
			std::vector<Sample*> samplelist{&s1_1, &s2_1, &s3_1, &s1_2, &s2_2, &s3_2};
			auto res = positionPower(samplelist, 1.51);
			uASSERT_EQUAL(1.2, res.min);
			uASSERT_EQUAL(3.2, res.max);
		}

		{ // 8, first one at position the remaining at other position
			// 1/4th of the samples are 2, and the second one belongs to group 2, which
			// will drag in the rest of group 2 with it
			std::vector<Sample*> samplelist{&s1_1, &s1_2, &s2_2, &s3_2,
			                                &s4_2, &s5_2, &s6_2, &s7_2};
			auto res = positionPower(samplelist, 1);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(7.2, res.max);
		}

		{ // 9, first one at position the remaining at other position + one from group 3
			// at the end which is ignored
			// 1/4th of the samples are 2, and the second one belongs to group 2, which
			// will drag in the rest of group 2 with it
			std::vector<Sample*> samplelist{&s1_1, &s1_2, &s2_2, &s3_2,
			                                &s4_2, &s5_2, &s6_2, &s7_2, &s7_3};
			auto res = positionPower(samplelist, 1);
			uASSERT_EQUAL(1.1, res.min);
			uASSERT_EQUAL(7.2, res.max);
		}

		{ // 8, first one from group 1, then 6 from group 2 and finally one from group 3
			// first and last should be ignored - input pos is closest to group 2
			std::vector<Sample*> samplelist{&s1_1,
			                                &s1_2, &s2_2, &s3_2, &s4_2,
			                                &s7_3};
			auto res = positionPower(samplelist, 2.1);
			uASSERT_EQUAL(1.2, res.min);
			uASSERT_EQUAL(4.2, res.max);
		}

	}
};

// Registers the fixture into the 'registry'
static test_position_powertest test;
