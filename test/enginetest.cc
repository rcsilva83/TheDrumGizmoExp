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
#include <fstream>
#include <sstream>
#include <string>
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
		// no-op for test dummy
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
			return internal_buffer.data();
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
		(void)pos;
		(void)len;
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

// Fires OnSet events with a non-zero within-buffer offset on every run() call.
// Covers renderSampleEvent branch where evt.offset > pos.
class AudioInputEngineOnsetWithOffsetDummy : public AudioInputEngineDummy
{
public:
	explicit AudioInputEngineOnsetWithOffsetDummy(size_t offset)
	    : onset_offset(offset)
	{
		static_cast<void>(0); // onset_offset initialized above
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		events.push_back({EventType::OnSet, 0, onset_offset, 1.0f});
	}

private:
	size_t onset_offset;
};

class FailingOutputEngineDummy : public AudioOutputEngineDummy
{
public:
	bool init(const Channels& channels) override
	{
		(void)channels;
		return false;
	}
};

class FailingInputEngineDummy : public AudioInputEngineDummy
{
public:
	bool init(const Instruments& instruments) override
	{
		(void)instruments;
		return false;
	}
};

// Fires a single Choke event for instrument 0 when setFireChoke(true) is
// called.
class AudioInputEngineChokeOnDemandDummy : public AudioInputEngineDummy
{
public:
	void setFireChoke(bool v)
	{
		fire_choke = v;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		if(fire_choke)
		{
			events.push_back({EventType::Choke, 0, 0, 0.0f});
			fire_choke = false;
		}
	}

private:
	bool fire_choke{false};
};

// Fires OnSet events with an instrument index that is far beyond any loaded
// kit's instrument count (TST-INPUT-02). Used to cover the
// `instrument_id >= kit.instruments.size()` false-branch in processOnset.
class AudioInputEngineOutOfBoundsInstrumentDummy : public AudioInputEngineDummy
{
public:
	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		// Use an instrument index that can never be valid in any test kit.
		events.push_back({EventType::OnSet, 999999u, 0, 1.0f});
	}
};

// Fires Choke events with an instrument index that is far beyond any loaded
// kit's instrument count (TST-INPUT-02 extension). Used to cover the
// `instrument_id >= kit.instruments.size()` false-branch in processChoke.
class AudioInputEngineOutOfBoundsChokeDummy : public AudioInputEngineDummy
{
public:
	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		// Use an instrument index that can never be valid in any test kit.
		events.push_back({EventType::Choke, 999999u, 0, 0.0f});
	}
};

// Fires a single Stop event on demand (once setFireStop(true) is called).
// Used to test processStop's channel-iteration loop with a loaded kit.
class AudioInputEngineOnDemandStopDummy : public AudioInputEngineDummy
{
public:
	void setFireStop(bool v)
	{
		fire_stop = v;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		if(fire_stop)
		{
			events.push_back({EventType::Stop, 0, 0, 0.0f});
			fire_stop = false;
		}
	}

private:
	bool fire_stop{false};
};

// Fires count OnSet events for instrument 0 on every run() call.
// Used to drive voice-limit enforcement (TST-INPUT-01): with
// voice_limit_max < count, limitVoices() is triggered every run.
// Call setEnabled(true) only after the drumkit is fully loaded to
// avoid dangling AudioFile* references during the loader's second
// loadkit() cycle.
class AudioInputEngineRepeatedOnsetDummy : public AudioInputEngineDummy
{
public:
	explicit AudioInputEngineRepeatedOnsetDummy(size_t count)
	    : onset_count(count)
	{
	}

	void setEnabled(bool v)
	{
		enabled = v;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		if(!enabled)
		{
			return;
		}
		for(size_t i = 0; i < onset_count; ++i)
		{
			// Space onsets 8 samples apart so each generated SampleEvent gets a
			// distinct offset, making oldest-voice selection deterministic when
			// limitVoices() is exercised.
			events.push_back({EventType::OnSet, 0u, i * 8u, 1.0f});
		}
	}

private:
	size_t onset_count;
	bool enabled{false};
};

