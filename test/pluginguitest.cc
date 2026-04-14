/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pluginguitest.cc
 *
 *  Fri Apr 14 2026
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

#include <cstdio>
#include <cstdlib>
#include <string>

#include <configfile.h>
#include <settings.h>

#include <dggui/window.h>

#include <plugingui/bleedcontrolframecontent.h>
#include <plugingui/diskstreamingframecontent.h>
#include <plugingui/drumkitframecontent.h>
#include <plugingui/humanizerframecontent.h>
#include <plugingui/maintab.h>
#include <plugingui/pluginconfig.h>
#include <plugingui/powerwidget.h>
#include <plugingui/resamplingframecontent.h>
#include <plugingui/sampleselectionframecontent.h>
#include <plugingui/statusframecontent.h>
#include <plugingui/timingframecontent.h>
#include <plugingui/voicelimitframecontent.h>

// PluginConfig tests are covered in pluginconfigtest.cc; those are not
// duplicated here.  This file focuses on plugingui widget headless tests.

TEST_CASE("StatusframeContent_loadstatus_all_states")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::StatusframeContent status(&window, settings_notifier);

	SUBCASE("drumkit_load_status_idle")
	{
		settings.drumkit_load_status.store(LoadStatus::Idle);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Idle);
	}

	SUBCASE("drumkit_load_status_parsing")
	{
		settings.drumkit_load_status.store(LoadStatus::Parsing);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Parsing);
	}

	SUBCASE("drumkit_load_status_loading")
	{
		settings.drumkit_load_status.store(LoadStatus::Loading);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Loading);
	}

	SUBCASE("drumkit_load_status_done")
	{
		settings.drumkit_load_status.store(LoadStatus::Done);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);
	}

	SUBCASE("drumkit_load_status_error")
	{
		settings.drumkit_load_status.store(LoadStatus::Error);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Error);
	}

	SUBCASE("midimap_load_status_idle")
	{
		settings.midimap_load_status.store(LoadStatus::Idle);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Idle);
	}

	SUBCASE("midimap_load_status_parsing")
	{
		settings.midimap_load_status.store(LoadStatus::Parsing);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Parsing);
	}

	SUBCASE("midimap_load_status_done")
	{
		settings.midimap_load_status.store(LoadStatus::Done);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Done);
	}

	SUBCASE("midimap_load_status_error")
	{
		settings.midimap_load_status.store(LoadStatus::Error);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Error);
	}
}

TEST_CASE("ResamplingFrameContent_updates")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::ResamplingframeContent resampling(
	    &window, settings, settings_notifier);

	SUBCASE("drumkit_samplerate_zero")
	{
		settings.drumkit_samplerate.store(0);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_samplerate.load(), std::size_t(0));
	}

	SUBCASE("drumkit_samplerate_nonzero")
	{
		settings.drumkit_samplerate.store(48000);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_samplerate.load(), std::size_t(48000));
	}

	SUBCASE("resampling_recommended_true")
	{
		settings.resampling_recommended.store(true);
		settings_notifier.evaluate();
		CHECK(settings.resampling_recommended.load());
	}

	SUBCASE("resampling_recommended_false")
	{
		settings.resampling_recommended.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.resampling_recommended.load());
	}

	SUBCASE("samplerate_update")
	{
		settings.samplerate.store(96000.0);
		settings_notifier.evaluate();
		CHECK_EQ(settings.samplerate.load(), 96000.0);
	}

	SUBCASE("resampling_quality_update")
	{
		settings.resampling_quality.store(0.85f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.resampling_quality.load(), 0.85f);
	}
}

TEST_CASE("DiskstreamingFrameContent_limits")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

	SUBCASE("disk_cache_upper_limit_below_threshold")
	{
		constexpr std::size_t min_limit = 1024ull * 1024ull * 32ull;
		std::size_t test_value = min_limit + 1024ull * 1024ull * 100ull;
		settings.disk_cache_upper_limit.store(test_value);
		settings_notifier.evaluate();
		CHECK_EQ(settings.disk_cache_upper_limit.load(), test_value);
	}

	SUBCASE("reload_counter_increment")
	{
		std::size_t before = settings.reload_counter.load();
		settings.reload_counter.store(before + 1);
		settings_notifier.evaluate();
		CHECK_EQ(settings.reload_counter.load(), before + 1);
	}
}

