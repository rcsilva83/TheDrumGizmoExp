/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pluginguitest.cc
 *
 *  Tue Apr 14 00:00:00 CEST 2026
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

#include <utility>

#include <dggui/button.h>
#include <dggui/checkbox.h>
#include <dggui/knob.h>
#include <dggui/label.h>
#include <dggui/lineedit.h>
#include <dggui/listbox.h>
#include <dggui/slider.h>

#include <plugingui/bleedcontrolframecontent.h>
#include <plugingui/diskstreamingframecontent.h>
#include <plugingui/drumkitframecontent.h>
#include <plugingui/filebrowser.h>
#include <plugingui/labeledcontrol.h>
#include <plugingui/pluginconfig.h>
#include <plugingui/resamplingframecontent.h>
#include <plugingui/sampleselectionframecontent.h>
#include <plugingui/settings.h>
#include <plugingui/statusframecontent.h>
#include <plugingui/timingframecontent.h>
#include <plugingui/voicelimitframecontent.h>

class MockWidget : public dggui::Widget
{
public:
	MockWidget() : dggui::Widget(nullptr)
	{
		_x = 0;
		_y = 0;
		_width = 800;
		_height = 600;
	}

	void resize(std::size_t width, std::size_t height) override
	{
		_width = width;
		_height = height;
	}

	void move(int x, int y) override
	{
		_x = x;
		_y = y;
	}

	int x() const override
	{
		return _x;
	}
	int y() const override
	{
		return _y;
	}
	std::size_t width() const override
	{
		return _width;
	}
	std::size_t height() const override
	{
		return _height;
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}
};

struct PluginguiFixture
{
	MockWidget parent;
	Settings settings;
	SettingsNotifier settings_notifier;

	PluginguiFixture() : settings_notifier(settings)
	{
	}
};

