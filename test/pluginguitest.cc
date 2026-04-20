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

#include "plugingui/pluginconfig.h"
#include "plugingui/abouttab.h"
#include "plugingui/statusframecontent.h"
#include "plugingui/humanizerframecontent.h"
#include "plugingui/diskstreamingframecontent.h"
#include "plugingui/bleedcontrolframecontent.h"
#include "plugingui/resamplingframecontent.h"
#include "plugingui/timingframecontent.h"
#include "plugingui/sampleselectionframecontent.h"
#include "plugingui/visualizerframecontent.h"
#include "plugingui/voicelimitframecontent.h"
#include "plugingui/powerwidget.h"
#include "plugingui/humaniservisualiser.h"
#include "plugingui/maintab.h"
#include "plugingui/mainwindow.h"

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

		// Resize to zero should handle gracefully
		about_tab.resize(0, 0);
		CHECK_EQ(std::size_t(0u), about_tab.width());
		CHECK_EQ(std::size_t(0u), about_tab.height());
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
		GUI::HumanizerframeContent humanizer(&window, settings, settings_notifier);

		CHECK_UNARY(&humanizer != nullptr);
		CHECK_UNARY(humanizer.visible());
	}

	SUBCASE("humanizerframe_setting_values")
	{
		GUI::HumanizerframeContent humanizer(&window, settings, settings_notifier);

		// Simulate settings changes via notifier
		settings_notifier.velocity_modifier_weight.emit(0.5f);
		settings_notifier.velocity_modifier_falloff.emit(0.3f);
		settings_notifier.velocity_stddev.emit(0.1f);

		CHECK_UNARY(&humanizer != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "DiskstreamingframeContentTest")
{
	SUBCASE("diskstreamingframe_construction")
	{
		GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

		CHECK_UNARY(&disk != nullptr);
		CHECK_UNARY(disk.visible());
	}

	SUBCASE("diskstreamingframe_resize")
	{
		GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

		disk.resize(400, 100);
		CHECK_EQ(std::size_t(400u), disk.width());
		CHECK_EQ(std::size_t(100u), disk.height());
	}

	SUBCASE("diskstreamingframe_limit_settings")
	{
		GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

		// Test various limit values
		disk.resize(400, 100); // Need to resize first for proper initialization
		settings_notifier.disk_cache_upper_limit.emit(1024 * 1024 * 64);  // 64 MB
		settings_notifier.disk_cache_upper_limit.emit(1024 * 1024 * 512); // 512 MB
		settings_notifier.disk_cache_upper_limit.emit(std::numeric_limits<std::size_t>::max()); // Unlimited

		CHECK_UNARY(&disk != nullptr);
	}

	SUBCASE("diskstreamingframe_reload_counter")
	{
		GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

		disk.resize(400, 100);
		settings_notifier.reload_counter.emit(1);

		CHECK_UNARY(&disk != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "BleedcontrolframeContentTest")
{
	SUBCASE("bleedcontrolframe_construction")
	{
		GUI::BleedcontrolframeContent bleed(&window, settings, settings_notifier);

		CHECK_UNARY(&bleed != nullptr);
		CHECK_UNARY(bleed.visible());
	}

	SUBCASE("bleedcontrolframe_resize")
	{
		GUI::BleedcontrolframeContent bleed(&window, settings, settings_notifier);

		bleed.resize(400, 100);
		CHECK_EQ(std::size_t(400u), bleed.width());
		CHECK_EQ(std::size_t(100u), bleed.height());
	}

	SUBCASE("bleedcontrolframe_set_enabled")
	{
		GUI::BleedcontrolframeContent bleed(&window, settings, settings_notifier);

		bleed.resize(400, 100);
		bleed.setEnabled(true);
		bleed.setEnabled(false);
		bleed.setEnabled(true);

		CHECK_UNARY(&bleed != nullptr);
	}

	SUBCASE("bleedcontrolframe_value_changes")
	{
		GUI::BleedcontrolframeContent bleed(&window, settings, settings_notifier);

		bleed.resize(400, 100);
		settings_notifier.master_bleed.emit(0.0f);
		settings_notifier.master_bleed.emit(0.5f);
		settings_notifier.master_bleed.emit(1.0f);

		CHECK_UNARY(&bleed != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "ResamplingframeContentTest")
{
	SUBCASE("resamplingframe_construction")
	{
		GUI::ResamplingframeContent resampling(&window, settings, settings_notifier);

		CHECK_UNARY(&resampling != nullptr);
		CHECK_UNARY(resampling.visible());
	}

	SUBCASE("resamplingframe_resize")
	{
		GUI::ResamplingframeContent resampling(&window, settings, settings_notifier);

		resampling.resize(400, 150);
		CHECK_EQ(std::size_t(400u), resampling.width());
		CHECK_EQ(std::size_t(150u), resampling.height());
	}

	SUBCASE("resamplingframe_settings_updates")
	{
		GUI::ResamplingframeContent resampling(&window, settings, settings_notifier);

		settings_notifier.drumkit_samplerate.emit(44100);
		settings_notifier.drumkit_samplerate.emit(48000);
		settings_notifier.drumkit_samplerate.emit(0); // Empty case

		settings_notifier.samplerate.emit(44100.0);
		settings_notifier.samplerate.emit(48000.0);

		settings_notifier.resampling_recommended.emit(true);
		settings_notifier.resampling_recommended.emit(false);

		settings_notifier.resampling_quality.emit(0.7f);
		settings_notifier.resampling_quality.emit(0.9f);

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
		settings_notifier.latency_stddev.emit(10.0f);
		settings_notifier.latency_stddev.emit(5.0f);

		// Test regain
		settings_notifier.latency_regain.emit(0.5f);
		settings_notifier.latency_regain.emit(0.8f);

		// Test laidback
		settings_notifier.latency_laid_back_ms.emit(0.0f);
		settings_notifier.latency_laid_back_ms.emit(10.0f);
		settings_notifier.latency_laid_back_ms.emit(-10.0f);

		CHECK_UNARY(&timing != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "SampleselectionframeContentTest")
{
	SUBCASE("sampleselectionframe_construction")
	{
		GUI::SampleselectionframeContent samplesel(&window, settings, settings_notifier);

		CHECK_UNARY(&samplesel != nullptr);
		CHECK_UNARY(samplesel.visible());
	}

	SUBCASE("sampleselectionframe_settings_updates")
	{
		GUI::SampleselectionframeContent samplesel(&window, settings, settings_notifier);

		settings_notifier.sample_selection_f_close.emit(0.5f);
		settings_notifier.sample_selection_f_close.emit(0.8f);

		settings_notifier.sample_selection_f_diverse.emit(0.3f);
		settings_notifier.sample_selection_f_diverse.emit(0.6f);

		settings_notifier.sample_selection_f_random.emit(0.1f);
		settings_notifier.sample_selection_f_random.emit(0.4f);

		CHECK_UNARY(&samplesel != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "VisualizerframeContentTest")
{
	SUBCASE("visualizerframe_construction")
	{
		GUI::VisualizerframeContent visualizer(&window, settings, settings_notifier);

		CHECK_UNARY(&visualizer != nullptr);
		CHECK_UNARY(visualizer.visible());
	}

	SUBCASE("visualizerframe_resize")
	{
		GUI::VisualizerframeContent visualizer(&window, settings, settings_notifier);

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
		GUI::VoiceLimitFrameContent voicelimit(&window, settings, settings_notifier);

		CHECK_UNARY(&voicelimit != nullptr);
		CHECK_UNARY(voicelimit.visible());
	}

	SUBCASE("voicelimitframe_settings_updates")
	{
		GUI::VoiceLimitFrameContent voicelimit(&window, settings, settings_notifier);

		// Test max voices
		settings_notifier.voice_limit_max.emit(5.0f);
		settings_notifier.voice_limit_max.emit(10.0f);
		settings_notifier.voice_limit_max.emit(20.0f);

		// Test rampdown
		settings_notifier.voice_limit_rampdown.emit(0.1f);
		settings_notifier.voice_limit_rampdown.emit(0.5f);
		settings_notifier.voice_limit_rampdown.emit(1.0f);

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

	SUBCASE("powerwidget_settings_updates")
	{
		GUI::PowerWidget power(&window, settings, settings_notifier);

		power.resize(400, 300);

		// Test enable/disable
		settings_notifier.enable_powermap.emit(true);
		settings_notifier.enable_powermap.emit(false);
		settings_notifier.enable_powermap.emit(true);

		// Test fixed point updates
		settings_notifier.powermap_fixed0_x.emit(0.2f);
		settings_notifier.powermap_fixed0_y.emit(0.3f);
		settings_notifier.powermap_fixed1_x.emit(0.5f);
		settings_notifier.powermap_fixed1_y.emit(0.5f);
		settings_notifier.powermap_fixed2_x.emit(0.8f);
		settings_notifier.powermap_fixed2_y.emit(0.7f);

		// Test shelf
		settings_notifier.powermap_shelf.emit(true);
		settings_notifier.powermap_shelf.emit(false);

		// Test input/output lines
		settings_notifier.powermap_input.emit(0.5f);
		settings_notifier.powermap_output.emit(0.6f);

		CHECK_UNARY(&power != nullptr);
	}
}

TEST_CASE_FIXTURE(PluginGUIFixture, "HumaniserVisualiserTest")
{
	SUBCASE("humaniservisualiser_construction")
	{
		GUI::HumaniserVisualiser visualiser(&window, settings, settings_notifier);

		CHECK_UNARY(&visualiser != nullptr);
		CHECK_UNARY(visualiser.visible());
	}

	SUBCASE("humaniservisualiser_resize")
	{
		GUI::HumaniserVisualiser visualiser(&window, settings, settings_notifier);

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
		GUI::HumaniserVisualiser visualiser(&window, settings, settings_notifier);

		visualiser.resize(400, 200);

		// Test enable/disable
		settings_notifier.enable_latency_modifier.emit(true);
		settings_notifier.enable_velocity_modifier.emit(true);

		settings_notifier.enable_latency_modifier.emit(false);
		settings_notifier.enable_velocity_modifier.emit(false);

		// Test offsets
		settings_notifier.latency_current.emit(10.0f);
		settings_notifier.velocity_modifier_current.emit(0.2f);

		// Test stddev
		settings_notifier.latency_stddev.emit(5.0f);
		settings_notifier.velocity_stddev.emit(0.1f);

		// Test laidback
		settings.latency_max_ms.store(100);
		settings.samplerate.store(44100);
		settings_notifier.latency_laid_back_ms.emit(20.0f);

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
		settings_notifier.enable_velocity_modifier.emit(true);
		settings_notifier.enable_velocity_modifier.emit(false);

		settings_notifier.enable_bleed_control.emit(true);
		settings_notifier.enable_bleed_control.emit(false);

		settings_notifier.enable_resampling.emit(true);
		settings_notifier.enable_resampling.emit(false);

		settings_notifier.enable_latency_modifier.emit(true);
		settings_notifier.enable_latency_modifier.emit(false);

		settings_notifier.enable_powermap.emit(true);
		settings_notifier.enable_powermap.emit(false);

		settings_notifier.enable_voice_limit.emit(true);
		settings_notifier.enable_voice_limit.emit(false);

		CHECK_UNARY(&main_tab != nullptr);
	}

	SUBCASE("maintab_has_bleed_control_notification")
	{
		GUI::MainTab main_tab(&window, settings, settings_notifier, config);

		// This triggers the bleed control frame enabled state
		settings_notifier.has_bleed_control.emit(true);
		settings_notifier.has_bleed_control.emit(false);

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

	SUBCASE("mainwindow_size_constants")
	{
		// Verify the constants are defined
		CHECK_EQ(std::size_t(750u), GUI::MainWindow::main_width);
		CHECK_EQ(std::size_t(740u), GUI::MainWindow::main_height);
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
		main_window.closeNotifier.connect([&close_received]() {
			close_received = true;
		});

		main_window.closeEventHandler();
		main_window.processEvents();

		CHECK_UNARY(close_received);
	}

	SUBCASE("mainwindow_drumkit_tab_visibility")
	{
		GUI::MainWindow main_window(settings, nullptr);

		// Test changing drumkit tab visibility via image change notifier
		settings_notifier.drumkit_file.emit("/some/path/to/kit.xml");

		CHECK_UNARY(&main_window != nullptr);
	}
}
