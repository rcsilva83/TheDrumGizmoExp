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

// Cycles through phases to exercise the !non_ramping.size() early-return
// branch in limitVoices() when all active voice groups are already ramping.
//   Phase::Idle         – fires no events (used during kit loading).
//   Phase::BuildGroups  – fires 3 OnSet events; with voice_limit_max=1 the
//                         third onset calls limitVoices() which chokes the
//                         oldest group.
//   Phase::ChokeAll     – fires a Choke event so processChoke() applies
//                         rampdown to all remaining non-ramping groups.
//   Phase::TriggerOnset – fires 1 OnSet; limitVoices sees all groups ramping
//                         → !non_ramping.size() TRUE → returns early.
class AudioInputEngineAllRampingDummy : public AudioInputEngineDummy
{
public:
	enum class Phase
	{
		Idle,
		BuildGroups,
		ChokeAll,
		TriggerOnset
	};

	void setPhase(Phase p)
	{
		phase = p;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		switch(phase)
		{
		case Phase::Idle:
			break;
		case Phase::BuildGroups:
			// Three onsets with distinct offsets so limitVoices() can
			// identify the oldest group deterministically.
			for(size_t i = 0; i < 3; ++i)
			{
				events.push_back({EventType::OnSet, 0u, i * 8u, 1.0f});
			}
			break;
		case Phase::ChokeAll:
			events.push_back({EventType::Choke, 0u, 0u, 0.0f});
			break;
		case Phase::TriggerOnset:
			events.push_back({EventType::OnSet, 0u, 0u, 1.0f});
			break;
		}
	}

private:
	Phase phase{Phase::Idle};
};

// Drives the two-instrument choke-group and directed-choke coverage paths.
//
// Phase::Idle           – fires no events (used during kit loading).
// Phase::FireVictim     – fires a single OnSet for the victim instrument
//                         (victim_id); creates SampleEvents for the victim.
// Phase::FireInitiators – fires three OnSet events for the initiator
//                         instrument (initiator_id) with distinct in-frame
//                         offsets so they are all processed within the same
//                         run() call:
//
//   Onset 1: applyChokeGroup / applyDirectedChoke sees the victim's events
//            (not ramping) → all AND-conditions TRUE → applyChoke executed.
//            Initiator's own events are then added to events_ds.
//   Onset 2: sees both the victim (now ramping → rampdown_count == -1 FALSE)
//            and the initiator's own events from onset 1 (instrument_id ==
//            current → "!=" condition FALSE). Covers both FALSE branches.
//   Onset 3: further exercise of the now-choked state.
//
// Firing all three initiator onsets within one frame guarantees that the
// events added after onset 1 are visible to onset 2's applyChokeGroup /
// applyDirectedChoke call (events are added synchronously before onset 2
// starts processing).
class AudioInputEngineChokeSequenceDummy : public AudioInputEngineDummy
{
public:
	enum class Phase
	{
		Idle,
		FireVictim,
		FireInitiators
	};

	AudioInputEngineChokeSequenceDummy(size_t victim, size_t initiator)
	    : victim_id(victim), initiator_id(initiator)
	{
	}

	void setPhase(Phase p)
	{
		phase = p;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		switch(phase)
		{
		case Phase::Idle:
			break;
		case Phase::FireVictim:
			events.push_back({EventType::OnSet, victim_id, 0u, 1.0f});
			break;
		case Phase::FireInitiators:
			// Three onsets in the same frame. Onset 2 observes the initiator
			// events already added by onset 1 (same instrument, so the
			// instrument_id-mismatch condition is false) and also observes
			// the victim's events already ramping (rampdown-active condition
			// is false), exercising both false branches of the choke guards.
			events.push_back({EventType::OnSet, initiator_id, 0u, 1.0f});
			events.push_back({EventType::OnSet, initiator_id, 8u, 1.0f});
			events.push_back({EventType::OnSet, initiator_id, 16u, 1.0f});
			break;
		}
	}

private:
	size_t victim_id;
	size_t initiator_id;
	Phase phase{Phase::Idle};
};

// Fires a single Stop event once when setFireStop(true) is called.
// Used to exercise processStop() with a loaded kit (so kit.channels is
// non-empty and the ch.num-counting loop body at lines 380-385 executes).
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
			events.push_back({EventType::Stop, 0u, 0u, 0.0f});
			fire_stop = false;
		}
	}

