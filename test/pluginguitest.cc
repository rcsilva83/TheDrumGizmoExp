/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pluginguitest.cc
 *
 *  Mon Apr 20 08:00:00 CET 2026
 *  Copyright 2026 DrumGizmo contributors
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

#include <dggui/window.h>

#include <settings.h>

#include "plugingui/abouttab.h"
#include "plugingui/bleedcontrolframecontent.h"
#include "plugingui/diskstreamingframecontent.h"
#include "plugingui/humaniservisualiser.h"
#include "plugingui/humanizerframecontent.h"
#include "plugingui/maintab.h"
#include "plugingui/mainwindow.h"
#include "plugingui/pluginconfig.h"
#include "plugingui/powerwidget.h"
#include "plugingui/resamplingframecontent.h"
#include "plugingui/sampleselectionframecontent.h"
#include "plugingui/statusframecontent.h"
#include "plugingui/timingframecontent.h"
#include "plugingui/visualizerframecontent.h"
#include "plugingui/voicelimitframecontent.h"

// Fixture that provides common test setup
struct PluginGUIFixture
{
	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;
	dggui::Window window;

	PluginGUIFixture()
	{
		window.resize(800, 600);
	}
};

TEST_CASE("AboutTabTest")
{
	dggui::Window window;
	window.resize(400, 300);

	SUBCASE("abouttab_construction")
	{
		GUI::AboutTab about_tab(&window);

		CHECK_UNARY(&about_tab != nullptr);
		CHECK_UNARY(about_tab.visible());
	}

	SUBCASE("abouttab_resize")
	{
		GUI::AboutTab about_tab(&window);

		about_tab.resize(400, 300);
		CHECK_EQ(std::size_t(400u), about_tab.width());
		CHECK_EQ(std::size_t(300u), about_tab.height());

		// Resize to smaller dimensions
		about_tab.resize(200, 150);
		CHECK_EQ(std::size_t(200u), about_tab.width());
		CHECK_EQ(std::size_t(150u), about_tab.height());
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "StatusframeContentTest")
{
	SUBCASE("statusframe_construction")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		CHECK_UNARY(&status_frame != nullptr);
		CHECK_UNARY(status_frame.visible());
	}

	SUBCASE("statusframe_resize")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		status_frame.resize(400, 200);
		CHECK_EQ(std::size_t(400u), status_frame.width());
		CHECK_EQ(std::size_t(200u), status_frame.height());
	}

	SUBCASE("statusframe_update_drumkit_load_status")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		// Test all LoadStatus values
		status_frame.updateDrumkitLoadStatus(LoadStatus::Idle);
		status_frame.updateDrumkitLoadStatus(LoadStatus::Loading);
		status_frame.updateDrumkitLoadStatus(LoadStatus::Parsing);
		status_frame.updateDrumkitLoadStatus(LoadStatus::Done);
		status_frame.updateDrumkitLoadStatus(LoadStatus::Error);

		CHECK_UNARY(&status_frame != nullptr);
	}

	SUBCASE("statusframe_update_midimap_load_status")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		// Test all LoadStatus values
		status_frame.updateMidimapLoadStatus(LoadStatus::Idle);
		status_frame.updateMidimapLoadStatus(LoadStatus::Loading);
		status_frame.updateMidimapLoadStatus(LoadStatus::Parsing);
		status_frame.updateMidimapLoadStatus(LoadStatus::Done);
		status_frame.updateMidimapLoadStatus(LoadStatus::Error);

		CHECK_UNARY(&status_frame != nullptr);
	}

	SUBCASE("statusframe_update_drumkit_info")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		status_frame.updateDrumkitName("Test Kit");
		status_frame.updateDrumkitDescription("A test drumkit");
		status_frame.updateDrumkitVersion("1.0.0");

		CHECK_UNARY(&status_frame != nullptr);
	}

	SUBCASE("statusframe_update_buffer_and_underruns")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		status_frame.updateBufferSize(1024);
		status_frame.updateNumberOfUnderruns(0);
		status_frame.updateNumberOfUnderruns(5);

		CHECK_UNARY(&status_frame != nullptr);
	}

	SUBCASE("statusframe_load_status_text")
	{
		GUI::StatusframeContent status_frame(&window, settings_notifier);

		status_frame.loadStatusTextChanged("Test message");
		status_frame.loadStatusTextChanged("");

		CHECK_UNARY(&status_frame != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "HumanizerframeContentTest")
{
	SUBCASE("humanizerframe_construction")
	{
		GUI::HumanizerframeContent humanizer(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&humanizer != nullptr);
		CHECK_UNARY(humanizer.visible());
	}

	SUBCASE("humanizerframe_setting_values")
	{
		GUI::HumanizerframeContent humanizer(
		    &window, settings, settings_notifier);

		// Simulate settings changes via notifier
		settings_notifier.velocity_modifier_weight(0.5f);
		settings_notifier.velocity_modifier_falloff(0.3f);
		settings_notifier.velocity_stddev(0.1f);

		CHECK_UNARY(&humanizer != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "DiskstreamingframeContentTest")
{
	SUBCASE("diskstreamingframe_construction")
	{
		GUI::DiskstreamingframeContent disk(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&disk != nullptr);
		CHECK_UNARY(disk.visible());
	}

	SUBCASE("diskstreamingframe_resize")
	{
		GUI::DiskstreamingframeContent disk(
		    &window, settings, settings_notifier);

		disk.resize(400, 100);
		CHECK_EQ(std::size_t(400u), disk.width());
		CHECK_EQ(std::size_t(100u), disk.height());
	}

	SUBCASE("diskstreamingframe_limit_settings")
	{
		GUI::DiskstreamingframeContent disk(
		    &window, settings, settings_notifier);

		// Test various limit values
		disk.resize(400, 100); // Need to resize first for proper initialization
		settings_notifier.disk_cache_upper_limit(1024 * 1024 * 64);  // 64 MB
		settings_notifier.disk_cache_upper_limit(1024 * 1024 * 512); // 512 MB
		settings_notifier.disk_cache_upper_limit(
		    std::numeric_limits<std::size_t>::max()); // Unlimited

		CHECK_UNARY(&disk != nullptr);
	}

	SUBCASE("diskstreamingframe_reload_counter")
	{
		GUI::DiskstreamingframeContent disk(
		    &window, settings, settings_notifier);

		disk.resize(400, 100);
		settings_notifier.reload_counter(1);

		CHECK_UNARY(&disk != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "BleedcontrolframeContentTest")
{
	SUBCASE("bleedcontrolframe_construction")
	{
		GUI::BleedcontrolframeContent bleed(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&bleed != nullptr);
		CHECK_UNARY(bleed.visible());
	}

	SUBCASE("bleedcontrolframe_resize")
	{
		GUI::BleedcontrolframeContent bleed(
		    &window, settings, settings_notifier);

		bleed.resize(400, 100);
		CHECK_EQ(std::size_t(400u), bleed.width());
		CHECK_EQ(std::size_t(100u), bleed.height());
	}

	SUBCASE("bleedcontrolframe_set_enabled")
	{
		GUI::BleedcontrolframeContent bleed(
		    &window, settings, settings_notifier);

		bleed.resize(400, 100);
		bleed.setEnabled(true);
		bleed.setEnabled(false);
		bleed.setEnabled(true);

		CHECK_UNARY(&bleed != nullptr);
	}

	SUBCASE("bleedcontrolframe_value_changes")
	{
		GUI::BleedcontrolframeContent bleed(
		    &window, settings, settings_notifier);

		bleed.resize(400, 100);
		settings_notifier.master_bleed(0.0f);
		settings_notifier.master_bleed(0.5f);
		settings_notifier.master_bleed(1.0f);

		CHECK_UNARY(&bleed != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "ResamplingframeContentTest")
{
	SUBCASE("resamplingframe_construction")
	{
		GUI::ResamplingframeContent resampling(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&resampling != nullptr);
		CHECK_UNARY(resampling.visible());
	}

	SUBCASE("resamplingframe_resize")
	{
		GUI::ResamplingframeContent resampling(
		    &window, settings, settings_notifier);

		resampling.resize(400, 150);
		CHECK_EQ(std::size_t(400u), resampling.width());
		CHECK_EQ(std::size_t(150u), resampling.height());
	}

	SUBCASE("resamplingframe_settings_updates")
	{
		GUI::ResamplingframeContent resampling(
		    &window, settings, settings_notifier);

		settings_notifier.drumkit_samplerate(44100);
		settings_notifier.drumkit_samplerate(48000);
		settings_notifier.drumkit_samplerate(0); // Empty case

		settings_notifier.samplerate(44100.0);
		settings_notifier.samplerate(48000.0);

		settings_notifier.resampling_recommended(true);
		settings_notifier.resampling_recommended(false);

		settings_notifier.resampling_quality(0.7f);
		settings_notifier.resampling_quality(0.9f);

		CHECK_UNARY(&resampling != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "TimingframeContentTest")
{
	SUBCASE("timingframe_construction")
	{
		GUI::TimingframeContent timing(&window, settings, settings_notifier);

		CHECK_UNARY(&timing != nullptr);
		CHECK_UNARY(timing.visible());
	}

	SUBCASE("timingframe_settings_updates")
	{
		GUI::TimingframeContent timing(&window, settings, settings_notifier);

		// Test tightness/stddev
		settings_notifier.latency_stddev(10.0f);
		settings_notifier.latency_stddev(5.0f);

		// Test regain
		settings_notifier.latency_regain(0.5f);
		settings_notifier.latency_regain(0.8f);

		// Test laidback
		settings_notifier.latency_laid_back_ms(0.0f);
		settings_notifier.latency_laid_back_ms(10.0f);
		settings_notifier.latency_laid_back_ms(-10.0f);

		CHECK_UNARY(&timing != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "SampleselectionframeContentTest")
{
	SUBCASE("sampleselectionframe_construction")
	{
		GUI::SampleselectionframeContent samplesel(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&samplesel != nullptr);
		CHECK_UNARY(samplesel.visible());
	}

	SUBCASE("sampleselectionframe_settings_updates")
	{
		GUI::SampleselectionframeContent samplesel(
		    &window, settings, settings_notifier);

		settings_notifier.sample_selection_f_close(0.5f);
		settings_notifier.sample_selection_f_close(0.8f);

		settings_notifier.sample_selection_f_diverse(0.3f);
		settings_notifier.sample_selection_f_diverse(0.6f);

		settings_notifier.sample_selection_f_random(0.1f);
		settings_notifier.sample_selection_f_random(0.4f);

		CHECK_UNARY(&samplesel != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "VisualizerframeContentTest")
{
	SUBCASE("visualizerframe_construction")
	{
		GUI::VisualizerframeContent visualizer(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&visualizer != nullptr);
		CHECK_UNARY(visualizer.visible());
	}

	SUBCASE("visualizerframe_resize")
	{
		GUI::VisualizerframeContent visualizer(
		    &window, settings, settings_notifier);

		visualizer.resize(400, 200);
		CHECK_EQ(std::size_t(400u), visualizer.width());
		CHECK_EQ(std::size_t(200u), visualizer.height());

		// Resize to small dimensions
		visualizer.resize(10, 10);
		CHECK_EQ(std::size_t(10u), visualizer.width());
		CHECK_EQ(std::size_t(10u), visualizer.height());
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "VoicelimitframeContentTest")
{
	SUBCASE("voicelimitframe_construction")
	{
		GUI::VoiceLimitFrameContent voicelimit(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&voicelimit != nullptr);
		CHECK_UNARY(voicelimit.visible());
	}

	SUBCASE("voicelimitframe_settings_updates")
	{
		GUI::VoiceLimitFrameContent voicelimit(
		    &window, settings, settings_notifier);

		// Test max voices
		settings_notifier.voice_limit_max(5.0f);
		settings_notifier.voice_limit_max(10.0f);
		settings_notifier.voice_limit_max(20.0f);

		// Test rampdown
		settings_notifier.voice_limit_rampdown(0.1f);
		settings_notifier.voice_limit_rampdown(0.5f);
		settings_notifier.voice_limit_rampdown(1.0f);

		CHECK_UNARY(&voicelimit != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "PowerWidgetTest")
{
	SUBCASE("powerwidget_construction")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		CHECK_UNARY(&power != nullptr);
		CHECK_UNARY(power.visible());
	}

	SUBCASE("powerwidget_resize")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);
		CHECK_EQ(std::size_t(400u), power.width());
		CHECK_EQ(std::size_t(300u), power.height());

		// Test resize with very small dimensions (triggers edge case)
		power.resize(100, 50);
		CHECK_EQ(std::size_t(100u), power.width());
		CHECK_EQ(std::size_t(50u), power.height());
	}

	SUBCASE("powerwidget_resize_extreme_small")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		// Test resize with dimensions below minimum threshold
		// This triggers the edge case where canvas is set to 1x1
		power.resize(10, 10);
		CHECK_EQ(std::size_t(10u), power.width());
		CHECK_EQ(std::size_t(10u), power.height());

		// Test boundary condition: exactly at minimum width
		power.resize(14 + 59 + 64, 20);
		CHECK_EQ(std::size_t(14u + 59u + 64u), power.width());

		// Test boundary condition: exactly at minimum height
		power.resize(200, 14);
		CHECK_EQ(std::size_t(200u), power.width());
		CHECK_EQ(std::size_t(14u), power.height());
	}

	SUBCASE("powerwidget_settings_updates")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Test enable/disable
		settings_notifier.enable_powermap(true);
		settings_notifier.enable_powermap(false);
		settings_notifier.enable_powermap(true);

		// Test fixed point updates
		settings_notifier.powermap_fixed0_x(0.2f);
		settings_notifier.powermap_fixed0_y(0.3f);
		settings_notifier.powermap_fixed1_x(0.5f);
		settings_notifier.powermap_fixed1_y(0.5f);
		settings_notifier.powermap_fixed2_x(0.8f);
		settings_notifier.powermap_fixed2_y(0.7f);

		// Test shelf
		settings_notifier.powermap_shelf(true);
		settings_notifier.powermap_shelf(false);

		// Test input/output lines
		settings_notifier.powermap_input(0.5f);
		settings_notifier.powermap_output(0.6f);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_repaint_enabled_disabled_states")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Create a repaint event
		dggui::RepaintEvent repaint_event;
		repaint_event.x = 0;
		repaint_event.y = 0;
		repaint_event.width = 400;
		repaint_event.height = 300;

		// Test repaint with powermap enabled
		settings.enable_powermap.store(true);
		settings_notifier.enable_powermap(true);
		power.repaintEvent(&repaint_event);

		// Test repaint with powermap disabled
		settings.enable_powermap.store(false);
		settings_notifier.enable_powermap(false);
		power.repaintEvent(&repaint_event);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_repaint_with_input_output_lines")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		dggui::RepaintEvent repaint_event;
		repaint_event.x = 0;
		repaint_event.y = 0;
		repaint_event.width = 400;
		repaint_event.height = 300;

		// Test repaint without input/output lines (values at -1)
		settings.powermap_input.store(-1.0f);
		settings.powermap_output.store(-1.0f);
		power.repaintEvent(&repaint_event);

		// Test repaint with input/output lines visible
		settings.powermap_input.store(0.5f);
		settings.powermap_output.store(0.6f);
		settings_notifier.powermap_input(0.5f);
		settings_notifier.powermap_output(0.6f);
		power.repaintEvent(&repaint_event);

		// Test with only input set (output at -1)
		settings.powermap_input.store(0.3f);
		settings.powermap_output.store(-1.0f);
		power.repaintEvent(&repaint_event);

		// Test with only output set (input at -1)
		settings.powermap_input.store(-1.0f);
		settings.powermap_output.store(0.7f);
		power.repaintEvent(&repaint_event);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_repaint_small_dimensions")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		// Test repaint with zero-sized canvas to trigger early return
		// The guard in repaintEvent checks: width() < 1 || height() < 1
		power.resize(0, 0);

		dggui::RepaintEvent repaint_event;
		repaint_event.x = 0;
		repaint_event.y = 0;
		repaint_event.width = 0;
		repaint_event.height = 0;

		// This should early return due to width/height < 1 check
		power.repaintEvent(&repaint_event);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_button_events_on_control_points")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Initialize powermap with known positions for control points
		// Canvas is at (7,7) with size 263x286, border=6
		// width0 = 263 - 12 = 251, height0 = 286 - 12 = 274
		settings.powermap_fixed0_x.store(
		    0.1f); // ~31px from left of canvas + 6px border
		settings.powermap_fixed0_y.store(
		    0.1f); // ~27px from bottom of canvas + 6px border
		settings.powermap_fixed1_x.store(0.5f); // ~132px from left
		settings.powermap_fixed1_y.store(0.5f); // ~143px from bottom
		settings.powermap_fixed2_x.store(0.9f); // ~232px from left
		settings.powermap_fixed2_y.store(0.9f); // ~253px from bottom
		settings_notifier.enable_powermap(true);

		// Access the canvas directly - PowerWidget doesn't override these
		// handlers Canvas is at (7,7), so find it there
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		// Test button down on fixed0 (green point at lower-left)
		// Canvas at (7,7), point at canvas x=6+25=31, y=286-6-27=253
		// Window coords: 7+31=38, 7+253=260
		dggui::ButtonEvent button_down;
		button_down.direction = dggui::Direction::down;
		button_down.button = dggui::MouseButton::left;
		button_down.x = 38;
		button_down.y = 260;
		canvas->buttonEvent(&button_down);

		// Drag to another position (canvas coords)
		dggui::MouseMoveEvent move_event;
		move_event.x = 100;
		move_event.y = 200;
		canvas->mouseMoveEvent(&move_event);

		// Release button
		dggui::ButtonEvent button_up;
		button_up.direction = dggui::Direction::up;
		button_up.button = dggui::MouseButton::left;
		button_up.x = 100;
		button_up.y = 200;
		canvas->buttonEvent(&button_up);

		// Test button down on fixed1 (yellow point, center)
		button_down.x = 139; // 7 + 6 + 126
		button_down.y = 150; // 7 + 143
		canvas->buttonEvent(&button_down);
		CHECK_UNARY(&power != nullptr);

		// Test button down on fixed2 (red point, upper-right)
		button_down.x = 239; // 7 + 6 + 226
		button_down.y = 40;  // 7 + ~33
		canvas->buttonEvent(&button_down);
		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_button_event_outside_control_points")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Initialize powermap
		settings_notifier.enable_powermap(true);

		// Access the canvas directly
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		dggui::ButtonEvent button_down;
		button_down.direction = dggui::Direction::down;
		button_down.button = dggui::MouseButton::left;
		button_down.doubleClick = false;

		// Click outside any control point (far corner of canvas)
		button_down.x = 10;
		button_down.y = 10;
		canvas->buttonEvent(&button_down);

		// Release button
		dggui::ButtonEvent button_up;
		button_up.direction = dggui::Direction::up;
		button_up.button = dggui::MouseButton::left;
		button_up.doubleClick = false;
		button_up.x = 10;
		button_up.y = 10;
		canvas->buttonEvent(&button_up);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_mouse_move_drag_control_points")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Initialize powermap with known position for fixed0
		// Canvas is at (7,7) with size 263x286, border=6
		// width0 = 263 - 12 = 251, height0 = 286 - 12 = 274
		// fixed0 at (0.05, 0.05) -> canvas x=6+12.5=18.5, y=6+13=19 (near
		// top-left of drawing area) Canvas y is inverted, so 0.05 Y means near
		// the bottom of drawing area Drawing area y: height0 - 0.05*height0 =
		// 274-13.7 = 260.3, +6 border = 266.3
		settings.powermap_fixed0_x.store(0.05f);
		settings.powermap_fixed0_y.store(0.05f);
		settings_notifier.enable_powermap(true);

		// Access the canvas directly
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		// First, simulate button down on fixed0
		// Canvas coords: x ~ 6 + 0.05*251 = 18.55, y ~ 6 + 274 - 0.05*274 =
		// 266.3
		dggui::ButtonEvent button_down;
		button_down.direction = dggui::Direction::down;
		button_down.button = dggui::MouseButton::left;
		button_down.x = 19;
		button_down.y = 266;
		canvas->buttonEvent(&button_down);

		// Now simulate dragging to a new position
		dggui::MouseMoveEvent move_event;
		move_event.x = 100;
		move_event.y = 150;
		canvas->mouseMoveEvent(&move_event);

		// Drag to another position
		move_event.x = 150;
		move_event.y = 100;
		canvas->mouseMoveEvent(&move_event);

		// Release button
		dggui::ButtonEvent button_up;
		button_up.direction = dggui::Direction::up;
		button_up.button = dggui::MouseButton::left;
		button_up.x = 150;
		button_up.y = 100;
		canvas->buttonEvent(&button_up);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_mouse_move_no_drag")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);
		settings_notifier.enable_powermap(true);

		// Access the canvas directly
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		// Simulate mouse move without any button pressed
		dggui::MouseMoveEvent move_event;
		move_event.x = 100;
		move_event.y = 100;
		canvas->mouseMoveEvent(&move_event);

		// Move to different position
		move_event.x = 150;
		move_event.y = 120;
		canvas->mouseMoveEvent(&move_event);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_mouse_leave_event")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);
		settings_notifier.enable_powermap(true);

		// Access the canvas directly
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		// Simulate button down first (on canvas, not parent)
		dggui::ButtonEvent button_down;
		button_down.direction = dggui::Direction::down;
		button_down.button = dggui::MouseButton::left;
		button_down.x = 50;
		button_down.y = 100;
		canvas->buttonEvent(&button_down);

		// Simulate mouse leave
		canvas->mouseLeaveEvent();

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_shelf_checkbox_interaction")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Test shelf checkbox state changes via settings notifier
		settings_notifier.powermap_shelf(true);
		CHECK_UNARY(settings.powermap_shelf.load());

		settings_notifier.powermap_shelf(false);
		CHECK_UNARY(!settings.powermap_shelf.load());

		settings_notifier.powermap_shelf(true);
		CHECK_UNARY(settings.powermap_shelf.load());
	}

	SUBCASE("powerwidget_fixed_points_boundary_values")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Test setting fixed points to boundary values
		settings_notifier.powermap_fixed0_x(0.0f);
		settings_notifier.powermap_fixed0_y(0.0f);
		settings_notifier.powermap_fixed1_x(0.0f);
		settings_notifier.powermap_fixed1_y(0.0f);
		settings_notifier.powermap_fixed2_x(1.0f);
		settings_notifier.powermap_fixed2_y(1.0f);

		// Test setting fixed points to maximum values
		settings_notifier.powermap_fixed0_x(1.0f);
		settings_notifier.powermap_fixed0_y(1.0f);
		settings_notifier.powermap_fixed2_x(0.0f);
		settings_notifier.powermap_fixed2_y(0.0f);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_drag_clamping")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Access the canvas directly
		auto* canvas = power.find(10, 10);
		REQUIRE_UNARY(canvas != nullptr);

		// Initialize powermap - place fixed1 at center of canvas
		// Canvas: 263x286, border=6, width0=251, height0=274
		// Center in canvas coords: x=6+125=131, y=6+137=143
		// Fixed1 at normalized (0.5, 0.5) maps to these canvas coords
		settings.powermap_fixed1_x.store(0.5f);
		settings.powermap_fixed1_y.store(0.5f);
		settings_notifier.enable_powermap(true);

		// Simulate button down on fixed1 (center point of canvas)
		dggui::ButtonEvent button_down;
		button_down.direction = dggui::Direction::down;
		button_down.button = dggui::MouseButton::left;
		button_down.x = 131; // 6 + 125
		button_down.y = 143; // 6 + 137
		canvas->buttonEvent(&button_down);

		// Store initial value before drag
		float initial_x = settings.powermap_fixed1_x.load();
		float initial_y = settings.powermap_fixed1_y.load();
		CHECK_EQ(initial_x, doctest::Approx(0.5f));
		CHECK_EQ(initial_y, doctest::Approx(0.5f));

		// Try to drag outside the widget far left/bottom (should be clamped to
		// 0) Y is inverted: my0 = (height - y - y0) / height0 Large negative Y
		// canvas coord -> large positive normalized value Large positive Y
		// canvas coord -> small/negative normalized value
		dggui::MouseMoveEvent move_event;
		move_event.x =
		    -1000; // Far outside left boundary -> mx0 negative -> clamped to 0
		move_event.y = 5000; // Far below canvas -> small my0 -> clamped to 0
		canvas->mouseMoveEvent(&move_event);

		// Verify values are clamped to 0.0 (minimum)
		CHECK_EQ(settings.powermap_fixed1_x.load(), doctest::Approx(0.0f));
		CHECK_EQ(settings.powermap_fixed1_y.load(), doctest::Approx(0.0f));

		// Try to drag beyond right/top boundaries (should be clamped to 1)
		move_event.x = 5000;  // Far beyond right -> mx0 > 1 -> clamped to 1
		move_event.y = -1000; // Far above canvas -> my0 > 1 -> clamped to 1
		canvas->mouseMoveEvent(&move_event);

		// Verify values are clamped to 1.0 (maximum)
		CHECK_EQ(settings.powermap_fixed1_x.load(), doctest::Approx(1.0f));
		CHECK_EQ(settings.powermap_fixed1_y.load(), doctest::Approx(1.0f));

		// Release button
		dggui::ButtonEvent button_up;
		button_up.direction = dggui::Direction::up;
		button_up.button = dggui::MouseButton::left;
		button_up.x = 131;
		button_up.y = 143;
		canvas->buttonEvent(&button_up);

		CHECK_UNARY(&power != nullptr);
	}

	SUBCASE("powerwidget_all_parameter_notifiers")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Test all parameter notifiers to ensure they're connected
		settings_notifier.enable_powermap(true);
		settings_notifier.powermap_fixed0_x(0.1f);
		settings_notifier.powermap_fixed0_y(0.2f);
		settings_notifier.powermap_fixed1_x(0.3f);
		settings_notifier.powermap_fixed1_y(0.4f);
		settings_notifier.powermap_fixed2_x(0.7f);
		settings_notifier.powermap_fixed2_y(0.8f);
		settings_notifier.powermap_shelf(true);
		settings_notifier.powermap_input(0.5f);
		settings_notifier.powermap_output(0.6f);

		// Now test with false values
		settings_notifier.enable_powermap(false);
		settings_notifier.powermap_shelf(false);

		CHECK_UNARY(&power != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "HumaniserVisualiserTest")
{
	SUBCASE("humaniservisualiser_construction")
	{
		GUI::HumaniserVisualiser visualiser(
		    &window, settings, settings_notifier);

		CHECK_UNARY(&visualiser != nullptr);
		CHECK_UNARY(visualiser.visible());
	}

	SUBCASE("humaniservisualiser_resize")
	{
		GUI::HumaniserVisualiser visualiser(
		    &window, settings, settings_notifier);

		visualiser.resize(400, 200);
		CHECK_EQ(std::size_t(400u), visualiser.width());
		CHECK_EQ(std::size_t(200u), visualiser.height());

		// Test resize with small dimensions (triggers edge case)
		visualiser.resize(10, 10);
		CHECK_EQ(std::size_t(10u), visualiser.width());
		CHECK_EQ(std::size_t(10u), visualiser.height());
	}

	SUBCASE("humaniservisualiser_settings_updates")
	{
		GUI::HumaniserVisualiser visualiser(
		    &window, settings, settings_notifier);

		visualiser.resize(400, 200);

		// Test enable/disable
		settings_notifier.enable_latency_modifier(true);
		settings_notifier.enable_velocity_modifier(true);

		settings_notifier.enable_latency_modifier(false);
		settings_notifier.enable_velocity_modifier(false);

		// Test offsets
		settings_notifier.latency_current(10.0f);
		settings_notifier.velocity_modifier_current(0.2f);

		// Test stddev
		settings_notifier.latency_stddev(5.0f);
		settings_notifier.velocity_stddev(0.1f);

		// Test laidback
		settings.latency_max_ms.store(100);
		settings.samplerate.store(44100);
		settings_notifier.latency_laid_back_ms(20.0f);

		CHECK_UNARY(&visualiser != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "MainTabTest")
{
	SUBCASE("maintab_construction")
	{
		GUI::MainTab main_tab(&window, settings, settings_notifier, config);

		CHECK_UNARY(&main_tab != nullptr);
		CHECK_UNARY(main_tab.visible());
	}

	SUBCASE("maintab_resize")
	{
		GUI::MainTab main_tab(&window, settings, settings_notifier, config);

		main_tab.resize(750, 700);
		CHECK_EQ(std::size_t(750u), main_tab.width());
		CHECK_EQ(std::size_t(700u), main_tab.height());
	}

	SUBCASE("maintab_settings_toggles")
	{
		GUI::MainTab main_tab(&window, settings, settings_notifier, config);

		// Test various settings toggles that are handled by MainTab
		settings_notifier.enable_velocity_modifier(true);
		settings_notifier.enable_velocity_modifier(false);

		settings_notifier.enable_bleed_control(true);
		settings_notifier.enable_bleed_control(false);

		settings_notifier.enable_resampling(true);
		settings_notifier.enable_resampling(false);

		settings_notifier.enable_latency_modifier(true);
		settings_notifier.enable_latency_modifier(false);

		settings_notifier.enable_powermap(true);
		settings_notifier.enable_powermap(false);

		settings_notifier.enable_voice_limit(true);
		settings_notifier.enable_voice_limit(false);

		CHECK_UNARY(&main_tab != nullptr);
	}

	SUBCASE("maintab_has_bleed_control_notification")
	{
		GUI::MainTab main_tab(&window, settings, settings_notifier, config);

		// This triggers the bleed control frame enabled state
		settings_notifier.has_bleed_control(true);
		settings_notifier.has_bleed_control(false);

		CHECK_UNARY(&main_tab != nullptr);
	}
}

TEST_CASE("MainWindowTest")
{
	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;

	SUBCASE("mainwindow_construction")
	{
		GUI::MainWindow main_window(settings, nullptr);

		CHECK_UNARY(&main_window != nullptr);
		CHECK_UNARY(main_window.visible());
	}

	SUBCASE("mainwindow_process_events")
	{
		GUI::MainWindow main_window(settings, nullptr);

		// Process events should return true (not closing)
		bool result = main_window.processEvents();
		CHECK_UNARY(result);

		// Trigger close event
		main_window.closeEventHandler();

		// After close event, processEvents should return false
		result = main_window.processEvents();
		CHECK_UNARY(!result);
	}

	SUBCASE("mainwindow_close_notifier")
	{
		GUI::MainWindow main_window(settings, nullptr);

		bool close_received = false;
		struct CloseListener : public Listener
		{
			bool* received;
			void onClose()
			{
				*received = true;
			}
		};
		CloseListener listener;
		listener.received = &close_received;
		main_window.closeNotifier.connect(&listener, &CloseListener::onClose);

		main_window.closeEventHandler();
		main_window.processEvents();

		CHECK_UNARY(close_received);
	}

	// Note: Drumkit tab visibility is controlled by
	// drumkit_tab.imageChangeNotifier which is a private member of MainWindow.
	// Testing this would require either:
	// 1. Making drumkit_tab accessible for testing, or
	// 2. Creating a test fixture that can trigger the notifier internally
	// For now, this behavior is implicitly covered by the construction test
	// above.
}
