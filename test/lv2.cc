/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            lv2.cc
 *
 *  Thu Feb 12 14:55:41 CET 2015
 *  Copyright 2015 Bent Bisballe Nyeng
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

#include <config.h>

#include <arpa/inet.h>
#include <chrono>
#include <memory.h>
#include <stdio.h>
#include <thread>

#include "drumkit_creator.h"
#include "lv2_test_host.h"

#define DG_URI "http://drumgizmo.org/lv2"

static const char* b2s(bool b)
{
	return b ? "true" : "false";
}

enum class Ports
{
	FreeWheel = 0,
	Latency,
	MidiPort,
	AudioPortOffset,
};

/**
 * Tests that should be performed:
 * -------------------------------
 * - Run without port connects (shouldn't crash)
 * - Run without output ports connects (shouldn't crash)
 * - Run with buffer size 0
 * - Run with VERY LARGE buffer size (>1MB?)
 * - Run with buffer size a prime number (and thereby not power of 2)
 * - Run with HUGE number of midi events in one buffer (10000)
 */

struct LV2Fixture
{
	DrumkitCreator drumkit_creator;
};

TEST_CASE("open_and_verify")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

TEST_CASE("lifecycle_invalid_transitions")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_UNARY(res != 0);

	res = h.activate();
	CHECK_UNARY(res != 0);

	res = h.deactivate();
	CHECK_UNARY(res != 0);

	res = h.connectPort((int)Ports::FreeWheel, nullptr);
	CHECK_UNARY(res != 0);

	res = h.run(1);
	CHECK_UNARY(res != 0);

	res = h.loadConfig(nullptr, 1);
	CHECK_UNARY(res != 0);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_UNARY(res != 0);

	res = h.connectPort(-1, nullptr);
	CHECK_UNARY(res != 0);

	res = h.connectPort(9999, nullptr);
	CHECK_UNARY(res != 0);

	res = h.run(-1);
	CHECK_UNARY(res != 0);

	res = h.deactivate();
	CHECK_UNARY(res != 0);

	res = h.activate();
	CHECK_EQ(0, res);

	res = h.activate();
	CHECK_UNARY(res != 0);

	res = h.deactivate();
	CHECK_EQ(0, res);

	res = h.deactivate();
	CHECK_UNARY(res != 0);

	res = h.close();
	CHECK_UNARY(res != 0);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_UNARY(res != 0);

	res = h.close();
	CHECK_EQ(0, res);
}

TEST_CASE("lifecycle_teardown_requires_destroy")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_UNARY(res != 0);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