TEST_CASE("BleedcontrolFrameContent_enable_disable")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::BleedcontrolframeContent bleed(&window, settings, settings_notifier);

	SUBCASE("bleed_control_enabled")
	{
		settings.has_bleed_control.store(true);
		settings_notifier.evaluate();
		CHECK(settings.has_bleed_control.load());
	}

	SUBCASE("bleed_control_disabled")
	{
		settings.has_bleed_control.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.has_bleed_control.load());
	}

	SUBCASE("master_bleed_value")
	{
		settings.master_bleed.store(0.75f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.master_bleed.load(), 0.75f);
	}
}

TEST_CASE("HumanizerFrameContent_settings")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::HumanizerframeContent humanizer(&window, settings, settings_notifier);

	SUBCASE("velocity_modifier_weight_update")
	{
		settings.velocity_modifier_weight.store(0.3f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.velocity_modifier_weight.load(), 0.3f);
	}

	SUBCASE("velocity_modifier_falloff_update")
	{
		settings.velocity_modifier_falloff.store(0.7f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.velocity_modifier_falloff.load(), 0.7f);
	}

	SUBCASE("velocity_stddev_update")
	{
		settings.velocity_stddev.store(0.5f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.velocity_stddev.load(), 0.5f);
	}
}

TEST_CASE("TimingFrameContent_conversions")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::TimingframeContent timing(&window, settings, settings_notifier);

	SUBCASE("latency_stddev_update")
	{
		settings.latency_stddev.store(5.0f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.latency_stddev.load(), 5.0f);
	}

	SUBCASE("latency_regain_update")
	{
		settings.latency_regain.store(0.8f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.latency_regain.load(), 0.8f);
	}

	SUBCASE("latency_laid_back_ms_update")
	{
		settings.latency_laid_back_ms.store(15.0f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.latency_laid_back_ms.load(), 15.0f);
	}
}

TEST_CASE("SampleSelectionFrameContent_settings")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::SampleselectionframeContent selection(
	    &window, settings, settings_notifier);

	SUBCASE("f_close_update")
	{
		settings.sample_selection_f_close.store(0.9f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.sample_selection_f_close.load(), 0.9f);
	}

	SUBCASE("f_diverse_update")
	{
		settings.sample_selection_f_diverse.store(0.2f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.sample_selection_f_diverse.load(), 0.2f);
	}

	SUBCASE("f_random_update")
	{
		settings.sample_selection_f_random.store(0.05f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.sample_selection_f_random.load(), 0.05f);
	}
}

TEST_CASE("VoiceLimitFrameContent_settings")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::VoiceLimitFrameContent voicelimit(
	    &window, settings, settings_notifier);

	SUBCASE("voice_limit_max_update")
	{
		settings.voice_limit_max.store(std::size_t(20));
		settings_notifier.evaluate();
		CHECK_EQ(settings.voice_limit_max.load(), std::size_t(20));
	}

	SUBCASE("voice_limit_rampdown_update")
	{
		settings.voice_limit_rampdown.store(1.0f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.voice_limit_rampdown.load(), 1.0f);
	}

	SUBCASE("enable_voice_limit_toggle")
	{
		settings.enable_voice_limit.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_voice_limit.load());

		settings.enable_voice_limit.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_voice_limit.load());
	}
}