private:
	bool fire_stop{false};
};

// TST-INPUT-03: Fires a single OnSet event for a configurable instrument
// index on the very next run() call, then auto-resets.
class AudioInputEngineOnsetOnDemandDummy : public AudioInputEngineDummy
{
public:
	void fireOnset(std::size_t instrument_id, std::size_t offset = 0)
	{
		pending_instrument = instrument_id;
		pending_offset = offset;
		has_pending = true;
	}

	void run(size_t pos, size_t len, std::vector<event_t>& events) override
	{
		(void)pos;
		(void)len;
		if(has_pending)
		{
			events.push_back(
			    {EventType::OnSet, pending_instrument, pending_offset, 1.0f});
			has_pending = false;
		}
	}

private:
	bool has_pending{false};
	std::size_t pending_instrument{0};
	std::size_t pending_offset{0};
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

	SUBCASE("voiceLimitAllGroupsRampingReturnsEarly")
	{
		// TST-INPUT-01 gap: covers the !non_ramping.size() early-return branch
		// in limitVoices() (src/inputprocessor.cc line 431).
		//
		// Sequence:
		//   Frame A (BuildGroups)  – 3 OnSet events; with voice_limit_max=1
		//     the third onset calls limitVoices() which chokes the oldest
		//     group.  State after: {g1(ramping), g2(not), g3(not)}.
		//   Frame B (ChokeAll)     – a Choke event runs processChoke(), which
		//     applies rampdown to g2 and g3 (g1 is already ramping → skipped).
		//     State after: {g1(R), g2(R), g3(R)}.
		//   Frame C (TriggerOnset) – 1 OnSet fires; processOnset calls
		//     limitVoices() which finds group_ids.size()=3 > max=1, runs the
		//     filter predicate on all three — all return false (ramping) —
		//     so non_ramping is empty → !non_ramping.size() TRUE → returns.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ar_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ar_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioInputEngineAllRampingDummy ie;
		AudioOutputEngineDummy oe;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		auto pollUntilLoaded = [&](size_t& time)
		{
			const size_t max_iter = 2000;
			for(size_t i = 0;
			    i < max_iter &&
			    settings.drumkit_load_status.load() != LoadStatus::Done;
			    ++i)
			{
				CHECK(dg.run(time, buf.data(), nsamples));
				time += nsamples;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);
		};
		auto stabilise = [&](size_t& time)
		{
			// Let the second loadkit() cycle complete.
			for(size_t i = 0; i < 200; ++i)
			{
				CHECK(dg.run(time, buf.data(), nsamples));
				time += nsamples;
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			REQUIRE(settings.drumkit_load_status.load() == LoadStatus::Done);
		};
		pollUntilLoaded(current_time);
		stabilise(current_time);
		settings.enable_voice_limit.store(true);
		settings.voice_limit_max.store(1u);
		settings.voice_limit_rampdown.store(50.0f);

		// Frame A: build groups; limitVoices() chokes the oldest.
		ie.setPhase(AudioInputEngineAllRampingDummy::Phase::BuildGroups);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// Frame B: choke all remaining non-ramping groups.
		ie.setPhase(AudioInputEngineAllRampingDummy::Phase::ChokeAll);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// Frame C: onset fires → limitVoices finds all groups ramping →
		// !non_ramping.size() TRUE → returns early. Engine must survive.
		ie.setPhase(AudioInputEngineAllRampingDummy::Phase::TriggerOnset);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("chokeGroupMutesOtherInstrumentSameGroup")
	{
		// TST-INPUT-03: covers applyChokeGroup() (src/inputprocessor.cc
		// lines 153-181). Two instruments share group="hihat". When the
		// second instrument fires an onset, applyChokeGroup() iterates
		// active SampleEvents for the first instrument and applies rampdown
		// (choke.group == instr.getGroup() && instrument_id != current_id &&
		// rampdown_count == -1 → TRUE branch). A third onset for the second
		// instrument then covers the instrument_id-equals FALSE branch (own
		// events) and the rampdown-already-active FALSE branch.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data},
		    {"instr2", "instr2.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_kit", 1, instruments, wav_infos};

		auto kit_file = drumkit_creator.create(kit_data);

		// Patch the drumkit XML: add group="hihat" to both instruments so
		// applyChokeGroup() finds a matching group name.
		{
			std::ifstream in(kit_file);
			REQUIRE_UNARY(in.is_open());
			std::ostringstream ss;
			ss << in.rdbuf();
			std::string content = ss.str();
			in.close();

			for(const std::string& fname : {"instr1.xml", "instr2.xml"})
			{
				const std::string from = "file=\"" + fname + "\">";
				const std::string to =
				    "file=\"" + fname + "\" group=\"hihat\">";
				auto pos = content.find(from);
				REQUIRE_NE(pos, std::string::npos);
				content.replace(pos, from.size(), to);
			}

			std::ofstream out(kit_file);
			REQUIRE_UNARY(out.is_open());
			out << content;
		}

		// FireVictim: instr0 (id=0) onset creates SampleEvents with
		// group="hihat". FireInitiators: three instr1 (id=1) onsets in the same
		// frame.
		//   Onset 1 – applyChokeGroup chokes instr0 (all TRUE), adds instr1
		//   events. Onset 2 – sees instr0 ramping (rampdown_count != -1 →
		//   FALSE) and
		//             instr1's own events from onset 1 (id=1 != 1 → FALSE).
		//   Onset 3 – further exercise.
		AudioInputEngineChokeSequenceDummy ie(0u, 1u);

		Settings settings;
		AudioOutputEngineDummy oe;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// FireVictim: onset for instr0 creates SampleEvents with group="hihat".
		ie.setPhase(AudioInputEngineChokeSequenceDummy::Phase::FireVictim);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// FireInitiators: three instr1 onsets in the same frame exercise all
		// applyChokeGroup branches, including rampdown FALSE and id!= FALSE.
		ie.setPhase(AudioInputEngineChokeSequenceDummy::Phase::FireInitiators);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("directChokeChokesTargetInstrument")
	{
		// Covers applyDirectedChoke() (src/inputprocessor.cc lines 184-210).
		// Instrument instr1 has a <chokes> entry (in the drumkit XML) pointing
		// to instr2. When instr1 fires an onset, applyDirectedChoke() iterates
		// over instr2's active SampleEvents and applies rampdown. Three instr1
		// onsets in the same frame also cover the rampdown-already-active FALSE
		// branch and the choke.instrument_id-mismatch FALSE branch (instr1's
		// own events from earlier onsets in the same frame).
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("dc_hit.wav", 2048, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		// instr1 (index 0) is the choke initiator; instr2 (index 1) is the
		// victim. The directed choke is declared inside instr1's block in the
		// drumkit XML (not in the instrument's own XML file), because
		// parseDrumkitFile() is what reads the directed-choke table.
		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data},
		    {"instr2", "instr2.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "dc_kit", 1, instruments, wav_infos};

		auto kit_file = drumkit_creator.create(kit_data);

		// Patch the drumkit XML to add a <chokes> element inside instr1's
		// <instrument> block, pointing to instr2. The <chokes> node must
		// appear inside the drumkit-level <instrument> element (not inside
		// the instrument's own XML file) because parseDrumkitFile() is what
		// reads the directed-choke table.
		{
			std::ifstream in(kit_file);
			REQUIRE_UNARY(in.is_open());
			std::ostringstream ss;
			ss << in.rdbuf();
			std::string content = ss.str();
			in.close();

			// Find the first </instrument> (instr1's closing tag) and
			// insert the <chokes> block immediately before it.
			const std::string from = "</instrument>";
			const std::string choke_xml = R"(<chokes>
<choke instrument="instr2" choketime="68"/>
</chokes>
)";
			auto pos = content.find(from);
			REQUIRE_NE(pos, std::string::npos);
			content.insert(pos, choke_xml);

			std::ofstream out(kit_file);
			REQUIRE_UNARY(out.is_open());
			out << content;
		}

		// FireVictim: instr2 (id=1) onset creates SampleEvents for the victim.
		// FireInitiators: three instr0 (id=0, has <chokes>) onsets in one
		// frame.
		//   Onset 1 – applyDirectedChoke finds instr2's events, chokes them
		//             (instrument_id match TRUE, rampdown==-1 TRUE). Adds
		//             instr0.
		//   Onset 2 – finds instr2 ramping (rampdown FALSE) + instr0's own
		//   events
		//             (instrument_id 0 != choke target 1 → FALSE). Both FALSE.
		//   Onset 3 – further exercise.
		AudioInputEngineChokeSequenceDummy ie(1u, 0u);

		Settings settings;
		AudioOutputEngineDummy oe;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// FireVictim: create SampleEvents for instr1 (the choke victim, id=1).
		ie.setPhase(AudioInputEngineChokeSequenceDummy::Phase::FireVictim);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// FireInitiators: three instr0 onsets in the same frame exercise all
		// applyDirectedChoke branches (instrument_id FALSE, rampdown FALSE).
		ie.setPhase(AudioInputEngineChokeSequenceDummy::Phase::FireInitiators);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("processStopBodyWithLoadedKit")
	{
		// Covers the processStop() channel-counting loop body
		// (src/inputprocessor.cc lines 380-389) when kit.channels is
		// non-empty. The existing runReturnsFalseAfterStopWhenNoActiveEvents
		// fires Stop before a kit is loaded (empty channels → loop never
		// iterates). Here we load a kit first, then fire a Stop event; the
		// loop iterates over the kit's channels, finds zero active
		// SampleEvents, and returns false (causing run() to return false).
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ps_hit.wav", 256, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ps_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioInputEngineOnDemandStopDummy ie;
		AudioOutputEngineDummy oe;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// Fire the Stop event with the kit loaded. processStop() will iterate
		// over kit.channels (non-empty), find zero active SampleEvents, and
		// return false — causing run() to return false.
		ie.setFireStop(true);
		auto result = dg.run(current_time, buf.data(), nsamples);
		CHECK_UNARY(!result);
	}

	SUBCASE("normalizedSamplesScalesEventVelocity")
	{
		// Covers the `event_sample.scale *= event.velocity` branch
		// (src/inputprocessor.cc line 303) when both
		// settings.normalized_samples is true and the sample's normalized
		// flag is set in the instrument XML.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("ns_hit.wav", 512, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr1", "instr1.xml", sample_data}};

		DrumkitCreator::DrumkitData kit_data{
		    "ns_kit", 1, instruments, wav_infos};

		auto kit_file = drumkit_creator.create(kit_data);

		// Patch the instrument XML to set normalized="true" on the sample
		// element so that sample->getNormalized() returns true.
		{
			auto kit_dir = kit_file.substr(0, kit_file.rfind('/'));
			auto instr_file = kit_dir + "/instr1.xml";

			std::ifstream in(instr_file);
			REQUIRE_UNARY(in.is_open());
			std::ostringstream ss;
			ss << in.rdbuf();
			std::string content = ss.str();
			in.close();

			// Find the opening sample tag and insert normalized="true" before
			// the closing >.
			const std::string marker = "<sample name=\"stroke\"";
			auto pos = content.find(marker);
			REQUIRE_NE(pos, std::string::npos);
			auto gt_pos = content.find('>', pos);
			REQUIRE_NE(gt_pos, std::string::npos);
			content.insert(gt_pos, " normalized=\"true\"");

			std::ofstream out(instr_file);
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

		// Enable normalized-sample processing before loading.
		settings.normalized_samples.store(true);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// Fire an audition onset with normalized_samples=true and a
		// normalized sample → event_sample.scale *= event.velocity executes.
		settings.audition_instrument.store("instr1");
		settings.audition_velocity.store(0.8f);
		settings.audition_counter.store(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}
	SUBCASE("applyChokeGroupGroupedInstrumentWithNoActiveEvents")
	{
		// TST-INPUT-03b: Covers the applyChokeGroup() path where
		// instr.getGroup() is non-empty (false-branch of early-return) and
		// events_ds is empty on the first onset.  The channel loop enters,
		// ch.num < NUM_CHANNELS is satisfied, and the inner events loop
		// exits immediately without entering its body.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_empty.wav", 64, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr0", "instr0.xml", sample_data, "hihat"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_empty_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// First onset: applyChokeGroup runs with getGroup()="hihat" but
		// events_ds is empty → the inner events loop exits without its body
		// being entered (false-branch of the early-return covered here too).
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("applyChokeGroupSameInstrumentSkipped")
	{
		// TST-INPUT-03d: Covers the false-branch of
		// event_sample.instrument_id != instrument_id when the same
		// instrument is re-triggered while it is still playing.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_same.wav", 1024, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr0", "instr0.xml", sample_data, "hihat"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_same_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// First onset: instr0 starts playing.
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// Second onset for the same instrument: applyChokeGroup finds
		// instr0's SampleEvent, group matches, but instrument_id == 0
		// (same) → event_sample.instrument_id != instrument_id is false
		// → no rampdown applied.
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("applyChokeGroupRampdownAndAlreadyRampingPaths")
	{
		// TST-INPUT-03c+f: Two instruments sharing group "hihat".
		// - Firing instr1 while instr0 plays: all three conditions true
		//   → applyChoke() is called (rampdown-applied path).
		// - Firing instr1 again: instr0's events already have
		//   rampdown_count > 0 → the rampdown_count == -1 false-branch.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_ramp.wav", 1024, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr0", "instr0.xml", sample_data, "hihat"},
		    {"instr1", "instr1.xml", sample_data, "hihat"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_ramp_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// instr0 starts playing.
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// instr1 fires (same group): applyChokeGroup sees instr0's SampleEvent
		// → group matches, instrument_id differs (0 != 1), rampdown_count == -1
		// → all three conditions true → applyChoke() applied.
		ie.fireOnset(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// instr1 fires again: instr0's events now have rampdown_count > 0
		// → the rampdown_count == -1 condition is false → already-ramping
		// skip path exercised.
		ie.fireOnset(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("applyChokeGroupDifferentGroupSkipped")
	{
		// TST-INPUT-03e: Covers the short-circuit false-branch of the first
		// condition (event_sample.group != instr.getGroup()) when two
		// instruments belong to different groups.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_diffgrp.wav", 1024, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr0", "instr0.xml", sample_data, "hihat"},
		    {"instr1", "instr1.xml", sample_data, "snare"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_diffgrp_kit", 1, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// instr0 (group "hihat") starts playing.
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// instr1 (group "snare") fires: applyChokeGroup sees instr0's
		// SampleEvent (group="hihat") but instr1.getGroup()="snare" →
		// event_sample.group != instr.getGroup() → first condition false
		// (short-circuit) → no rampdown.
		ie.fireOnset(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}

	SUBCASE("applyChokeGroupSkipsChannelAboveNumChannels")
	{
		// TST-INPUT-03g: Covers the ch.num >= NUM_CHANNELS true-branch in
		// applyChokeGroup().  A kit with 17 channels has channel 16 with
		// ch.num == 16 == NUM_CHANNELS, which hits the continue guard.
		std::vector<DrumkitCreator::WavInfo> wav_infos = {
		    DrumkitCreator::WavInfo("cg_17ch.wav", 64, 0x1110)};

		std::vector<DrumkitCreator::Audiofile> audiofiles = {
		    {&wav_infos.front(), 1}};

		std::vector<DrumkitCreator::SampleData> sample_data = {
		    {"stroke", audiofiles}};

		std::vector<DrumkitCreator::InstrumentData> instruments = {
		    {"instr0", "instr0.xml", sample_data, "hihat"},
		    {"instr1", "instr1.xml", sample_data, "hihat"}};

		DrumkitCreator::DrumkitData kit_data{
		    "cg_17ch_kit", 17, instruments, wav_infos};

		Settings settings;
		AudioOutputEngineDummy oe;
		AudioInputEngineOnsetOnDemandDummy ie;
		DrumGizmo dg(settings, oe, ie);
		dg.setFrameSize(256);
		CHECK(dg.init());

		constexpr size_t nsamples = 256;
		std::vector<sample_t> buf(nsamples, 0.0f);

		auto kit_file = drumkit_creator.create(kit_data);
		settings.drumkit_file.store(kit_file);

		size_t current_time = 0;
		const size_t max_iter = 2000;
		for(size_t i = 0; i < max_iter && settings.drumkit_load_status.load() !=
		                                      LoadStatus::Done;
		    ++i)
		{
			CHECK(dg.run(current_time, buf.data(), nsamples));
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

		// instr0 fires: SampleEvent created for ch0 only (ch1-ch15 have no
		// audiofile mapping; ch16 is skipped by the NUM_CHANNELS guard in
		// processOnset before it can be added to events_ds).
		ie.fireOnset(0);
		CHECK(dg.run(current_time, buf.data(), nsamples));
		current_time += nsamples;

		// instr1 fires (same group): applyChokeGroup iterates all 17
		// channels; ch16 has ch.num == 16 == NUM_CHANNELS → the
		// ch.num >= NUM_CHANNELS true-branch is taken (continue).
		ie.fireOnset(1);
		CHECK(dg.run(current_time, buf.data(), nsamples));
	}
}