TEST_CASE_FIXTURE(LV2Fixture, "run_no_ports_connected")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	const char config_fmt[] =
	    "<config>\n"
	    "  <value name=\"drumkitfile\">%s</value>\n"
	    "  <value name=\"midimapfile\">%s</value>\n"
	    "  <value name=\"enable_velocity_modifier\">%s</value>\n"
	    "  <value name=\"velocity_modifier_falloff\">%f</value>\n"
	    "  <value name=\"velocity_modifier_weight\">%f</value>\n"
	    "  <value name=\"enable_velocity_randomiser\">%s</value>\n"
	    "  <value name=\"velocity_randomiser_weight\">%f</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"disk_cache_upper_limit\">%d</value>\n"
	    "  <value name=\"disk_cache_chunk_size\">%d</value>\n"
	    "  <value name=\"disk_cache_enable\">%s</value>\n"
	    "  <value name=\"enable_bleed_control\">%s</value>\n"
	    "  <value name=\"master_bleed\">%f</value>\n"
	    "  <value name=\"enable_latency_modifier\">%s</value>\n"
	    "  <value name=\"latency_max\">%d</value>\n"
	    "  <value name=\"latency_laid_back\">%d</value>\n"
	    "  <value name=\"latency_stddev\">%f</value>\n"
	    "  <value name=\"latency_regain\">%f</value>\n"
	    "</config>";

	// Create drumkit
	auto kit1_file = drumkit_creator.createStdKit("kit1");

	auto midimapfile = drumkit_creator.createStdMidimap("midimap");
	bool enable_velocity_modifier = true;
	float velocity_modifier_falloff = 0.5;
	float velocity_modifier_weight = 0.25;
	bool enable_velocity_randomiser = false;
	float velocity_randomiser_weight = 0.1;
	bool enable_resampling = false;
	int disk_cache_upper_limit = 1024 * 1024;
	int disk_cache_chunk_size = 1024 * 1024 * 1024;
	bool disk_cache_enable = true;
	bool enable_bleed_control = false;
	float master_bleed = 1.0f;
	bool enable_latency_modifier = false;
	int latency_max = 0u;
	int latency_laid_back = 0u;
	float latency_stddev = 100.0f;
	float latency_regain = 0.9f;

	char config[sizeof(config_fmt) * 2];
	sprintf(config, config_fmt, kit1_file.c_str(), midimapfile.c_str(),
	    b2s(enable_velocity_modifier), velocity_modifier_falloff,
	    velocity_modifier_weight, b2s(enable_velocity_randomiser),
	    velocity_randomiser_weight, b2s(enable_resampling),
	    b2s(enable_resampling), disk_cache_upper_limit, disk_cache_chunk_size,
	    b2s(disk_cache_enable), b2s(enable_bleed_control), master_bleed,
	    b2s(enable_latency_modifier), latency_max, latency_laid_back,
	    latency_stddev, latency_regain);

	res = h.loadConfig(config, strlen(config));
	CHECK_EQ(0, res);

	// run for 1 samples to trigger kit loading
	res = h.run(1);
	CHECK_EQ(0, res);
	std::this_thread::sleep_for(
	    std::chrono::milliseconds(1)); // wait for kit to get loaded (async),

	res = h.run(100);
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

TEST_CASE_FIXTURE(LV2Fixture, "run_no_output_ports_connected")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	const char config_fmt[] =
	    "<config>\n"
	    "  <value name=\"drumkitfile\">%s</value>\n"
	    "  <value name=\"midimapfile\">%s</value>\n"
	    "  <value name=\"enable_velocity_modifier\">%s</value>\n"
	    "  <value name=\"velocity_modifier_falloff\">%f</value>\n"
	    "  <value name=\"velocity_modifier_weight\">%f</value>\n"
	    "  <value name=\"enable_velocity_randomiser\">%s</value>\n"
	    "  <value name=\"velocity_randomiser_weight\">%f</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"disk_cache_upper_limit\">%d</value>\n"
	    "  <value name=\"disk_cache_chunk_size\">%d</value>\n"
	    "  <value name=\"disk_cache_enable\">%s</value>\n"
	    "  <value name=\"enable_bleed_control\">%s</value>\n"
	    "  <value name=\"master_bleed\">%f</value>\n"
	    "  <value name=\"enable_latency_modifier\">%s</value>\n"
	    "  <value name=\"latency_max\">%d</value>\n"
	    "  <value name=\"latency_laid_back\">%d</value>\n"
	    "  <value name=\"latency_stddev\">%f</value>\n"
	    "  <value name=\"latency_regain\">%f</value>\n"
	    "</config>";

	// Create drumkit
	auto kit1_file = drumkit_creator.createStdKit("kit1");

	auto midimapfile = drumkit_creator.createStdMidimap("midimap");
	bool enable_velocity_modifier = true;
	float velocity_modifier_falloff = 0.5;
	float velocity_modifier_weight = 0.25;
	bool enable_velocity_randomiser = false;
	float velocity_randomiser_weight = 0.1;
	bool enable_resampling = false;
	int disk_cache_upper_limit = 1024 * 1024;
	int disk_cache_chunk_size = 1024 * 1024 * 1024;
	bool disk_cache_enable = true;
	bool enable_bleed_control = false;
	float master_bleed = 1.0f;
	bool enable_latency_modifier = false;
	int latency_max = 0u;
	int latency_laid_back = 0u;
	float latency_stddev = 100.0f;
	float latency_regain = 0.9f;

	char config[sizeof(config_fmt) * 2];
	sprintf(config, config_fmt, kit1_file.c_str(), midimapfile.c_str(),
	    b2s(enable_velocity_modifier), velocity_modifier_falloff,
	    velocity_modifier_weight, b2s(enable_velocity_randomiser),
	    velocity_randomiser_weight, b2s(enable_resampling),
	    b2s(enable_resampling), disk_cache_upper_limit, disk_cache_chunk_size,
	    b2s(disk_cache_enable), b2s(enable_bleed_control), master_bleed,
	    b2s(enable_latency_modifier), latency_max, latency_laid_back,
	    latency_stddev, latency_regain);

	res = h.loadConfig(config, strlen(config));
	CHECK_EQ(0, res);

	// Port buffers:
	char sequence_buffer[4096];
	bool freeWheel = false;

	// Free wheel port
	res = h.connectPort((int)Ports::FreeWheel, static_cast<void*>(&freeWheel));
	CHECK_EQ(0, res);

	LV2TestHost::Sequence seq(sequence_buffer, sizeof(sequence_buffer));
	res = h.connectPort((int)Ports::MidiPort, seq.data());
	CHECK_EQ(0, res);

	// run for 1 samples to trigger kit loading
	res = h.run(1);
	CHECK_EQ(0, res);
	std::this_thread::sleep_for(
	    std::chrono::milliseconds(1)); // wait for kit to get loaded (async),

	seq.addMidiNote(5, 1, 127);
	res = h.run(100);
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