TEST_CASE("SettingsNotifier_propagation")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	SUBCASE("enable_velocity_modifier_propagation")
	{
		settings.enable_velocity_modifier.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_velocity_modifier.load());
	}

	SUBCASE("enable_resampling_propagation")
	{
		settings.enable_resampling.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_resampling.load());
	}

	SUBCASE("enable_latency_modifier_propagation")
	{
		settings.enable_latency_modifier.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_latency_modifier.load());
	}

	SUBCASE("enable_powermap_propagation")
	{
		settings.enable_powermap.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_powermap.load());
	}

	SUBCASE("powermap_shelf_propagation")
	{
		settings.powermap_shelf.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.powermap_shelf.load());
	}

	SUBCASE("enable_bleed_control_propagation")
	{
		settings.enable_bleed_control.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_bleed_control.load());
	}

	SUBCASE("buffer_size_update")
	{
		settings.buffer_size.store(512);
		settings_notifier.evaluate();
		CHECK_EQ(settings.buffer_size.load(), std::size_t(512));
	}

	SUBCASE("load_status_text_update")
	{
		settings.load_status_text.store("Test message");
		settings_notifier.evaluate();
		CHECK_EQ(settings.load_status_text.load(), std::string("Test message"));
	}

	SUBCASE("drumkit_name_update")
	{
		settings.drumkit_name.store("Test Kit");
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_name.load(), std::string("Test Kit"));
	}

	SUBCASE("drumkit_description_update")
	{
		settings.drumkit_description.store("A test drumkit");
		settings_notifier.evaluate();
		CHECK_EQ(
		    settings.drumkit_description.load(), std::string("A test drumkit"));
	}

	SUBCASE("drumkit_version_update")
	{
		settings.drumkit_version.store("1.0");
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_version.load(), std::string("1.0"));
	}

	SUBCASE("number_of_underruns_update")
	{
		settings.number_of_underruns.store(42);
		settings_notifier.evaluate();
		CHECK_EQ(settings.number_of_underruns.load(), std::size_t(42));
	}

	SUBCASE("disk_cache_enable_update")
	{
		settings.disk_cache_enable.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.disk_cache_enable.load());
	}

	SUBCASE("latency_current_update")
	{
		settings.latency_current.store(5.0f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.latency_current.load(), 5.0f);
	}

	SUBCASE("velocity_modifier_current_update")
	{
		settings.velocity_modifier_current.store(0.8f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.velocity_modifier_current.load(), 0.8f);
	}

	SUBCASE("powermap_input_output_update")
	{
		settings.powermap_input.store(0.5f);
		settings.powermap_output.store(0.6f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.powermap_input.load(), 0.5f);
		CHECK_EQ(settings.powermap_output.load(), 0.6f);
	}
}

TEST_CASE("Diskstreaming_limits_boundary_values")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	constexpr std::size_t min_limit = 1024ull * 1024ull * 32ull;

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::DiskstreamingframeContent disk(&window, settings, settings_notifier);

	SUBCASE("minimum_limit")
	{
		settings.disk_cache_upper_limit.store(min_limit);
		settings_notifier.evaluate();
		CHECK_EQ(settings.disk_cache_upper_limit.load(), min_limit);
	}
}

TEST_CASE("DrumkitframeContent_load_status_drumkit")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::DrumkitframeContent drumkit(
	    &window, settings, settings_notifier, config);

	SUBCASE("drumkit_idle_keeps_blue")
	{
		settings.drumkit_load_status.store(LoadStatus::Idle);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Idle);
	}

	SUBCASE("drumkit_parsing_keeps_blue")
	{
		settings.drumkit_load_status.store(LoadStatus::Parsing);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Parsing);
	}

	SUBCASE("drumkit_loading_keeps_blue")
	{
		settings.drumkit_load_status.store(LoadStatus::Loading);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Loading);
	}

	SUBCASE("drumkit_done_sets_progress_green")
	{
		settings.drumkit_load_status.store(LoadStatus::Done);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);
	}

	SUBCASE("drumkit_error_sets_progress_red")
	{
		settings.drumkit_load_status.store(LoadStatus::Error);
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Error);
	}
}

TEST_CASE("DrumkitframeContent_load_status_midimap")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::DrumkitframeContent drumkit(
	    &window, settings, settings_notifier, config);

	SUBCASE("midimap_idle_resets_progress")
	{
		settings.midimap_load_status.store(LoadStatus::Idle);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Idle);
	}

	SUBCASE("midimap_parsing_sets_progress_1")
	{
		settings.midimap_load_status.store(LoadStatus::Parsing);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Parsing);
	}

	SUBCASE("midimap_loading_sets_progress_1")
	{
		settings.midimap_load_status.store(LoadStatus::Loading);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Loading);
	}

	SUBCASE("midimap_done_sets_progress_green")
	{
		settings.midimap_load_status.store(LoadStatus::Done);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Done);
	}

	SUBCASE("midimap_error_sets_progress_red")
	{
		settings.midimap_load_status.store(LoadStatus::Error);
		settings_notifier.evaluate();
		CHECK_EQ(settings.midimap_load_status.load(), LoadStatus::Error);
	}
}

