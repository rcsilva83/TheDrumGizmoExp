/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            engine.cc
 *
 *  Fri Nov 29 18:09:02 CET 2013
 *  Copyright 2013 Bent Bisballe Nyeng
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

#include <chrono>
#include <thread>
#include <vector>

#include <drumgizmo.h>

#include "drumkit_creator.h"

class AudioOutputEngineDummy : public AudioOutputEngine
{
public:
	bool init(const Channels& channels) override
	{
		return true;
	}

	void setParm(const std::string& parm, const std::string& value) override
	{
		(void)parm;
		(void)value;
	}

	bool start() override
	{
		return true;
	}
	void stop() override
	{
		static_cast<void>(0);
	}

	void pre(size_t nsamples) override
	{
		(void)nsamples;
	}
	void run(int ch, sample_t* samples, size_t nsamples) override
	{
		(void)ch;
		(void)samples;
		(void)nsamples;
	}
	void post(size_t nsamples) override
	{
		(void)nsamples;
	}
	std::size_t getSamplerate() const override
	{
		return 44100;
	}
	bool isFreewheeling() const override
	{
		return true;
	}
};

class AudioOutputEngineBufferDummy : public AudioOutputEngineDummy
{
public:
	void setInternalBufferSize(std::size_t nsamples)
	{
		internal_buffer.resize(nsamples, 0.0f);
	}

	void setFreewheeling(bool value)
	{
		freewheeling = value;
	}

	sample_t* getBuffer(int ch) const override
	{
		if(ch % 2 == 0)
		{
			return const_cast<sample_t*>(internal_buffer.data());
		}

		return nullptr;
	}

	bool isFreewheeling() const override
	{
		return freewheeling;
	}

private:
	bool freewheeling{true};
	mutable std::vector<sample_t> internal_buffer;
};

class AudioInputEngineDummy : public AudioInputEngine
{
public:
	bool init(const Instruments& instruments) override
	{
		return true;
	}

	void setParm(const std::string& parm, const std::string& value) override
	{
		(void)parm;
		(void)value;
	}

	bool start() override
	{
		return true;
	}
	void stop() override
	{
		static_cast<void>(0);
	}

	void pre() override
	{
		static_cast<void>(0);
	}
	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		(void)events;
	}
	void post() override
	{
		static_cast<void>(0);
	}
	void setSampleRate(double sample_rate) override
	{
		(void)sample_rate;
	}
	bool isFreewheeling() const override
	{
		return true;
	}
};

class AudioInputEngineConfigurableDummy : public AudioInputEngineDummy
{
public:
	void setFreewheeling(bool value)
	{
		freewheeling = value;
	}

	bool isFreewheeling() const override
	{
		return freewheeling;
	}

private:
	bool freewheeling{true};
};

class AudioInputEngineStopDummy : public AudioInputEngineDummy
{
public:
	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		if(sent_stop)
		{
			return;
		}

		events.push_back({EventType::Stop, 0, 0, 0.0f});
		sent_stop = true;
	}

private:
	bool sent_stop{false};
};

struct test_engineFixture
{
	DrumkitCreator drumkit_creator;
};