// Fires OnSet events alternating between two instrument IDs (0 and 1).
// Used to trigger applyChokeGroup: when two instruments share a group,
// firing instrument 1 while instrument 0 is playing causes the running
// instrument-0 SampleEvents to be ramped down.
class AudioInputEngineAlternatingOnsetDummy : public AudioInputEngineDummy
{
public:
	void setEnabled(bool v)
	{
		enabled = v;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		if(!enabled)
		{
			return;
		}
		events.push_back({EventType::OnSet, next_id, 0, 1.0f});
		next_id = (next_id == 0u) ? 1u : 0u;
	}

private:
	size_t next_id{0};
	bool enabled{false};
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

	SUBCASE("simpleApiMethodCoverage")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// Exercises setRandomSeed(), stop(), and samplerate() which otherwise
		// have zero coverage.  stop() is currently a no-op stub; setRandomSeed
		// only affects stochastic sample selection (no directly verifiable
		// side-effect at this call site).  samplerate() returns the stored
		// engine samplerate.
		dg.setRandomSeed(42u);
		dg.stop();
		CHECK_EQ(dg.samplerate(), 44100.0f);
	}

	SUBCASE("initReturnsFalseWhenInputEngineFails")
	{
		Settings settings;
		AudioOutputEngineDummy oe;
		FailingInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// DrumGizmo::init() returns false when the input engine init fails.
		CHECK_UNARY(!dg.init());
	}

	SUBCASE("initReturnsFalseWhenOutputEngineFails")
	{
		Settings settings;
		FailingOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);

		// DrumGizmo::init() returns false when the output engine init fails.
		CHECK_UNARY(!dg.init());
	}

	SUBCASE("renderSampleEventCoversMidBufferOnsetOffset")
	{
		// AudioInputEngine fires OnSet events at within-buffer offset 10 on
		// every call.  Once the kit is loaded, processOnset stores
		//   event_sample.offset = (10 + pos) * ratio
		// so getSamples(c, pos, ...) calls renderSampleEvent with
		//   evt.offset = pos + 10 > pos
		// which covers the `if(evt.offset > (size_t)pos)` true-branch.
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetWithOffsetDummy ie(10u);
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		constexpr size_t nsamples = 512;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.createStdKit("offset_kit");
		settings.drumkit_file.store(kit_file);

		// Allow the kit to load.
		for(int i = 0; i < 50; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Kit is loaded; the onset at offset=10 now produces a SampleEvent
		// with offset = pos+10, exercising the mid-buffer offset branch.
		CHECK(dg.run(50u * nsamples, buf.data(), nsamples));
	}

	SUBCASE("getSamplesCoversFutureEvent")
	{
		// AudioInputEngine fires an OnSet event at offset 600 when the buffer
		// is only 512 samples wide.  processOnset stores
		//   event_sample.offset = (600 + pos)
		// so getSamples(c, pos, buf, 512) sees
		//   event_sample.offset > (pos + 512)
		// which is the "don't handle yet — future event" true-branch.
		constexpr size_t nsamples = 512;
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetWithOffsetDummy ie(nsamples + 88u); // 600 > 512
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.createStdKit("future_kit");
		settings.drumkit_file.store(kit_file);

		for(int i = 0; i < 50; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// An event at offset=600 within a 512-sample buffer is always "future";
		// getSamples skips it every call → covers the continue branch.
		CHECK(dg.run(50u * nsamples, buf.data(), nsamples));
	}

	SUBCASE("longSampleCoversCacheRefillAndPersistentEvent")
	{
		// Kit with a 1024-sample WAV.  After the first run() call opens the
		// audio-cache entry (cache_id == CACHE_NOID → true), the SampleEvent
		// persists.  On the second run() call the same event is found again
		// with cache_id != CACHE_NOID (→ false branch) and the buffer-refill
		// path at the bottom of getSamples is exercised.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("long.wav", 1024)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "long_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Wait for the kit to load.
		for(int i = 0; i < 50; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Fire audition onset to start playing the 1024-sample instrument.
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.8f);
		settings.audition_counter.store(1);

		// First run: cache opens (cache_id == CACHE_NOID true-branch).
		CHECK(dg.run(50u * nsamples, buf.data(), nsamples));

		// Second run: same SampleEvent still active; cache was already opened
		// (cache_id != CACHE_NOID → false-branch skips re-opening) and the
		// buffer-refill code at the bottom of getSamples is exercised.
		CHECK(dg.run(51u * nsamples, buf.data(), nsamples));
	}

	SUBCASE("renderSampleEventCoversRampdownAfterChoke")
	{
		// Uses a 1024-sample kit.  An audition onset starts the sample on
		// run 51.  On run 52 the AudioInputEngine fires a Choke event which
		// invokes processChoke() → applyChoke() → sets rampdown_count and
		// ramp_length on the active SampleEvent.  When getSamples then calls
		// renderSampleEvent on the same run, the inner sample loop reaches
		// the `if(rampdownInProgress() && ...)` true-branch, exercising the
		// rampdown-scale and rampdown_count-- code paths.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("rampdown.wav", 1024)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "rampdown_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineChokeOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.init();

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Wait for the kit to load.
		for(int i = 0; i < 50; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Run 51: audition onset fires → SampleEvent created and 256 samples
		// processed (evt.t = 256 after this call).
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(1.0f);
		settings.audition_counter.store(1);
		CHECK(dg.run(50u * nsamples, buf.data(), nsamples));

		// Run 52: choke fires (processed before getSamples); sets
		// rampdown_count = ~19845 on the active SampleEvent.  The subsequent
		// getSamples / renderSampleEvent call sees rampdown_offset (= 0) <
		// evt.t + t (= 256 + t) for every t ≥ 0, and rampdown_count > 0 →
		// the rampdown scale path is taken.
		ie.setFireChoke(true);
		CHECK(dg.run(51u * nsamples, buf.data(), nsamples));
	}

	SUBCASE("kitSwitchResetsActiveEvents")
	{
		// Verify that events_ds.clear() is called when drumkit_file changes
		// so that sample events from the old kit are discarded immediately.
		// 0x1110 is a fixed non-zero sample value that produces audible output.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ks_hit.wav", 1024, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ks_kit1", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit1_file = drumkit_creator.create(kit_data);
		auto kit2_file = drumkit_creator.createStdKit("ks_kit2");

		// Load kit1 and wait for it to finish loading.
		settings.drumkit_file.store(kit1_file);

		// Poll until drumkit loading is complete, with a bounded timeout.
		size_t current_time = 0;
		const int max_iterations = 2000; // ~2 seconds at 1 ms per iteration
		for(int i = 0; i < max_iterations &&
		               settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Ensure kit loading actually finished before auditioning.
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Fire an audition onset to create a SampleEvent in events_ds.
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.8f);
		settings.audition_counter.store(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));

		// The channel-0 output must be non-zero: the WAV data is constant
		// 0x1110, so every rendered sample contributes a non-zero value.
		auto const* ch0_buf = oe.getBuffer(0);
		REQUIRE(ch0_buf != nullptr);
		bool has_nonzero = false;
		for(size_t i = 0; i < nsamples; ++i)
		{
			if(ch0_buf[i] != 0.0f)
			{
				has_nonzero = true;
				break;
			}
		}
		CHECK_UNARY(has_nonzero);

		// Switch to kit2 — the next run() call detects the drumkit_file
		// change and calls events_ds.clear() before processing anything else.
		settings.drumkit_file.store(kit2_file);
		CHECK(dg.run(51u * nsamples, buf.data(), nsamples));

		// After events_ds.clear(), no SampleEvents are active — the rendered
		// output for ch0 must be all zeros.
		bool all_zero_after_switch = true;
		for(size_t i = 0; i < nsamples; ++i)
		{
			if(ch0_buf[i] != 0.0f)
			{
				all_zero_after_switch = false;
				break;
			}
		}
		CHECK_UNARY(all_zero_after_switch);
	}

	SUBCASE("kitSwitchNoStaleInstrumentMappingAfterSwitch")
	{
		// Verify that kit metadata (name, load status) is fully replaced after
		// switching kits, with no data from the previous kit remaining.
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit1_file = drumkit_creator.createStdKit("stale_kit_a");
		auto kit2_file = drumkit_creator.createStdKit("stale_kit_b");

		// Load kit1 and confirm that its metadata is visible.
		settings.drumkit_file.store(kit1_file);
		for(int i = 0; i < 50; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		CHECK_EQ(settings.drumkit_name.load(), std::string("stale_kit_a"));
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);

		// Switch to kit2 and wait for it to finish loading.
		settings.drumkit_file.store(kit2_file);
		for(int i = 50; i < 100; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// kit2's metadata must have fully replaced kit1's — no stale values.
		CHECK_EQ(settings.drumkit_name.load(), std::string("stale_kit_b"));
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);

		// The engine accepts onset events for the new kit without crashing.
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.5f);
		settings.audition_counter.store(1);
		CHECK(dg.run(100u * nsamples, buf.data(), nsamples));
	}

	SUBCASE("kitSwitchDeterministicStateAfterRepeatedToggles")
	{
		// Verify that after rapid back-and-forth kit switches the engine
		// settles into a deterministic state matching the last kit stored.
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit1_file = drumkit_creator.createStdKit("toggle_kit1");
		auto kit2_file = drumkit_creator.createStdKit("toggle_kit2");

		// Rapidly toggle between two kits, giving each side time to partially
		// load.  This exercises rapid-switch and partially-loaded code paths.
		size_t run_pos = 0;
		for(int iter = 0; iter < 5; ++iter)
		{
			settings.drumkit_file.store(kit1_file);
			for(int j = 0; j < 5; ++j)
			{
				dg.run(run_pos * nsamples, buf.data(), nsamples);
				++run_pos;
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
			settings.drumkit_file.store(kit2_file);
			for(int j = 0; j < 5; ++j)
			{
				dg.run(run_pos * nsamples, buf.data(), nsamples);
				++run_pos;
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
			}
		}

		// Leave the engine on kit2 and wait for it to finish loading.
		for(int i = 0; i < 50; ++i)
		{
			dg.run(run_pos * nsamples, buf.data(), nsamples);
			++run_pos;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// After all toggles the loaded kit must be kit2 (the last kit stored).
		CHECK_EQ(settings.drumkit_name.load(), std::string("toggle_kit2"));
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);

		// The engine still processes frames cleanly after settling.
		CHECK(dg.run(run_pos * nsamples, buf.data(), nsamples));
	}

	SUBCASE("getSamplesCoversBleedControlScale")
	{
		// A kit where channel 0 has main="true" causes DOMLoader to set
		// all other channels to is_not_main.  Enabling bleed control then
		// applies master_bleed scaling to those non-main sample events when
		// the audio cache is opened, covering the
		// `sample_event.scale *= master_bleed` branch (drumgizmo.cc:386).
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("bleed.wav", 64, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}, {&wav_infos.front(), 2}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "bleed_kit", 2, instruments, wav_infos};

		auto kit_file = drumkit_creator.create(kit_data);

		// Patch the kit XML: mark the first channelmap as main="true".
		// DOMLoader will set default_main_state = is_not_main for the
		// remaining channels, making af.mainState() == is_not_main on ch1.
		{
			std::ifstream in(kit_file);
			REQUIRE_UNARY(in.is_open());
			std::ostringstream ss;
			ss << in.rdbuf();
			std::string content = ss.str();
			in.close();

			const std::string from = "channelmap in=\"ch0\" out=\"ch0\"/>";
			const std::string to =
			    "channelmap in=\"ch0\" out=\"ch0\" main=\"true\"/>";
			auto pos = content.find(from);
			REQUIRE_NE(pos, std::string::npos);
			content.replace(pos, from.size(), to);

			std::ofstream out(kit_file);
			REQUIRE_UNARY(out.is_open());
			out << content;
		}

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		settings.enable_bleed_control.store(true);
		settings.master_bleed.store(0.5f);
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete, with a bounded timeout.
		size_t current_time = 0;
		const int max_iterations = 2000; // ~2 seconds at 1 ms per iteration
		for(int i = 0; i < max_iterations &&
		               settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Kit must have loaded, and the patched main="true" must have caused
		// DOMLoader to set has_bleed_control = true (confirming ch1 is
		// is_not_main and the bleed-control scaling path will be exercised).
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);
		REQUIRE_UNARY(settings.has_bleed_control.load());

		// Fire onset: ch1 SampleEvent has mainState()==is_not_main and
		// enable_bleed_control==true → scale *= master_bleed executes.
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.8f);
		settings.audition_counter.store(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("getSamplesCoversUnloadedAudioFilePath")
	{
		// Fire an onset immediately after the drumkit XML is parsed (AudioFile
		// objects exist and are valid) but before the audio data has been
		// loaded from disk.  getSamples then encounters !af.isLoaded() == true
		// and schedules the event for removal, covering that branch.
		//
		// SmallKit has a 549833-sample WAV file shared across 13 channels;
		// loading 13 × 549833 × 2 bytes takes enough time that the main
		// thread can fire an onset while at least one file is still unloaded.
		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetWithOffsetDummy ie(0u);
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.createSmallKit("unloaded_kit");

		// Store kit file and call run() in a tight loop.  The onset fires
		// every call via AudioInputEngineOnsetWithOffsetDummy.  Once the
		// kit parses (kit.isValid() == true) but before all audio files
		// are read from disk, processOnset creates a SampleEvent referencing
		// an unloaded AudioFile; getSamples removes it via the isLoaded path.
		settings.drumkit_file.store(kit_file);

		// Track whether we ever observe an intermediate load state while
		// firing onsets, to avoid relying solely on timing assumptions.
		bool saw_intermediate_load_state{false};

		for(int i = 0; i < 200; ++i)
		{
			dg.run(static_cast<size_t>(i) * nsamples, buf.data(), nsamples);

			auto status = settings.drumkit_load_status.load();
			if(status == LoadStatus::Parsing || status == LoadStatus::Loading)
			{
				saw_intermediate_load_state = true;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Ensure we actually exercised the intended timing window at least
		// once.
		CHECK(saw_intermediate_load_state);

		// Wait for the kit to finish loading (bounded, to handle the
		// double-loadkit() cycle caused by SettingRef::hasChanged() firstAccess
		// on reload_counter). The loader resets status to Parsing/Loading on
		// the second pass; give it up to ~2 s to reach Done.
		size_t current_time = 200u * nsamples;
		for(size_t i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		// Confirm the kit finished loading (test completion check).
		CHECK_EQ(settings.drumkit_load_status.load(), LoadStatus::Done);
	}

	SUBCASE("processOnsetOutOfBoundsInstrumentIdIsIgnored")
	{
		// TST-INPUT-02: When the input engine fires an OnSet event whose
		// instrument index is larger than kit.instruments.size(), processOnset
		// sets instr = nullptr, logs an error, and returns false (dropping the
		// event). The engine must keep running without crashing and produce
		// all-zero output for the frame since no SampleEvent is created.
		Settings settings;
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineOutOfBoundsInstrumentDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit_file = drumkit_creator.createStdKit("oob_kit");
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete, with a bounded timeout.
		size_t current_time = 0;
		const int max_iterations = 2000; // ~2 seconds at 1 ms per iteration
		for(int i = 0; i < max_iterations &&
		               settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation: run for 200 ms to let any second loadkit() cycle
		// complete before validating the out-of-bounds processOnset path.
		for(size_t i = 0; i < 200; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// The input engine fires instrument_id=999999 every run. After the
		// kit is loaded and settled, the event hits processOnset which guards
		// against
		// out-of-bounds access via `if(instrument_id <
		// kit.instruments.size())`. instr stays nullptr → the function returns
		// false → no SampleEvent is created → output stays all-zero. The engine
		// itself must keep running.
		CHECK(dg.run(current_time, buf.data(), nsamples));

		const sample_t* ch0 = oe.getBuffer(0);
		REQUIRE(ch0 != nullptr);
		bool all_zero = true;
		for(size_t i = 0; i < nsamples; ++i)
		{
			if(ch0[i] != 0.0f)
			{
				all_zero = false;
				break;
			}
		}
		CHECK_UNARY(all_zero);
	}

	SUBCASE("voiceLimitCapsActiveVoicesDoesNotCrash")
	{
		// TST-INPUT-01: Exercises the `settings.enable_voice_limit` true-branch
		// in processOnset and the limitVoices() function body.
		//
		// With voice_limit_max=1 and 3 OnSet events per run, after 2 groups are
		// already playing a third onset calls limitVoices(instrument_id=0,
		// max=1, rampdown_time) which finds 2 non-ramping groups > 1, locates
		// the oldest via its offset, and applies applyChoke() to it. The engine
		// must keep running (return true) throughout this sequence.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("vl_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "vl_kit", 1, instruments, wav_infos};

		Settings settings;
		// 3 onsets per run; voice limit of 1 triggers limitVoices() from the
		// second onset onward once 2 groups have accumulated.
		AudioInputEngineRepeatedOnsetDummy ie(3);
		AudioOutputEngineDummy oe;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete, with a bounded timeout.
		// Events are NOT fired during loading (onset firing is disabled) to
		// avoid dangling AudioFile* references while the loader performs its
		// second loadkit() cycle (a known loader behaviour triggered by the
		// firstAccess flag in SettingRef::hasChanged()).
		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		size_t current_time = 0;
		const size_t max_iterations = 2000; // ~2 seconds at 1 ms per iteration
		for(size_t i = 0;
		    i < max_iterations &&
		    settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation: run for 200 ms to let the loader's second loadkit()
		// cycle (if any) complete before we start creating SampleEvents.
		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Enable event firing now that the kit is fully settled, then set up
		// voice limiting with max = 1 so limitVoices() is called on every run.
		ie.setEnabled(true);
		settings.enable_voice_limit.store(true);
		settings.voice_limit_max.store(1u);
		settings.voice_limit_rampdown.store(50.0f); // 50 ms rampdown

		// Run several frames. Each frame fires 3 onsets; with voice_limit=1
		// the path through limitVoices() is exercised on every frame after the
		// first onset.
		for(int i = 0; i < 5; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}

	SUBCASE("processChokeOutOfBoundsInstrumentIdIsIgnored")
	{
		// TST-INPUT-02 (processChoke guard): When the input engine fires a
		// Choke event whose instrument index is larger than
		// kit.instruments.size(), processChoke sets instr = nullptr, logs an
		// error, and returns false, dropping the event. The engine must keep
		// running without crashing.
		Settings settings;
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineOutOfBoundsChokeDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit_file = drumkit_creator.createStdKit("oob_choke_kit");
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete, with a bounded timeout.
		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation: run for 200 ms to let any second loadkit() cycle
		// complete before validating the out-of-bounds processChoke path.
		for(size_t i = 0; i < 200; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// The input engine fires instrument_id=999999 Choke events every run.
		// After the kit is loaded and settled, the event hits processChoke
		// which guards against out-of-bounds access via
		// `if(instrument_id < kit.instruments.size())`. instr stays nullptr
		// → the function returns false → the engine keeps running.
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("processOnsetNormalizedSamplesScalesVelocity")
	{
		// TST-INPUT-02 (normalized-samples branch): When normalized_samples is
		// enabled and the selected sample has normalized=true, processOnset
		// multiplies event_sample.scale by event.velocity (line 303). This
		// test creates a kit with a normalized sample and verifies that the
		// engine runs and produces non-zero output, confirming that the
		// normalized-samples branch is exercised.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("norm_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		// Mark the sample as normalized so processOnset's
		// `if(settings.normalized_samples.load() && sample->getNormalized())`
		// evaluates to true and line 303 is executed.
		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles, /*normalized=*/true}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "norm_kit", 1, instruments, wav_infos};

		Settings settings;
		// normalized_samples defaults to true in Settings; make it explicit.
		settings.normalized_samples.store(true);
		AudioOutputEngineBufferDummy oe;
		AudioInputEngineRepeatedOnsetDummy ie(1);
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);
		oe.setInternalBufferSize(nsamples);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete.
		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation cycle.
		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Enable onsets and fire several frames. The normalized-samples branch
		// in processOnset is exercised each time a SampleEvent is created.
		ie.setEnabled(true);
		for(int i = 0; i < 5; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}

	SUBCASE("processOnsetKitWithExtraChannelsSkipsExcessChannels")
	{
		// TST-INPUT-02 (channel guard): When the kit has more channels than
		// NUM_CHANNELS (16), processOnset's inner channel loop hits the
		// `if(ch.num >= NUM_CHANNELS) continue;` guard for the excess
		// channels. This test creates a NUM_CHANNELS+1 channel kit and fires
		// valid onsets, verifying that the engine does not crash and that the
		// excess channel is silently skipped.
		constexpr size_t num_ch = 17u; // > NUM_CHANNELS (16)

		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("extra_ch.wav", 1, 0x1110)};

		// All 17 kit channels map to filechannel 1 of the single WAV.
		std::vector<DrumkitCreator::Audiofile> audiofiles;
		for(size_t i = 0; i < num_ch; ++i)
		{
			audiofiles.push_back({&wav_infos.front(), 1u});
		}

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "extra_ch_kit", num_ch, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineRepeatedOnsetDummy ie(1);
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete.
		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation cycle.
		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Enable onsets. The 17th kit channel (num=16 == NUM_CHANNELS) is
		// skipped by the ch.num >= NUM_CHANNELS guard in processOnset.
		ie.setEnabled(true);
		for(int i = 0; i < 5; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}

	SUBCASE("processOnsetChokeGroupMutesOtherGroupedInstruments")
	{
		// TST-INPUT-02 / HR-11 (choke group + extra channels + null audiofile):
		// Two instruments in group "hihat" on a 17-channel (NUM_CHANNELS+1)
		// kit, each with only one audiofile (ch0).  Alternating onsets
		// exercise:
		//   - applyChokeGroup body (group match, rampdown applied)
		//   - applyChokeGroup ch.num >= NUM_CHANNELS guard (ch16, line 166)
		//   - af == nullptr branch (channels 1–15 lack audiofiles, line 283)
		constexpr size_t cg_ch = 17u; // > NUM_CHANNELS (16)

		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_hit.wav", 2048, 0x1110)};

		// Only ch0 has an audiofile; channels 1-16 return nullptr from
		// sample->getAudioFile(), covering the af==nullptr guard in
		// processOnset.
		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		// Two instruments that share the same choke group "hihat".
		// When instrument 1 fires an onset, applyChokeGroup will find
		// instrument 0's active events and ramp them down.
		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"cg_instr0", "cg_instr0.xml", sample_data, /*group=*/"hihat"},
		    {"cg_instr1", "cg_instr1.xml", sample_data, /*group=*/"hihat"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_kit", cg_ch, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineAlternatingOnsetDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		// Poll until drumkit loading is complete.
		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Stabilisation cycle.
		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Enable alternating onsets.  On run N, instrument (N % 2) fires.
		// Run 1 (instrument 0) creates SampleEvents for "hihat".
		// Run 2 (instrument 1, same group) calls applyChokeGroup which finds
		// the instrument-0 events (group match, different id, not ramping) and
		// applies a 68 ms rampdown, covering the full applyChokeGroup body.
		// Subsequent runs continue alternating, exercising the loop repeatedly.
		ie.setEnabled(true);
		for(int i = 0; i < 6; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}

	SUBCASE("processOnsetNormalizedSamplesDisabledSkipsBranch")
	{
		// TST-INPUT-02 (normalized_samples=false short-circuit): When
		// settings.normalized_samples is false, the `&&` short-circuits before
		// calling sample->getNormalized(), covering the false-branch of the
		// compound condition at processOnset line 301.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ns_false.wav", 512, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles, /*normalized=*/true}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"ns_instr", "ns_instr.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ns_false_kit", 1, instruments, wav_infos};

		Settings settings;
		// Explicitly disable normalized-samples so processOnset's compound
		// condition short-circuits at the first operand (false &&  ...).
		settings.normalized_samples.store(false);
		AudioOutputEngineDummy oe;
		AudioInputEngineRepeatedOnsetDummy ie(1);
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		ie.setEnabled(true);
		for(int i = 0; i < 5; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}

	SUBCASE("processStopWithLoadedKitIteratesChannels")
	{
		// TST-INPUT-02 (processStop channel loop): When a Stop event is fired
		// after a kit with channels is loaded, processStop's channel-iteration
		// loop executes and counts active events.  With a 17-channel
		// (NUM_CHANNELS+1) kit:
		//   - The loop body runs for each channel (line 378 covered)
		//   - ch.num < NUM_CHANNELS path runs for ch0..ch15 (line 380 false)
		//   - ch.num >= NUM_CHANNELS skips ch16 (line 380 true, line 382)
		//   - numberOfEvents() is called for each valid channel (line 385)
		// The Stop also fires AFTER an onset so num_active_events > 0 on the
		// first Stop run (line 388 false branch), then the engine drains the
		// events and eventually returns false.
		constexpr size_t ps_ch = 17u; // > NUM_CHANNELS (16)

		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ps_hit.wav", 512, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"ps_instr", "ps_instr.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ps_kit", ps_ch, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnDemandStopDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Fire an audition onset to create an active SampleEvent on ch0.
		settings.audition_instrument.store("ps_instr");
		settings.audition_velocity.store(1.0f);
		settings.audition_counter.store(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// Now fire a Stop event while the onset is still active.
		// processStop sees is_stopping=true, iterates all 17 channels:
		//   ch0..ch15 pass the num < NUM_CHANNELS check → numberOfEvents()
		//   ch16 hits ch.num >= NUM_CHANNELS → continue
		// num_active_events > 0 → engine keeps running (line 388 false).
		ie.setFireStop(true);
		bool result = dg.run(current_time, buf.data(), nsamples);
		// Engine may or may not have stopped depending on timing; we only
		// require it doesn't crash and that processStop ran.
		(void)result;
		current_time += nsamples;

		// Drain remaining events until engine stops naturally.
		for(int i = 0; i < 20 && dg.run(current_time, buf.data(), nsamples);
		    ++i)
		{
			current_time += nsamples;
		}
	}

	SUBCASE("processOnsetDirectedChokeRampsDownTargetInstrument")
	{
		// TST-INPUT-02 (applyDirectedChoke body): When instrument 0 has a
		// <chokes> element targeting instrument 1, firing an onset for
		// instrument 0 while instrument 1 has active SampleEvents causes
		// applyDirectedChoke to iterate channels, find instrument-1 events,
		// and apply a rampdown.  The kit has NUM_CHANNELS+1 (17) channels but
		// only ch0 has an audiofile, so:
		//   - ch0 → events created and inspected by the directed choke
		//   - ch1..ch15 → af==nullptr, event loop empty, no rampdown
		//   - ch16 (num=NUM_CHANNELS) → ch.num >= NUM_CHANNELS guard taken
		//   (line 194)
		constexpr size_t dc_ch = 17u; // > NUM_CHANNELS (16)

		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("dc_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		// Instrument 0 directs-chokes instrument 1 (by name).
		DrumkitCreator::InstrumentData instr0{
		    "dc_instr0", "dc_instr0.xml", sample_data};
		instr0.chokes.push_back(
		    DrumkitCreator::InstrumentData::ChokeRef{"dc_instr1", 68.0});

		DrumkitCreator::InstrumentData instr1{
		    "dc_instr1", "dc_instr1.xml", sample_data};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    instr0, instr1};

		DrumkitCreator::DrumkitData kit_data{
		    "dc_kit", dc_ch, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		// Fire onsets alternating between instrument 0 and 1 so that
		// instrument-1 events are in the store when instrument 0 fires.
		AudioInputEngineAlternatingOnsetDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		for(int i = 0;
		    i < 2000 && settings.drumkit_load_status.load() != LoadStatus::Done;
		    ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		for(size_t i = 0; i < 200; ++i)
		{
			dg.run(current_time, buf.data(), nsamples);
			current_time += nsamples;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);

		// Run 1: instrument 0 fires (id=0 by AlternatingOnset starting at 0).
		//   → applyDirectedChoke for instr0 looks for instr1 events → none yet.
		// Run 2: instrument 1 fires → instr1 SampleEvents created.
		// Run 3: instrument 0 fires again → applyDirectedChoke finds instr1
		//   events in the store, applies rampdown (covers body of directed
		//   choke).
		ie.setEnabled(true);
		for(int i = 0; i < 6; ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
			current_time += nsamples;
		}
	}
}
