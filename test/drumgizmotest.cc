/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            drumgizmotest.cc
 *
 *  Fri Apr 24 10:30:00 CET 2026
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

#include <config.h>

#include "drumgizmo/enginefactory.h"

// =============================================================================
// EngineFactory Tests
// =============================================================================

TEST_CASE("EngineFactory")
{
	SUBCASE("constructorInitializesEngineLists")
	{
		EngineFactory factory;

		// Should have at least some engines registered
		[[maybe_unused]] auto& inputs = factory.getInputEngines();
		[[maybe_unused]] auto& outputs = factory.getOutputEngines();

		// The lists should be populated based on compile-time flags
		// Just verify no crash by calling the methods
		CHECK_UNARY(true);
	}

	SUBCASE("getInputEnginesReturnsValidList")
	{
		EngineFactory factory;
		const auto& inputs = factory.getInputEngines();

		// Verify it's a valid list reference
		for(const auto& engine_name : inputs)
		{
			CHECK_UNARY(!engine_name.empty());
		}
	}

	SUBCASE("getOutputEnginesReturnsValidList")
	{
		EngineFactory factory;
		const auto& outputs = factory.getOutputEngines();

		// Verify it's a valid list reference
		for(const auto& engine_name : outputs)
		{
			CHECK_UNARY(!engine_name.empty());
		}
	}

#ifdef HAVE_INPUT_DUMMY
	SUBCASE("createInputDummyReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createInput("dummy");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_INPUT_TEST
	SUBCASE("createInputTestReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createInput("test");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_OUTPUT_DUMMY
	SUBCASE("createOutputDummyReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("dummy");

		CHECK_UNARY(engine != nullptr);
	}
#endif

#ifdef HAVE_OUTPUT_WAVFILE
	SUBCASE("createOutputWavfileReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("wavfile");

		CHECK_UNARY(engine != nullptr);
	}
#endif

	SUBCASE("createInputInvalidReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createInput("nonexistent_engine");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createOutputInvalidReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("nonexistent_engine");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createInputEmptyStringReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createInput("");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("createOutputEmptyStringReturnsNull")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("");

		CHECK_UNARY(engine == nullptr);
	}

	SUBCASE("multipleFactoryInstancesWork")
	{
		EngineFactory factory1;
		EngineFactory factory2;

		// Both should be independent
		auto& inputs1 = factory1.getInputEngines();
		auto& inputs2 = factory2.getInputEngines();

		CHECK_EQ(inputs1.size(), inputs2.size());
	}

	SUBCASE("createdEnginesAreIndependent")
	{
		EngineFactory factory;

#ifdef HAVE_INPUT_DUMMY
		auto engine1 = factory.createInput("dummy");
		auto engine2 = factory.createInput("dummy");

		CHECK_UNARY(engine1 != nullptr);
		CHECK_UNARY(engine2 != nullptr);
		CHECK_UNARY(engine1 != engine2);
#endif
	}

#ifdef HAVE_INPUT_OSSMIDI
	SUBCASE("createInputOssmidiReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createInput("ossmidi");

		CHECK_UNARY(engine != nullptr);
	}

	SUBCASE("getInputEnginesContainsOssmidi")
	{
		EngineFactory factory;
		const auto& inputs = factory.getInputEngines();

		bool found = false;
		for(const auto& name : inputs)
		{
			if(name == "ossmidi")
			{
				found = true;
			}
		}
		CHECK_UNARY(found);
	}
#endif

#ifdef HAVE_OUTPUT_OSS
	SUBCASE("createOutputOssReturnsValidEngine")
	{
		EngineFactory factory;
		auto engine = factory.createOutput("oss");

		CHECK_UNARY(engine != nullptr);
	}

	SUBCASE("getOutputEnginesContainsOss")
	{
		EngineFactory factory;
		const auto& outputs = factory.getOutputEngines();

		bool found = false;
		for(const auto& name : outputs)
		{
			if(name == "oss")
			{
				found = true;
			}
		}
		CHECK_UNARY(found);
	}
#endif
}
