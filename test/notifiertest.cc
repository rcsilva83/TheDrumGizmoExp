/* -*- Mode: c++ -*- */
/***************************************************************************
 *            notifiertest.cc
 *
 *  Sat Jul 15 09:56:51 CEST 2017
 *  Copyright 2017 Bent Bisballe Nyeng
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
#include <doctest/doctest.h>

#include <vector>

#include <notifier.h>

class Probe : public Listener
{
public:
	Probe(std::vector<Probe*>& triggers) : triggers(triggers)
	{
	}

	void slot()
	{
		triggers.push_back(this);
	}

	std::vector<Probe*>& triggers;
};

TEST_CASE("NotifierTest")
{
	SUBCASE("testTest")
	{
		Notifier<> notifier;
		std::vector<Probe*> triggers;
		Probe foo1(triggers);
		Probe foo2(triggers);

		{ // Order as initialisation
			notifier.connect(&foo1, &Probe::slot);
			notifier.connect(&foo2, &Probe::slot);
			notifier();
			std::vector<Probe*> ref;
			ref.push_back(&foo1);
			ref.push_back(&foo2);
			CHECK_EQ(ref.size(), triggers.size());
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[0], triggers[0]);
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[1], triggers[1]);
			notifier.disconnect(&foo1);
			notifier.disconnect(&foo2);
			triggers.clear();
		}

		{ // Reverse order
			notifier.connect(&foo2, &Probe::slot);
			notifier.connect(&foo1, &Probe::slot);
			notifier();
			std::vector<Probe*> ref;
			ref.push_back(&foo2);
			ref.push_back(&foo1);
			CHECK_EQ(ref.size(), triggers.size());
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[0], triggers[0]);
			// cppcheck-suppress containerOutOfBounds
			CHECK_EQ(ref[1], triggers[1]);
			notifier.disconnect(&foo1);
			notifier.disconnect(&foo2);
			triggers.clear();
		}
	}
}