/*
 * Test state save (exercises ConfigStringIO::get() and all bool2str/float2str/
 * int2str helper functions).
 */
TEST_CASE_FIXTURE(LV2Fixture, "state_save")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	res = h.saveConfig();
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

/*
 * Test state restore with a partial config that omits drumkitfile and
 * midimapfile and sets only a subset of the other options. This exercises
 * both the "value present" (true) branches for the listed settings and the
 * "value not present" (false) branches of the corresponding
 * if(p.value("xxx") != "") checks in ConfigStringIO::set().
 */
TEST_CASE_FIXTURE(LV2Fixture, "state_restore_partial_config")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	// Provide only a subset of settings (no drumkitfile / midimapfile).
	// This exercises:
	//   - The FALSE branch of every if(p.value("xxx") != "") for the
	//     settings that are absent (all the ones not listed here).
	//   - The TRUE branch for the settings that ARE listed.
	//   - The FALSE branch of the drumkitfile / midimapfile guards at the
	//     bottom of ConfigStringIO::set() (lines 848 and 854).
	const char partial_config[] =
	    "<config version=\"1.0\">\n"
	    "  <value name=\"enable_velocity_modifier\">false</value>\n"
	    "  <value name=\"velocity_modifier_falloff\">0.5</value>\n"
	    "  <value name=\"velocity_modifier_weight\">0.25</value>\n"
	    "  <value name=\"velocity_stddev\">0.1</value>\n"
	    "  <value name=\"sample_selection_f_close\">0.3</value>\n"
	    "  <value name=\"sample_selection_f_diverse\">0.4</value>\n"
	    "  <value name=\"sample_selection_f_random\">0.3</value>\n"
	    "  <value name=\"enable_velocity_randomiser\">false</value>\n"
	    "  <value name=\"velocity_randomiser_weight\">0.1</value>\n"
	    "  <value name=\"enable_resampling\">true</value>\n"
	    "  <value name=\"resampling_quality\">1.0</value>\n"
	    "  <value name=\"disk_cache_upper_limit\">1048576</value>\n"
	    "  <value name=\"disk_cache_chunk_size\">1073741824</value>\n"
	    "  <value name=\"disk_cache_enable\">true</value>\n"
	    "  <value name=\"enable_bleed_control\">false</value>\n"
	    "  <value name=\"master_bleed\">1.0</value>\n"
	    "  <value name=\"enable_latency_modifier\">false</value>\n"
	    "  <value name=\"latency_laid_back_ms\">0.0</value>\n"
	    "  <value name=\"latency_stddev\">100.0</value>\n"
	    "  <value name=\"latency_regain\">0.9</value>\n"
	    "  <value name=\"enable_powermap\">false</value>\n"
	    "  <value name=\"powermap_fixed0_x\">0.0</value>\n"
	    "  <value name=\"powermap_fixed0_y\">0.0</value>\n"
	    "  <value name=\"powermap_fixed1_x\">0.5</value>\n"
	    "  <value name=\"powermap_fixed1_y\">0.5</value>\n"
	    "  <value name=\"powermap_fixed2_x\">1.0</value>\n"
	    "  <value name=\"powermap_fixed2_y\">1.0</value>\n"
	    "  <value name=\"powermap_shelf\">false</value>\n"
	    "  <value name=\"enable_voice_limit\">false</value>\n"
	    "  <value name=\"voice_limit_max\">64</value>\n"
	    "  <value name=\"voice_limit_rampdown\">0.5</value>\n"
	    "</config>";

	res = h.loadConfig(partial_config, strlen(partial_config));
	CHECK_EQ(0, res);

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