TEST_CASE("DrumkitframeContent_file_and_progress_notifiers")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::DrumkitframeContent drumkit(
	    &window, settings, settings_notifier, config);

	SUBCASE("drumkit_file_notifier")
	{
		settings.drumkit_file.store("/path/to/kit.xml");
		settings_notifier.evaluate();
		CHECK_EQ(settings.drumkit_file.load(), std::string("/path/to/kit.xml"));
	}

	SUBCASE("midimap_file_notifier")
	{
		settings.midimap_file.store("/path/to/midimap.xml");
		settings_notifier.evaluate();
		CHECK_EQ(
		    settings.midimap_file.load(), std::string("/path/to/midimap.xml"));
	}

	SUBCASE("number_of_files_notifier")
	{
		settings.number_of_files.store(100);
		settings_notifier.evaluate();
		CHECK_EQ(settings.number_of_files.load(), std::size_t(100));
	}

	SUBCASE("number_of_files_loaded_notifier")
	{
		settings.number_of_files_loaded.store(50);
		settings_notifier.evaluate();
		CHECK_EQ(settings.number_of_files_loaded.load(), std::size_t(50));
	}
}

TEST_CASE("PowerWidget_parameter_propagation")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::PowerWidget power(&window, settings, settings_notifier);

	SUBCASE("powermap_shelf_checkbox_false")
	{
		settings.powermap_shelf.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.powermap_shelf.load());
	}

	SUBCASE("powermap_shelf_checkbox_true")
	{
		settings.powermap_shelf.store(true);
		settings_notifier.evaluate();
		CHECK(settings.powermap_shelf.load());
	}

	SUBCASE("enable_powermap_false")
	{
		settings.enable_powermap.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_powermap.load());
	}

	SUBCASE("powermap_fixed_points_propagation")
	{
		settings.powermap_fixed0_x.store(0.0f);
		settings.powermap_fixed0_y.store(0.0f);
		settings.powermap_fixed1_x.store(0.5f);
		settings.powermap_fixed1_y.store(0.5f);
		settings.powermap_fixed2_x.store(1.0f);
		settings.powermap_fixed2_y.store(1.0f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.powermap_fixed0_x.load(), 0.0f);
		CHECK_EQ(settings.powermap_fixed0_y.load(), 0.0f);
		CHECK_EQ(settings.powermap_fixed1_x.load(), 0.5f);
		CHECK_EQ(settings.powermap_fixed1_y.load(), 0.5f);
		CHECK_EQ(settings.powermap_fixed2_x.load(), 1.0f);
		CHECK_EQ(settings.powermap_fixed2_y.load(), 1.0f);
	}

	SUBCASE("powermap_input_output_propagation")
	{
		settings.powermap_input.store(0.3f);
		settings.powermap_output.store(0.4f);
		settings_notifier.evaluate();
		CHECK_EQ(settings.powermap_input.load(), 0.3f);
		CHECK_EQ(settings.powermap_output.load(), 0.4f);
	}
}

TEST_CASE("MainTab_switch_toggles")
{
	if(!std::getenv("DISPLAY"))
	{
		return;
	}

	Settings settings;
	SettingsNotifier settings_notifier{settings};
	GUI::Config config;
	dggui::Window window{nullptr};
	window.setCaption("pluginguitest");
	window.show();
	window.resize(800, 600);

	GUI::MainTab maintab(&window, settings, settings_notifier, config);

	SUBCASE("enable_velocity_modifier_toggle")
	{
		settings.enable_velocity_modifier.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_velocity_modifier.load());

		settings.enable_velocity_modifier.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_velocity_modifier.load());
	}

	SUBCASE("enable_resampling_toggle")
	{
		settings.enable_resampling.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_resampling.load());

		settings.enable_resampling.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_resampling.load());
	}

	SUBCASE("enable_powermap_toggle")
	{
		settings.enable_powermap.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_powermap.load());

		settings.enable_powermap.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_powermap.load());
	}

	SUBCASE("has_bleed_control_toggle")
	{
		settings.has_bleed_control.store(true);
		settings_notifier.evaluate();
		CHECK(settings.has_bleed_control.load());

		settings.has_bleed_control.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.has_bleed_control.load());
	}

	SUBCASE("enable_voice_limit_toggle")
	{
		settings.enable_voice_limit.store(true);
		settings_notifier.evaluate();
		CHECK(settings.enable_voice_limit.load());

		settings.enable_voice_limit.store(false);
		settings_notifier.evaluate();
		CHECK_UNARY(!settings.enable_voice_limit.load());
	}
}