TEST_CASE_FIXTURE(test_engineFixture, "test_engine")
{
	SUBCASE("loading")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(100);

		// Create drumkits
		auto kit1_file = drumkit_creator.createStdKit("kit1");
		auto kit2_file = drumkit_creator.createStdKit("kit2");

		// Switch kits emmidiately without giving the loader time to work:
		for(int i = 0; i < 100; ++i)
		{
			settings.drumkit_file.store(kit1_file);
			settings.drumkit_file.store(kit2_file);
		}

		// Switch kits with small delay giving the loader time to work a little:
		for(int i = 0; i < 100; ++i)
		{
			settings.drumkit_file.store(kit1_file);
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			settings.drumkit_file.store(kit2_file);
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}

		// Switch kits with bigger delay giving the loader time to finish
		for(int i = 0; i < 100; ++i)
		{
			settings.drumkit_file.store(kit1_file);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			settings.drumkit_file.store(kit2_file);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	SUBCASE("setSamplerateQualityClamping")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// Quality at lower boundary (0.0)
		dg.setSamplerate(44100.0f, 0.0f);
		auto latency_q0 = dg.getLatency();

		// Quality below lower boundary should be clamped to 0.0
		dg.setSamplerate(44100.0f, -0.5f);
		auto latency_qneg = dg.getLatency();

		// Quality at upper boundary (1.0)
		dg.setSamplerate(44100.0f, 1.0f);
		auto latency_q1 = dg.getLatency();

		// Quality above upper boundary should be clamped to 1.0
		dg.setSamplerate(44100.0f, 1.5f);
		auto latency_qhigh = dg.getLatency();

		// Out-of-range quality values produce identical latency to their
		// clamped boundary
		CHECK_EQ(latency_q0, latency_qneg);
		CHECK_EQ(latency_q1, latency_qhigh);
		// Higher quality uses a larger filter, so it contributes more latency
		CHECK(latency_q0 < latency_q1);
	}

	SUBCASE("getLatencyIncludesResamplerWhenEnabled")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// Default: resampling enabled, ratio=1.0 (44100→44100), ratio != 0.0
		// getLatency() adds zita[0].inpsize() when enable_resampling && ratio
		// != 0.0
		CHECK(dg.getLatency() > 0u);
	}

	SUBCASE("getLatencyExcludesResamplerWhenDisabled")
	{
		Settings settings;
		settings.enable_resampling.store(false);
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// First run() propagates enable_resampling=false to the engine member
		dg.init();
		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);
		dg.run(0, buf.data(), nsamples);

		// With resampling disabled and default settings (latency modifier off),
		// getLatency() returns 0
		CHECK_EQ(dg.getLatency(), 0u);
	}

	SUBCASE("resamplingRecommendedSetWhenRatioNotOne")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// Initially drumkit_samplerate (44100) == output samplerate (44100):
		// ratio=1.0
		CHECK_UNARY(!settings.resampling_recommended.load());

		// Change output samplerate so ratio = 44100/48000 != 1.0
		dg.setSamplerate(48000.0f);
		CHECK_UNARY(settings.resampling_recommended.load());

		// Restore equal samplerates: ratio=1.0
		dg.setSamplerate(44100.0f);
		CHECK_UNARY(!settings.resampling_recommended.load());
	}

	SUBCASE("runWithRatioNotOne")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		// Set output samplerate different from drumkit samplerate (44100)
		// so ratio = 44100/48000 != 1.0, which selects the resampling branch
		dg.setSamplerate(48000.0f);

		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		// run() should execute the resampling code path without crashing
		auto result = dg.run(0, buf.data(), nsamples);

		// No Stop event was seen, so run() returns true
		CHECK(result);
		// Resampler latency is included (enable_resampling=true, ratio != 0.0)
		CHECK(dg.getLatency() > 0u);
	}

	SUBCASE("runWithRatioOne")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		// Default: output samplerate == drumkit samplerate (44100), ratio=1.0
		// Selects the non-resampling code path
		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto result = dg.run(0, buf.data(), nsamples);

		CHECK(result);
	}

	SUBCASE("runReturnsFalseAfterStopWhenNoActiveEvents")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineStopDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto result = dg.run(0, buf.data(), nsamples);

		CHECK_UNARY(!result);
	}

	SUBCASE("runtimeEnableResamplingToggleAffectsLatency")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		dg.setSamplerate(48000.0f);

		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		CHECK(dg.run(0, buf.data(), nsamples));
		auto latency_enabled = dg.getLatency();
		CHECK(latency_enabled > 0u);

		settings.enable_resampling.store(false);
		CHECK(dg.run(nsamples, buf.data(), nsamples));
		CHECK_EQ(dg.getLatency(), 0u);

		settings.enable_resampling.store(true);
		CHECK(dg.run(2 * nsamples, buf.data(), nsamples));
		CHECK(dg.getLatency() > 0u);
	}

	SUBCASE("runtimeResamplingQualityChangeAffectsLatency")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		settings.resampling_quality.store(0.0f);
		CHECK(dg.run(0, buf.data(), nsamples));
		auto latency_q0 = dg.getLatency();

		settings.resampling_quality.store(1.0f);
		CHECK(dg.run(nsamples, buf.data(), nsamples));
		auto latency_q1 = dg.getLatency();

		CHECK(latency_q1 > latency_q0);
	}

	SUBCASE("runTracksAuditionAndFreewheelBranches")
	{
		Settings settings;
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineConfigurableDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit_file = drumkit_creator.createStdKit("audition_kit");
		settings.drumkit_file.store(kit_file);
		for(int i = 0; i < 50; ++i)
		{
			dg.run(i * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.8f);
		settings.audition_counter.store(1);

		ie.setFreewheeling(false);
		oe.setFreewheeling(true);
		CHECK(dg.run(0, buf.data(), nsamples));

		settings.audition_instrument.store("missing-instrument");
		settings.audition_counter.store(2);

		ie.setFreewheeling(true);
		oe.setFreewheeling(false);
		CHECK(dg.run(nsamples, buf.data(), nsamples));
	}

	SUBCASE("runWithWideKitCoversChannelLimitAndOutputPaths")
	{
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("wide.wav", 64, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles;
		for(std::size_t i = 0; i < 17; ++i)
		{
			audiofiles.push_back({&wav_infos.front(), i + 1});
		}

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke1", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"wide_instr", "wide_instr.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "wide_kit", 17, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);
		for(int i = 0; i < 50; ++i)
		{
			dg.run(i * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		CHECK(dg.run(0, buf.data(), nsamples));

		dg.setSamplerate(48000.0f);
		CHECK(dg.run(nsamples, buf.data(), nsamples));
		CHECK(dg.run(2 * nsamples, buf.data(), nsamples));
	}
}