/*
 * Test state restore with invalid XML. This exercises the parse-error branch
 * (p.parseString() returns false) in ConfigStringIO::set().
 */
TEST_CASE_FIXTURE(LV2Fixture, "state_restore_invalid_config")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	const char invalid_config[] = "<<not valid xml>>";

	res = h.loadConfig(invalid_config, strlen(invalid_config));
	CHECK_EQ(
	    0, res); // loadConfig itself succeeds; the plugin handles the error

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

/*
 * Test the inline display rendering across multiple LoadStatus values.
 * Exercises all major branches in DrumGizmoPlugin::onInlineRedraw():
 *   - show_bar / show_image visibility conditions (varied max_height)
 *   - context_needs_update (first call vs. same-dimension repeated call)
 *   - something_needs_update true and false paths
 *   - LoadStatus::Idle/Loading/Parsing (blue bar) and Done (green bar)
 *   - inline_image_first_draw path for the image block
 *   - pixel-format conversion loop
 *
 * If the inline display extension is not supported by the installed plugingizmo
 * version, renderInlineDisplay() returns 2 and the display-specific checks are
 * skipped gracefully.
 */
TEST_CASE_FIXTURE(LV2Fixture, "inline_display_coverage")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	// Phase 1: render before any kit is loaded (LoadStatus::Idle,
	// number_of_files == 0).  Use max_height == 0 so show_bar and show_image
	// are both false, avoiding the division-by-zero in the progress
	// calculation that would occur when number_of_files == 0.
	// This exercises: context_needs_update = true (first call, data is null),
	// something_needs_update = true, show_bar = false, show_image = false.
	res = h.renderInlineDisplay(100, 0);
	bool inline_display_supported = (res == 0);
	// Extension either succeeds (0) or is not supported (2).
	CHECK_UNARY(res == 0 || res == 2);

	if(inline_display_supported)
	{
		// Phase 2: same dimensions again — context.data is now set, dimensions
		// unchanged, no settings have changed → context_needs_update = false,
		// something_needs_update = false.  The update block is skipped.
		res = h.renderInlineDisplay(100, 0);
		CHECK_EQ(0, res);
	}

	// Load a valid kit to get the plugin into an active loading state.
	auto kit1_file = drumkit_creator.createStdKit("idkit");
	auto midimap_file = drumkit_creator.createStdMidimap("idmidimap");

	const char config_fmt[] = "<config version=\"1.0\">\n"
	                          "  <value name=\"drumkitfile\">%s</value>\n"
	                          "  <value name=\"midimapfile\">%s</value>\n"
	                          "</config>";

	char config[4096];
	const int config_len = snprintf(config, sizeof(config), config_fmt,
	    kit1_file.c_str(), midimap_file.c_str());
	REQUIRE(config_len >= 0);
	REQUIRE(static_cast<size_t>(config_len) < sizeof(config));

	res = h.loadConfig(config, strlen(config));
	CHECK_EQ(0, res);

	// Trigger the async kit-loading machinery.
	res = h.run(1);
	CHECK_EQ(0, res);

	// Wait conservatively for the asynchronous load to complete before
	// rendering the small-height inline display. This avoids racing the
	// loader thread while the total file count may still be zero, which can
	// otherwise make progress become 0/0 and trigger undefined behavior when
	// the bar width is converted to int.
	std::this_thread::sleep_for(std::chrono::milliseconds(1200));

	if(inline_display_supported)
	{
		// Phase 3: bar_height == 11 px (from the progress TexturedBox dy1=11).
		// With max_height == 11 the bar fits but the image does not.
		// show_bar = true, show_image = false.
		// This render is delayed until async loading has settled so progress
		// calculation is deterministic and finite.
		res = h.renderInlineDisplay(100, 11);
		CHECK_EQ(0, res);

		// Phase 4: large max_height makes both bar and image visible.
		// show_bar = true, show_image = true.
		// inline_image_first_draw path exercised.
		res = h.renderInlineDisplay(100, 100000);
		CHECK_EQ(0, res);

		// Phase 5: same dimensions again.
		// context_needs_update = false; inline_image_first_draw is now false.
		// something_needs_update may be false (no changes since last render).
		res = h.renderInlineDisplay(100, 100000);
		CHECK_EQ(0, res);

		// Phase 6: different width — forces context.width != width, so
		// context_needs_update = true again.
		res = h.renderInlineDisplay(200, 100000);
		CHECK_EQ(0, res);
	}

	// Wait for the kit to finish loading (LoadStatus::Done).
	std::this_thread::sleep_for(std::chrono::seconds(1));

	if(inline_display_supported)
	{
		// Phase 7: LoadStatus::Done → green-bar switch case.
		res = h.renderInlineDisplay(100, 100000);
		CHECK_EQ(0, res);
	}

	// Phase 8: Trigger a reload of the same kit so that the plugin transitions
	// back to Loading/Parsing/Idle.  Run once to kick off the async loader,
	// then render IMMEDIATELY (before the loader thread finishes) so that we
	// exercise the LoadStatus::Idle/Loading/Parsing → blue bar switch arm.
	res = h.loadConfig(config, strlen(config));
	CHECK_EQ(0, res);
	res = h.run(1);
	CHECK_EQ(0, res);

	if(inline_display_supported)
	{
		// Render right away: LoadStatus is very likely still Idle/Loading here.
		// The switch dispatches to case LoadStatus::Parsing/Loading/Idle.
		res = h.renderInlineDisplay(100, 100000);
		CHECK_EQ(0, res);
	}

	// Let the second load finish cleanly before destroying the instance.
	std::this_thread::sleep_for(std::chrono::seconds(1));

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}

