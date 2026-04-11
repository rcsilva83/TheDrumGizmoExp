/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            pluginguitest.cc
 *
 *  Sat Apr 11 00:00:00 CEST 2026
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

#include <settings.h>
#include <notifier.h>
#include <dggui/guievent.h>

#include <mainwindow.h>

// ---------------------------------------------------------------------------
// Lightweight listener for close notification
// ---------------------------------------------------------------------------

struct CloseListener : public Listener
{
	bool notified{false};
	void onClose()
	{
		notified = true;
	}
};

// ---------------------------------------------------------------------------
// Shared test fixture
// ---------------------------------------------------------------------------

struct PluginGuiFixture
{
	Settings settings;
	SettingsNotifier settings_notifier{settings};

	// MainWindow construction exercises MainTab, DrumkitTab, AboutTab and
	// all their frame-content children (including FileBrowser dialog).
	GUI::MainWindow win{settings, nullptr};

	//! Process a single event loop cycle.
	bool processEvents()
	{
		return win.processEvents();
	}

	//! Resize window and process resulting resize event.
	void resize(std::size_t w, std::size_t h)
	{
		win.resize(w, h);
		win.processEvents();
	}

	//! Evaluate all settings notifiers and process one cycle.
	void applySettings()
	{
		settings_notifier.evaluate();
		win.processEvents();
	}

	//! Inject a synthetic GUI event and process one event loop cycle.
	void inject(std::shared_ptr<dggui::Event> event)
	{
		win.injectTestEvent(event);
		win.processEvents();
	}
};

// ---------------------------------------------------------------------------
// Event helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<dggui::ButtonEvent>
makeButtonEvent(int x, int y, dggui::Direction dir, bool doubleClick = false)
{
	auto evt = std::make_shared<dggui::ButtonEvent>();
	evt->x = x;
	evt->y = y;
	evt->button = dggui::MouseButton::left;
	evt->direction = dir;
	evt->doubleClick = doubleClick;
	return evt;
}

static std::shared_ptr<dggui::MouseMoveEvent>
makeMouseMoveEvent(int x, int y)
{
	auto evt = std::make_shared<dggui::MouseMoveEvent>();
	evt->x = x;
	evt->y = y;
	return evt;
}

// ===========================================================================
// MainWindow tests
// ===========================================================================

TEST_CASE("MainWindow: construction without crash")
{
	Settings s;
	SettingsNotifier sn{s};
	GUI::MainWindow win{s, nullptr};
	CHECK_UNARY(win.processEvents());
}

