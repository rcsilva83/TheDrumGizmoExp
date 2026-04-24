/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            enginefactorytest.cc
 *
 *  Fri Apr 24 10:31:01 CET 2026
 *  Copyright 2026 DrumGizmo team
 *
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

#include "enginefactory.h"

TEST_CASE("EngineFactory")
{
	EngineFactory factory;

	SUBCASE("getInputEnginesIsNotEmpty")
	{
		CHECK_UNARY(!factory.getInputEngines().empty());
	}

	SUBCASE("getOutputEnginesIsNotEmpty")
	{
		CHECK_UNARY(!factory.getOutputEngines().empty());
	}

	SUBCASE("createDummyInputReturnsValidEngine")
	{
		for(const auto& name : factory.getInputEngines())
		{
			if(name == "dummy")
			{
				auto engine = factory.createInput("dummy");
				CHECK_UNARY(engine != nullptr);
				return;
			}
		}
	}

	SUBCASE("createTestInputReturnsValidEngine")
	{
		for(const auto& name : factory.getInputEngines())
		{
			if(name == "test")
			{
				auto engine = factory.createInput("test");
				CHECK_UNARY(engine != nullptr);
				return;
			}
		}
	}

	SUBCASE("createInvalidInputReturnsNull")
	{
		auto engine = factory.createInput("nonexistent-engine");
		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createDummyOutputReturnsValidEngine")
	{
		for(const auto& name : factory.getOutputEngines())
		{
			if(name == "dummy")
			{
				auto engine = factory.createOutput("dummy");
				CHECK_UNARY(engine != nullptr);
				return;
			}
		}
	}

	SUBCASE("createInvalidOutputReturnsNull")
	{
		auto engine = factory.createOutput("nonexistent-engine");
		CHECK_UNARY(engine == nullptr);
	}
}