TEST_CASE_FIXTURE(PluginguiFixture, "LabeledControlTest")
{
	SUBCASE("setValue_without_transformation_function")
	{
		GUI::LabeledControl ctrl(&parent, "Test");
		dggui::Knob knob(&ctrl);
		knob.resize(30, 30);
		ctrl.setControl(&knob);

		ctrl.offset = 10.0f;
		ctrl.scale = 2.0f;
		knob.valueChangedNotifier(5.0f);
	}

	SUBCASE("setValue_with_transformation_function")
	{
		GUI::LabeledControl ctrl(&parent, "Test");
		dggui::Knob knob(&ctrl);
		knob.resize(30, 30);
		ctrl.setControl(&knob);

		auto transform = [](float value, float scale,
		                     float offset) -> std::string
		{ return "transformed: " + std::to_string(value * scale + offset); };
		ctrl.setValueTransformationFunction(transform);

		ctrl.offset = 10.0f;
		ctrl.scale = 2.0f;
		knob.valueChangedNotifier(5.0f);
	}

	SUBCASE("LabeledControl_resize")
	{
		GUI::LabeledControl ctrl(&parent, "Test");
		dggui::Knob knob(&ctrl);
		knob.resize(30, 30);
		ctrl.setControl(&knob);
		ctrl.resize(100, 100);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "StatusframeContentLoadStatusTest")
{
	SUBCASE("updateDrumkitLoadStatus_Idle")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitLoadStatus(LoadStatus::Idle);
	}

	SUBCASE("updateDrumkitLoadStatus_Parsing")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitLoadStatus(LoadStatus::Parsing);
	}

	SUBCASE("updateDrumkitLoadStatus_Loading")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitLoadStatus(LoadStatus::Loading);
	}

	SUBCASE("updateDrumkitLoadStatus_Done")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitLoadStatus(LoadStatus::Done);
	}

	SUBCASE("updateDrumkitLoadStatus_Error")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitLoadStatus(LoadStatus::Error);
	}

	SUBCASE("updateMidimapLoadStatus_Idle")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateMidimapLoadStatus(LoadStatus::Idle);
	}

	SUBCASE("updateMidimapLoadStatus_Parsing")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateMidimapLoadStatus(LoadStatus::Parsing);
	}

	SUBCASE("updateMidimapLoadStatus_Loading")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateMidimapLoadStatus(LoadStatus::Loading);
	}

	SUBCASE("updateMidimapLoadStatus_Done")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateMidimapLoadStatus(LoadStatus::Done);
	}

	SUBCASE("updateMidimapLoadStatus_Error")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateMidimapLoadStatus(LoadStatus::Error);
	}

	SUBCASE("updateContent_multiple_fields")
	{
		GUI::StatusframeContent content(&parent, settings_notifier);
		content.updateDrumkitName("TestKit");
		content.updateDrumkitDescription("Test Description");
		content.updateDrumkitVersion("1.0");
		content.updateBufferSize(1024);
		content.updateNumberOfUnderruns(5);
		content.loadStatusTextChanged("Test message");
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "DrumkitframeContentLoadStatusTest")
{
	SUBCASE("setDrumKitLoadStatus_Idle")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setDrumKitLoadStatus(LoadStatus::Idle);
	}

	SUBCASE("setDrumKitLoadStatus_Parsing")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setDrumKitLoadStatus(LoadStatus::Parsing);
	}

	SUBCASE("setDrumKitLoadStatus_Loading")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setDrumKitLoadStatus(LoadStatus::Loading);
	}

	SUBCASE("setDrumKitLoadStatus_Done")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setDrumKitLoadStatus(LoadStatus::Done);
	}

	SUBCASE("setDrumKitLoadStatus_Error")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setDrumKitLoadStatus(LoadStatus::Error);
	}

	SUBCASE("setMidiMapLoadStatus_Idle")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setMidiMapLoadStatus(LoadStatus::Idle);
	}

	SUBCASE("setMidiMapLoadStatus_Parsing")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setMidiMapLoadStatus(LoadStatus::Parsing);
	}

	SUBCASE("setMidiMapLoadStatus_Loading")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setMidiMapLoadStatus(LoadStatus::Loading);
	}

	SUBCASE("setMidiMapLoadStatus_Done")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setMidiMapLoadStatus(LoadStatus::Done);
	}

	SUBCASE("setMidiMapLoadStatus_Error")
	{
		GUI::Config config;
		GUI::DrumkitframeContent content(
		    &parent, settings, settings_notifier, config);
		content.setMidiMapLoadStatus(LoadStatus::Error);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "DiskstreamingframeContentTest")
{
	SUBCASE("limitSettingsValueChanged_unlimited")
	{
		GUI::DiskstreamingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		constexpr std::size_t max_limit = 1024 * 1024 * 1024 * 4 - 1;
		settings_notifier.disk_cache_upper_limit(max_limit);
	}

	SUBCASE("limitSettingsValueChanged_limited")
	{
		GUI::DiskstreamingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		constexpr std::size_t min_limit = 1024 * 1024 * 32;
		settings_notifier.disk_cache_upper_limit(min_limit);
	}

	SUBCASE("limitSettingsValueChanged_middle")
	{
		GUI::DiskstreamingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		constexpr std::size_t min_limit = 1024 * 1024 * 32;
		constexpr std::size_t max_limit = 1024 * 1024 * 1024 * 4 - 1;
		settings_notifier.disk_cache_upper_limit((min_limit + max_limit) / 2);
	}

	SUBCASE("reloadClicked")
	{
		GUI::DiskstreamingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		content.reloadClicked();
	}

	SUBCASE("reloaded")
	{
		GUI::DiskstreamingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		content.reloaded(0);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "ResamplingframeContentTest")
{
	SUBCASE("updateDrumkitSamplerate_zero")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.drumkit_samplerate(0);
	}

	SUBCASE("updateDrumkitSamplerate_nonzero")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.drumkit_samplerate(44100);
	}

	SUBCASE("updateSessionSamplerate")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.samplerate(48000.0);
	}

	SUBCASE("updateResamplingRecommended_true")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.resampling_recommended(true);
	}

	SUBCASE("updateResamplingRecommended_false")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.resampling_recommended(false);
	}

	SUBCASE("updateResamplingQuality")
	{
		GUI::ResamplingframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.resampling_quality(0.8f);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "VoiceLimitFrameContentTest")
{
	SUBCASE("maxvoicesSettingsValueChanged")
	{
		GUI::VoiceLimitFrameContent content(
		    &parent, settings, settings_notifier);
		content.resize(200, 100);
		settings_notifier.voice_limit_max(20.0f);
	}

	SUBCASE("rampdownSettingsValueChanged")
	{
		GUI::VoiceLimitFrameContent content(
		    &parent, settings, settings_notifier);
		content.resize(200, 100);
		settings_notifier.voice_limit_rampdown(1.0f);
	}

	SUBCASE("voice_limit_max_callback")
	{
		GUI::VoiceLimitFrameContent content(
		    &parent, settings, settings_notifier);
		content.resize(200, 100);
		settings_notifier.voice_limit_max(15.0f);
		settings_notifier.voice_limit_max(25.0f);
		settings_notifier.voice_limit_max(30.0f);
	}

	SUBCASE("voice_limit_rampdown_callback")
	{
		GUI::VoiceLimitFrameContent content(
		    &parent, settings, settings_notifier);
		content.resize(200, 100);
		settings_notifier.voice_limit_rampdown(0.01f);
		settings_notifier.voice_limit_rampdown(1.0f);
		settings_notifier.voice_limit_rampdown(2.0f);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "BleedcontrolframeContentTest")
{
	SUBCASE("resize")
	{
		GUI::BleedcontrolframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
	}

	SUBCASE("resize_small")
	{
		GUI::BleedcontrolframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(100, 50);
	}

	SUBCASE("setEnabled_true")
	{
		GUI::BleedcontrolframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		content.setEnabled(true);
	}

	SUBCASE("setEnabled_false")
	{
		GUI::BleedcontrolframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		content.setEnabled(false);
	}

	SUBCASE("master_bleed_callback")
	{
		GUI::BleedcontrolframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.master_bleed(0.0f);
		settings_notifier.master_bleed(0.5f);
		settings_notifier.master_bleed(1.0f);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "TimingframeContentTest")
{
	SUBCASE("latency_stddev_callback")
	{
		GUI::TimingframeContent content(&parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.latency_stddev(0.0f);
		settings_notifier.latency_stddev(2.0f);
		settings_notifier.latency_stddev(10.0f);
	}

	SUBCASE("latency_regain_callback")
	{
		GUI::TimingframeContent content(&parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.latency_regain(0.0f);
		settings_notifier.latency_regain(0.5f);
		settings_notifier.latency_regain(1.0f);
	}

	SUBCASE("latency_laidback_ms_callback")
	{
		GUI::TimingframeContent content(&parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.latency_laid_back_ms(-50.0f);
		settings_notifier.latency_laid_back_ms(0.0f);
		settings_notifier.latency_laid_back_ms(50.0f);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "SampleselectionframeContentTest")
{
	SUBCASE("sample_selection_f_close_callback")
	{
		GUI::SampleselectionframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.sample_selection_f_close(0.0f);
		settings_notifier.sample_selection_f_close(0.85f);
		settings_notifier.sample_selection_f_close(1.0f);
	}

	SUBCASE("sample_selection_f_diverse_callback")
	{
		GUI::SampleselectionframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.sample_selection_f_diverse(0.0f);
		settings_notifier.sample_selection_f_diverse(0.16f);
		settings_notifier.sample_selection_f_diverse(1.0f);
	}

	SUBCASE("sample_selection_f_random_callback")
	{
		GUI::SampleselectionframeContent content(
		    &parent, settings, settings_notifier);
		content.resize(300, 100);
		settings_notifier.sample_selection_f_random(0.0f);
		settings_notifier.sample_selection_f_random(0.07f);
		settings_notifier.sample_selection_f_random(1.0f);
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "BrowseFileTest")
{
	SUBCASE("resize_zero_width")
	{
		GUI::BrowseFile browse(&parent);
		browse.resize(0, 30);
	}

	SUBCASE("resize_normal")
	{
		GUI::BrowseFile browse(&parent);
		browse.resize(300, 30);
	}

	SUBCASE("resize_small_width")
	{
		GUI::BrowseFile browse(&parent);
		browse.resize(50, 30);
	}

	SUBCASE("getLineEditWidth")
	{
		GUI::BrowseFile browse(&parent);
		browse.resize(300, 30);
		auto w = browse.getLineEditWidth();
		CHECK_GT(w, std::size_t(0));
	}

	SUBCASE("getButtonWidth")
	{
		GUI::BrowseFile browse(&parent);
		browse.resize(300, 30);
		auto w = browse.getButtonWidth();
		CHECK_GT(w, std::size_t(0));
	}
}

TEST_CASE_FIXTURE(PluginguiFixture, "SettingsNotifierEvaluateTest")
{
	SUBCASE("evaluate_all_settings_changed")
	{
		GUI::StatusframeContent status_content(&parent, settings_notifier);
		GUI::ResamplingframeContent resampling_content(
		    &parent, settings, settings_notifier);

		settings.drumkit_load_status.store(LoadStatus::Parsing);
		settings.drumkit_name.store("TestKit");
		settings.drumkit_description.store("Test Description");
		settings.drumkit_version.store("1.0");
		settings.midimap_load_status.store(LoadStatus::Loading);
		settings.buffer_size.store(1024);
		settings.number_of_underruns.store(5);
		settings.load_status_text.store("Test message");
		settings.drumkit_samplerate.store(44100);
		settings.samplerate.store(48000.0);
		settings.resampling_recommended.store(true);
		settings.resampling_quality.store(0.8f);

		settings_notifier.evaluate();
	}
}