TEST_CASE("MainWindow: processEvents returns true when not closing")
{
	PluginGuiFixture f;
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("MainWindow: processEvents returns false after closeEventHandler")
{
	PluginGuiFixture f;
	CloseListener listener;
	f.win.closeNotifier.connect(&listener, &CloseListener::onClose);

	f.win.closeEventHandler();
	CHECK_FALSE(f.processEvents());
	CHECK_UNARY(listener.notified);
}

TEST_CASE("MainWindow: closing flag resets after one false return")
{
	PluginGuiFixture f;
	f.win.closeEventHandler();
	CHECK_FALSE(f.processEvents()); // closing=true → false
	CHECK_UNARY(f.processEvents()); // flag cleared → true
}

TEST_CASE("MainWindow: resize to normal dimensions")
{
	PluginGuiFixture f;
	f.resize(750, 740);
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("MainWindow: resize below sidebar threshold")
{
	PluginGuiFixture f;
	// Width < 2*16=32 exercises the std::max(0,...) branch in sizeChanged
	f.resize(10, 200);
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("MainWindow: repaintEvent with window hidden")
{
	PluginGuiFixture f;
	f.resize(750, 740);
	f.win.hide();
	f.processEvents(); // repaintEvent invoked with visible()==false
	f.win.show();
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("MainWindow: close event via native event injection")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// A CloseEvent triggers MainWindow::closeEventHandler via
	// eventHandler->closeNotifier. Inject first, then process once.
	f.win.injectTestEvent(std::make_shared<dggui::CloseEvent>());
	CHECK_FALSE(f.win.processEvents());
}

TEST_CASE("MainWindow: move event via injection")
{
	PluginGuiFixture f;
	f.resize(750, 740);
	auto evt = std::make_shared<dggui::MoveEvent>();
	evt->x = 50;
	evt->y = 50;
	f.inject(evt);
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// StatusframeContent
// ===========================================================================

TEST_CASE("StatusframeContent: drumkit load status - all branches")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	for(auto status : {LoadStatus::Idle, LoadStatus::Parsing,
	                   LoadStatus::Loading, LoadStatus::Done, LoadStatus::Error})
	{
		f.settings.drumkit_load_status.store(status);
		f.applySettings();
	}
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("StatusframeContent: midimap load status - all branches")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	for(auto status : {LoadStatus::Idle, LoadStatus::Parsing,
	                   LoadStatus::Loading, LoadStatus::Done, LoadStatus::Error})
	{
		f.settings.midimap_load_status.store(status);
		f.applySettings();
	}
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("StatusframeContent: text fields update")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.drumkit_name.store("TestKit");
	f.settings.drumkit_description.store("A test kit");
	f.settings.drumkit_version.store("1.0");
	f.settings.buffer_size.store(512);
	f.settings.number_of_underruns.store(3);
	f.settings.load_status_text.store("Loading...");
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// DrumkitframeContent
// ===========================================================================

TEST_CASE("DrumkitframeContent: drumkit load status - all states")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	for(auto status : {LoadStatus::Idle, LoadStatus::Parsing,
	                   LoadStatus::Loading, LoadStatus::Done, LoadStatus::Error})
	{
		f.settings.drumkit_load_status.store(status);
		f.applySettings();
	}
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("DrumkitframeContent: midimap load status - all states")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	for(auto status : {LoadStatus::Idle, LoadStatus::Parsing,
	                   LoadStatus::Loading, LoadStatus::Done, LoadStatus::Error})
	{
		f.settings.midimap_load_status.store(status);
		f.applySettings();
	}
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("DrumkitframeContent: file progress update")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.number_of_files.store(100);
	f.settings.number_of_files_loaded.store(50);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("DrumkitframeContent: drumkit and midimap file paths")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.drumkit_file.store("/tmp/test.xml");
	f.settings.midimap_file.store("/tmp/test.midmap");
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// ResamplingframeContent
// ===========================================================================

TEST_CASE("ResamplingframeContent: drumkit samplerate zero branch")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.drumkit_samplerate.store(0);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("ResamplingframeContent: drumkit samplerate non-zero branch")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.drumkit_samplerate.store(44100);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("ResamplingframeContent: session samplerate and resampling recommended")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.samplerate.store(48000.0);
	f.applySettings();

	f.settings.resampling_recommended.store(true);
	f.applySettings();

	f.settings.resampling_recommended.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("ResamplingframeContent: resampling quality knob")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.resampling_quality.store(0.9f);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// BleedcontrolframeContent
// ===========================================================================

TEST_CASE("BleedcontrolframeContent: setEnabled true and false branches")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// has_bleed_control → bleedcontrol_frame.setEnabled
	// → BleedcontrolframeContent::setEnabled(true/false)
	f.settings.has_bleed_control.store(true);
	f.applySettings();

	f.settings.has_bleed_control.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("BleedcontrolframeContent: master bleed value")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.master_bleed.store(0.75f);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// HumanizerframeContent
// ===========================================================================

TEST_CASE("HumanizerframeContent: velocity modifier settings")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.velocity_modifier_weight.store(0.5f);
	f.settings.velocity_modifier_falloff.store(0.3f);
	f.settings.velocity_stddev.store(0.2f);
	f.applySettings();

	f.settings.enable_velocity_modifier.store(true);
	f.applySettings();

	f.settings.enable_velocity_modifier.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// TimingframeContent
// ===========================================================================

TEST_CASE("TimingframeContent: enable and disable timing humanizer")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_latency_modifier.store(true);
	f.applySettings();

	f.settings.enable_latency_modifier.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// VoiceLimitframeContent
// ===========================================================================

TEST_CASE("VoiceLimitframeContent: voice limit enable and disable")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_voice_limit.store(true);
	f.settings.voice_limit_max.store(4);
	f.settings.voice_limit_rampdown.store(0.5f);
	f.applySettings();

	f.settings.enable_voice_limit.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// DiskstreamingframeContent
// ===========================================================================

TEST_CASE("DiskstreamingframeContent: disk cache settings")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.disk_cache_upper_limit.store(512 * 1024 * 1024);
	f.settings.disk_cache_chunk_size.store(256 * 1024);
	f.settings.disk_cache_enable.store(true);
	f.applySettings();

	f.settings.disk_cache_enable.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// SampleselectionframeContent
// ===========================================================================

TEST_CASE("SampleselectionframeContent: sample selection knobs")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.sample_selection_f_close.store(0.5f);
	f.settings.sample_selection_f_diverse.store(0.3f);
	f.settings.sample_selection_f_random.store(0.2f);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// PowerWidget
// ===========================================================================

TEST_CASE("PowerWidget: enable and disable powermap")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_powermap.store(true);
	f.applySettings();

	f.settings.enable_powermap.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("PowerWidget: input/output indicator lines with and without signal")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// Non-(-1): draws indicator lines branch
	f.settings.powermap_input.store(0.5f);
	f.settings.powermap_output.store(0.6f);
	f.applySettings();

	// (-1): skips indicator branch
	f.settings.powermap_input.store(-1.0f);
	f.settings.powermap_output.store(-1.0f);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("PowerWidget: shelf toggle true and false")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.powermap_shelf.store(true);
	f.applySettings();

	f.settings.powermap_shelf.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("PowerWidget: fixed node position changes")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.powermap_fixed0_x.store(0.1f);
	f.settings.powermap_fixed0_y.store(0.2f);
	f.settings.powermap_fixed1_x.store(0.5f);
	f.settings.powermap_fixed1_y.store(0.5f);
	f.settings.powermap_fixed2_x.store(0.9f);
	f.settings.powermap_fixed2_y.store(0.8f);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("PowerWidget: canvas too small to paint")
{
	PluginGuiFixture f;
	// Very small window exercises the guard branch in Canvas::repaintEvent
	f.resize(50, 50);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// HumaniserVisualiser (VisualizerframeContent)
// ===========================================================================

TEST_CASE("HumaniserVisualiser: resize below minimum guard")
{
	PluginGuiFixture f;
	// Width < 14 → canvas.resize(1,1) branch
	f.resize(10, 10);
	f.applySettings();
	CHECK_UNARY(f.processEvents());
}

TEST_CASE("VisualizerframeContent: latency and velocity enabled states")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_latency_modifier.store(true);
	f.settings.latency_current.store(5.0f);
	f.settings.latency_stddev.store(1.0f);
	f.settings.latency_max_ms.store(100.0f);
	f.applySettings();

	f.settings.enable_velocity_modifier.store(true);
	f.settings.velocity_modifier_current.store(0.5f);
	f.settings.velocity_stddev.store(0.1f);
	f.applySettings();

	// Disable both
	f.settings.enable_latency_modifier.store(false);
	f.settings.enable_velocity_modifier.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// Event injection: mouse, keyboard, scroll
// ===========================================================================

TEST_CASE("EventHandler: button down and up events")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.inject(makeButtonEvent(200, 200, dggui::Direction::down));
	f.inject(makeButtonEvent(200, 200, dggui::Direction::up));

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: double-click followed by down (absorbed by guard)")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// doubleClick=true sets lastWasDoubleClick; next down event is skipped
	f.inject(makeButtonEvent(200, 200, dggui::Direction::down,
	                         /*doubleClick=*/true));
	f.inject(makeButtonEvent(200, 200, dggui::Direction::down));

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: consecutive mouse moves (only last processed)")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// Three consecutive mouse-move events are queued before processEvents;
	// the EventHandler collapses them to one.
	f.win.injectTestEvent(makeMouseMoveEvent(100, 100));
	f.win.injectTestEvent(makeMouseMoveEvent(110, 110));
	f.win.injectTestEvent(makeMouseMoveEvent(120, 120));
	f.win.processEvents();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: mouse enter event")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	auto enter_evt = std::make_shared<dggui::MouseEnterEvent>();
	enter_evt->x = 100;
	enter_evt->y = 150;
	f.inject(enter_evt);

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: mouse leave after mouse enter (mouseFocus path)")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// Set mouse focus via a move
	f.inject(makeMouseMoveEvent(200, 200));
	// Leave event clears the mouse focus
	f.inject(std::make_shared<dggui::MouseLeaveEvent>());

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: scroll event")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	auto scroll_evt = std::make_shared<dggui::ScrollEvent>();
	scroll_evt->x = 200;
	scroll_evt->y = 300;
	scroll_evt->delta = 1.0f;
	f.inject(scroll_evt);

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: key event with keyboard focus")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	// Click a focusable widget to set keyboard focus
	f.inject(makeButtonEvent(200, 200, dggui::Direction::down));
	f.inject(makeButtonEvent(200, 200, dggui::Direction::up));

	auto key_evt = std::make_shared<dggui::KeyEvent>();
	key_evt->direction = dggui::Direction::down;
	key_evt->keycode = dggui::Key::character;
	key_evt->text = "a";
	f.inject(key_evt);

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("EventHandler: repaint event type is a no-op")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	auto repaint_evt = std::make_shared<dggui::RepaintEvent>();
	repaint_evt->x = 0;
	repaint_evt->y = 0;
	repaint_evt->width = 100;
	repaint_evt->height = 100;
	f.inject(repaint_evt);

	CHECK_UNARY(f.processEvents());
}

// ===========================================================================
// MainTab: exercise all OnChange callbacks
// ===========================================================================

TEST_CASE("MainTab: all frame on/off switch callbacks (enable)")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_velocity_modifier.store(true);
	f.settings.enable_resampling.store(true);
	f.settings.enable_latency_modifier.store(true);
	f.settings.enable_powermap.store(true);
	f.settings.enable_voice_limit.store(true);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}

TEST_CASE("MainTab: all frame on/off switch callbacks (disable)")
{
	PluginGuiFixture f;
	f.resize(750, 740);

	f.settings.enable_velocity_modifier.store(false);
	f.settings.enable_resampling.store(false);
	f.settings.enable_latency_modifier.store(false);
	f.settings.enable_powermap.store(false);
	f.settings.enable_voice_limit.store(false);
	f.applySettings();

	CHECK_UNARY(f.processEvents());
}