TEST_CASE_FIXTURE(LV2Fixture, "test1")
{
	int res;

	LV2TestHost h(LV2_PATH);

	res = h.open(DG_URI);
	CHECK_EQ(0, res);

	res = h.verify();
	CHECK_EQ(0, res);

	res = h.createInstance(44100);
	CHECK_EQ(0, res);

	const char config_fmt[] =
	    "<config>\n"
	    "  <value name=\"drumkitfile\">%s</value>\n"
	    "  <value name=\"midimapfile\">%s</value>\n"
	    "  <value name=\"enable_velocity_modifier\">%s</value>\n"
	    "  <value name=\"velocity_modifier_falloff\">%f</value>\n"
	    "  <value name=\"velocity_modifier_weight\">%f</value>\n"
	    "  <value name=\"enable_velocity_randomiser\">%s</value>\n"
	    "  <value name=\"velocity_randomiser_weight\">%f</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"enable_resampling\">%s</value>\n"
	    "  <value name=\"disk_cache_upper_limit\">%d</value>\n"
	    "  <value name=\"disk_cache_chunk_size\">%d</value>\n"
	    "  <value name=\"disk_cache_enable\">%s</value>\n"
	    "  <value name=\"enable_bleed_control\">%s</value>\n"
	    "  <value name=\"master_bleed\">%f</value>\n"
	    "  <value name=\"enable_latency_modifier\">%s</value>\n"
	    "  <value name=\"latency_max\">%d</value>\n"
	    "  <value name=\"latency_laid_back\">%d</value>\n"
	    "  <value name=\"latency_stddev\">%f</value>\n"
	    "  <value name=\"latency_regain\">%f</value>\n"
	    "</config>";

	// Create drumkit
	auto kit1_file = drumkit_creator.createStdKit("kit1");

	auto midimapfile = drumkit_creator.createStdMidimap("midimap");
	bool enable_velocity_modifier = true;
	float velocity_modifier_falloff = 0.5;
	float velocity_modifier_weight = 0.25;
	bool enable_velocity_randomiser = false;
	float velocity_randomiser_weight = 0.1;
	bool enable_resampling = false;
	int disk_cache_upper_limit = 1024 * 1024;
	int disk_cache_chunk_size = 1024 * 1024 * 1024;
	bool disk_cache_enable = true;
	bool enable_bleed_control = false;
	float master_bleed = 1.0f;
	bool enable_latency_modifier = false;
	int latency_max = 0u;
	int latency_laid_back = 0u;
	float latency_stddev = 100.0f;
	float latency_regain = 0.9f;

	char config[sizeof(config_fmt) * 2];
	sprintf(config, config_fmt, kit1_file.c_str(), midimapfile.c_str(),
	    b2s(enable_velocity_modifier), velocity_modifier_falloff,
	    velocity_modifier_weight, b2s(enable_velocity_randomiser),
	    velocity_randomiser_weight, b2s(enable_resampling),
	    b2s(enable_resampling), disk_cache_upper_limit, disk_cache_chunk_size,
	    b2s(disk_cache_enable), b2s(enable_bleed_control), master_bleed,
	    b2s(enable_latency_modifier), latency_max, latency_laid_back,
	    latency_stddev, latency_regain);

	res = h.loadConfig(config, strlen(config));
	CHECK_EQ(0, res);

	// Port buffers:
	char sequence_buffer[4096];
	float pcm_buffer[NUM_CHANNELS][10];
	bool freeWheel = true;

	// Free wheel port
	res = h.connectPort((int)Ports::FreeWheel, static_cast<void*>(&freeWheel));
	CHECK_EQ(0, res);

	LV2TestHost::Sequence seq(sequence_buffer, sizeof(sequence_buffer));
	res = h.connectPort((int)Ports::MidiPort, seq.data());
	CHECK_EQ(0, res);

	for(int i = 0; i < NUM_CHANNELS; ++i)
	{
		for(int j = 0; j < 10; ++j)
		{
			pcm_buffer[i][j] = 0.42;
		}
		res += h.connectPort((int)Ports::AudioPortOffset + i, pcm_buffer[i]);
	}
	CHECK_EQ(0, res);

	// run for 1 samples to trigger kit loading
	res = h.run(1);
	CHECK_EQ(0, res);
	std::this_thread::sleep_for(
	    std::chrono::seconds(1)); // wait for kit to get loaded (async),

	seq.addMidiNote(5, 1, 127);
	for(int i = 0; i < 10; i++)
	{
		res = h.run(10);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		CHECK_EQ(0, res);

		// printf("Iteration:\n");
		// for(int k = 0; k < 16; k++) {
		//	printf("#%d ", k);
		//	for(int j = 0; j < 10; j++) printf("[%f]", pcm_buffer[k][j]);
		//	printf("\n");
		// }
		// printf("\n");

		seq.clear();
	}

	seq.addMidiNote(5, 1, 127);
	res = h.run(10);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	CHECK_EQ(0, res);

	/*
	printf("Iteration:\n");
	for(int k = 0; k < 4; k++) {
	    printf("#%d ", k);
	    for(int j = 0; j < 10; j++) printf("[%f]", pcm_buffer[k][j]);
	    printf("\n");
	}
	printf("\n");
	*/

	union
	{
		float f;
		unsigned int u;
	} comp_val;

	comp_val.u = 1040744448; // floating point value 0.133301....

	for(int k = 0; k < 4; k++)
	{
		for(int j = 0; j < 10; j++)
		{
			CHECK_EQ(((j == 5) ? comp_val.f : 0), pcm_buffer[k][j]);
		}
	}
	seq.clear();

	res = h.destroyInstance();
	CHECK_EQ(0, res);

	res = h.close();
	CHECK_EQ(0, res);
}
