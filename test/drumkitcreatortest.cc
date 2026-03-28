/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumkitcreatortest.cc
 *
 *  Sat Jan 14 15:55:39 CET 2017
 *  Copyright 2017 André Nusser
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
#include <doctest/doctest.h>

#include <fstream>

#include "drumkit_creator.h"

struct DrumkitcreatorTestFixture
{
	DrumkitCreator drumkit_creator;
};

TEST_CASE_FIXTURE(DrumkitcreatorTestFixture, "DrumkitcreatorTest")
{
	SUBCASE("createStdKit_returns_valid_xml_path")
	{
		std::string kit_path = drumkit_creator.createStdKit("stdkit");

		// Result must be a non-empty path pointing to an XML file
		CHECK_UNARY(kit_path.find(".xml") != std::string::npos);

		// The drumkit file must actually exist on disk
		std::ifstream f(kit_path);
		CHECK_UNARY(f.good());
	}
}
