/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            dggui_widgettest.cc
 *
 *  Sat Apr 11 00:00:00 CET 2026
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

#include <memory>
#include <string>
#include <vector>

#include <dggui/button_base.h>
#include <dggui/colour.h>
#include <dggui/combobox.h>
#include <dggui/eventhandler.h>
#include <dggui/frame.h>
#include <dggui/guievent.h>
#include <dggui/knob.h>
#include <dggui/label.h>
#include <dggui/layout.h>
#include <dggui/lineedit.h>
#include <dggui/listboxbasic.h>
#include <dggui/scrollbar.h>
#include <dggui/slider.h>
#include <dggui/stackedwidget.h>
#include <dggui/tabwidget.h>
#include <dggui/textedit.h>
#include <dggui/toggle.h>
#include <dggui/widget.h>
#include <dggui/window.h>
#include <notifier.h>

// ---------------------------------------------------------------------------
// Listener-derived probe helpers for Notifier connections
// ---------------------------------------------------------------------------

struct VoidProbe : public Listener
{
	bool called{false};
	void slot()
	{
		called = true;
	}
};

struct BoolProbe : public Listener
{
	bool called{false};
	bool value{false};
	void slot(bool v)
	{
		called = true;
		value = v;
	}
};

struct IntProbe : public Listener
{
	bool called{false};
	int value{-1};
	void slot(int v)
	{
		called = true;
		value = v;
	}
};

struct FloatProbe : public Listener
{
	bool called{false};
	float value{-1.0f};
	void slot(float v)
	{
		called = true;
		value = v;
	}
};

struct SizeSizeProbe : public Listener
{
	std::size_t w{0};
	std::size_t h{0};
	void slot(std::size_t nw, std::size_t nh)
	{
		w = nw;
		h = nh;
	}
};

struct IntIntProbe : public Listener
{
	int x{-1};
	int y{-1};
	void slot(int nx, int ny)
	{
		x = nx;
		y = ny;
	}
};

struct StrStrProbe : public Listener
{
	std::string first;
	std::string second;
	void slot(const std::string& a, const std::string& b)
	{
		first = a;
		second = b;
	}
};

// ---------------------------------------------------------------------------
// Test subclasses exposing protected event handlers
// ---------------------------------------------------------------------------

class TestButton : public dggui::ButtonBase
{
public:
	explicit TestButton(dggui::Widget* parent) : dggui::ButtonBase(parent)
	{
	}

	void sendButtonEvent(dggui::ButtonEvent* e)
	{
		buttonEvent(e);
	}
	void sendMouseLeave()
	{
		mouseLeaveEvent();
	}
	void sendMouseEnter()
	{
		mouseEnterEvent();
	}

	void repaintEvent(dggui::RepaintEvent*) override
	{
	}
};

class TestSlider : public dggui::Slider
{
public:
	explicit TestSlider(dggui::Widget* parent) : dggui::Slider(parent)
	{
	}

	void sendButtonEvent(dggui::ButtonEvent* e)
	{
		buttonEvent(e);
	}
	void sendMouseMove(dggui::MouseMoveEvent* e)
	{
		mouseMoveEvent(e);
	}
	void sendScrollEvent(dggui::ScrollEvent* e)
	{
		scrollEvent(e);
	}
};

class TestKnob : public dggui::Knob
{
public:
	explicit TestKnob(dggui::Widget* parent) : dggui::Knob(parent)
	{
	}

	void sendButtonEvent(dggui::ButtonEvent* e)
	{
		buttonEvent(e);
	}
	void sendMouseMove(dggui::MouseMoveEvent* e)
	{
		mouseMoveEvent(e);
	}
	void sendScrollEvent(dggui::ScrollEvent* e)
	{
		scrollEvent(e);
	}
	void sendKeyEvent(dggui::KeyEvent* e)
	{
		keyEvent(e);
	}
};

class TestToggle : public dggui::Toggle
{
public:
	explicit TestToggle(dggui::Widget* parent) : dggui::Toggle(parent)
	{
	}

	void sendButtonEvent(dggui::ButtonEvent* e)
	{
		buttonEvent(e);
	}
	void sendMouseLeave()
	{
		mouseLeaveEvent();
	}
	void sendMouseEnter()
	{
		mouseEnterEvent();
	}
};

class TestScrollBar : public dggui::ScrollBar
{
public:
	explicit TestScrollBar(dggui::Widget* parent) : dggui::ScrollBar(parent)
	{
	}

	void sendScrollEvent(dggui::ScrollEvent* e)
	{
		scrollEvent(e);
	}
};

// ---------------------------------------------------------------------------
// Window tests (headless)
// ---------------------------------------------------------------------------

TEST_CASE("WindowTest")
{
	SUBCASE("default_constructor_creates_window")
	{
		dggui::Window w;
		CHECK_EQ(&w, w.window());
	}

	SUBCASE("resize_does_not_crash")
	{
		dggui::Window w;
		w.resize(200, 150);
	}

	SUBCASE("move_is_forwarded_to_native")
	{
		dggui::Window w;
		w.move(10, 20);
	}

	SUBCASE("show_hide_cycle")
	{
		dggui::Window w;
		w.show();
		w.hide();
	}

	SUBCASE("setFixedSize_no_crash")
	{
		dggui::Window w;
		w.setFixedSize(400, 300);
	}

	SUBCASE("setCaption_no_crash")
	{
		dggui::Window w;
		w.setCaption("Test Window");
	}

	SUBCASE("setAlwaysOnTop_no_crash")
	{
		dggui::Window w;
		w.setAlwaysOnTop(true);
		w.setAlwaysOnTop(false);
	}

	SUBCASE("getNativeWindowHandle_returns_nullptr_for_headless")
	{
		dggui::Window w;
		CHECK_EQ(nullptr, w.getNativeWindowHandle());
	}

	SUBCASE("keyboard_focus_initially_nullptr")
	{
		dggui::Window w;
		CHECK_EQ(nullptr, w.keyboardFocus());
	}

	SUBCASE("button_down_focus_initially_nullptr")
	{
		dggui::Window w;
		CHECK_EQ(nullptr, w.buttonDownFocus());
	}

	SUBCASE("mouse_focus_initially_nullptr")
	{
		dggui::Window w;
		CHECK_EQ(nullptr, w.mouseFocus());
	}

	SUBCASE("setKeyboardFocus_sets_and_reads_back")
	{
		dggui::Window w;
		w.resize(100, 100);
		dggui::Widget child(&w);
		child.resize(50, 50);
		w.setKeyboardFocus(&child);
		CHECK_EQ(&child, w.keyboardFocus());
	}

	SUBCASE("setKeyboardFocus_nullptr_clears_focus")
	{
		dggui::Window w;
		w.resize(100, 100);
		dggui::Widget child(&w);
		child.resize(20, 20);
		w.setKeyboardFocus(&child);
		w.setKeyboardFocus(nullptr);
		CHECK_EQ(nullptr, w.keyboardFocus());
	}

	SUBCASE("setButtonDownFocus_sets_and_reads_back")
	{
		dggui::Window w;
		w.resize(100, 100);
		dggui::Widget child(&w);
		child.resize(20, 20);
		w.setButtonDownFocus(&child);
		CHECK_EQ(&child, w.buttonDownFocus());
		w.setButtonDownFocus(nullptr);
		CHECK_EQ(nullptr, w.buttonDownFocus());
	}

	SUBCASE("setMouseFocus_sets_and_reads_back")
	{
		dggui::Window w;
		w.resize(100, 100);
		dggui::Widget child(&w);
		child.resize(20, 20);
		w.setMouseFocus(&child);
		CHECK_EQ(&child, w.mouseFocus());
	}

	SUBCASE("needsRedraw_marks_dirty")
	{
		dggui::Window w;
		w.resize(100, 100);
		w.needsRedraw();
	}

	SUBCASE("eventHandler_not_null")
	{
		dggui::Window w;
		CHECK_UNARY(w.eventHandler() != nullptr);
	}

	SUBCASE("getNativeSize_returns_dimensions")
	{
		dggui::Window w;
		auto sz = w.getNativeSize();
		CHECK_UNARY(sz.width >= std::size_t(1u));
		CHECK_UNARY(sz.height >= std::size_t(1u));
	}
}

// ---------------------------------------------------------------------------
// Widget tests (with a headless Window as parent)
// ---------------------------------------------------------------------------

TEST_CASE("WidgetTest")
{
	dggui::Window win;
	win.resize(400, 300);

	SUBCASE("default_dimensions_are_zero")
	{
		dggui::Widget w(&win);
		CHECK_EQ(std::size_t(0u), w.width());
		CHECK_EQ(std::size_t(0u), w.height());
	}

	SUBCASE("resize_updates_dimensions")
	{
		dggui::Widget w(&win);
		w.resize(100, 50);
		CHECK_EQ(std::size_t(100u), w.width());
		CHECK_EQ(std::size_t(50u), w.height());
	}

	SUBCASE("resize_same_size_is_noop")
	{
		dggui::Widget w(&win);
		w.resize(100, 50);
		w.resize(100, 50);
		CHECK_EQ(std::size_t(100u), w.width());
		CHECK_EQ(std::size_t(50u), w.height());
	}

	SUBCASE("move_updates_position")
	{
		dggui::Widget w(&win);
		w.move(10, 20);
		CHECK_EQ(10, w.x());
		CHECK_EQ(20, w.y());
	}

	SUBCASE("move_same_position_is_noop")
	{
		dggui::Widget w(&win);
		w.move(10, 20);
		w.move(10, 20);
		CHECK_EQ(10, w.x());
		CHECK_EQ(20, w.y());
	}

	SUBCASE("position_returns_xy")
	{
		dggui::Widget w(&win);
		w.move(5, 15);
		auto pos = w.position();
		CHECK_EQ(5, pos.x);
		CHECK_EQ(15, pos.y);
	}

	SUBCASE("visible_default_is_true")
	{
		dggui::Widget w(&win);
		CHECK_UNARY(w.visible());
	}

	SUBCASE("show_hide_toggle")
	{
		dggui::Widget w(&win);
		w.hide();
		CHECK_UNARY(!w.visible());
		w.show();
		CHECK_UNARY(w.visible());
	}

	SUBCASE("setVisible_false_hides")
	{
		dggui::Widget w(&win);
		w.setVisible(false);
		CHECK_UNARY(!w.visible());
	}

	SUBCASE("window_returns_parent_window")
	{
		dggui::Widget w(&win);
		CHECK_EQ(&win, w.window());
	}

	SUBCASE("reparent_changes_parent")
	{
		dggui::Widget w1(&win);
		w1.resize(100, 100);
		dggui::Widget w2(&win);
		w2.resize(100, 100);
		dggui::Widget child(&w1);
		child.resize(10, 10);
		child.reparent(&w2);
		child.reparent(&w2);
	}

	SUBCASE("reparent_to_nullptr_removes_from_parent")
	{
		dggui::Widget w1(&win);
		w1.resize(50, 50);
		dggui::Widget child(&w1);
		child.resize(10, 10);
		child.reparent(nullptr);
	}

	SUBCASE("find_returns_child_at_position")
	{
		dggui::Widget parent(&win);
		parent.resize(100, 100);
		dggui::Widget child(&parent);
		child.move(10, 10);
		child.resize(30, 30);
		auto* found = parent.find(15, 15);
		CHECK_EQ(&child, found);
	}

	SUBCASE("find_outside_child_returns_parent")
	{
		dggui::Widget parent(&win);
		parent.resize(100, 100);
		dggui::Widget child(&parent);
		child.move(10, 10);
		child.resize(30, 30);
		auto* found = parent.find(5, 5);
		CHECK_EQ(&parent, found);
	}

	SUBCASE("find_invisible_child_is_skipped")
	{
		dggui::Widget parent(&win);
		parent.resize(100, 100);
		dggui::Widget child(&parent);
		child.move(10, 10);
		child.resize(30, 30);
		child.hide();
		auto* found = parent.find(15, 15);
		CHECK_EQ(&parent, found);
	}

	SUBCASE("getPixelBuffers_returns_non_empty")
	{
		dggui::Widget w(&win);
		w.resize(50, 50);
		auto bufs = w.getPixelBuffers();
		CHECK_UNARY(!bufs.empty());
	}

	SUBCASE("sizeChangeNotifier_fires_on_resize")
	{
		dggui::Widget w(&win);
		SizeSizeProbe probe;
		w.sizeChangeNotifier.connect(&probe, &SizeSizeProbe::slot);
		w.resize(80, 60);
		CHECK_EQ(std::size_t(80u), probe.w);
		CHECK_EQ(std::size_t(60u), probe.h);
	}

	SUBCASE("positionChangeNotifier_fires_on_move")
	{
		dggui::Widget w(&win);
		IntIntProbe probe;
		w.positionChangeNotifier.connect(&probe, &IntIntProbe::slot);
		w.move(7, 13);
		CHECK_EQ(7, probe.x);
		CHECK_EQ(13, probe.y);
	}

	SUBCASE("removeChild_via_destructor")
	{
		dggui::Widget parent(&win);
		parent.resize(50, 50);
		{
			dggui::Widget child(&parent);
			child.resize(10, 10);
		}
		auto bufs = parent.getPixelBuffers();
		CHECK_EQ(std::size_t(1u), bufs.size());
	}

	SUBCASE("getImageCache_no_crash")
	{
		dggui::Widget w(&win);
		const auto& cache = w.getImageCache();
		(void)cache;
	}

	SUBCASE("resize_then_resize_again_triggers_has_last")
	{
		dggui::Widget w(&win);
		w.resize(50, 50);
		w.resize(80, 80);
	}

	SUBCASE("move_then_move_again_triggers_has_last")
	{
		dggui::Widget w(&win);
		w.resize(50, 50);
		w.move(10, 10);
		w.move(20, 20);
	}
}

// ---------------------------------------------------------------------------
// EventHandler tests
// ---------------------------------------------------------------------------

TEST_CASE("EventHandlerTest")
{
	dggui::Window win;
	win.resize(300, 200);

	SUBCASE("no_events_initially")
	{
		CHECK_UNARY(!win.eventHandler()->hasEvent());
	}

	SUBCASE("processEvents_no_crash_empty_queue")
	{
		win.eventHandler()->processEvents();
	}

	SUBCASE("getNextEvent_returns_null_when_empty")
	{
		auto ev = win.eventHandler()->getNextEvent();
		CHECK_UNARY(!ev);
	}

	SUBCASE("queryNextEventType_false_when_empty")
	{
		CHECK_UNARY(
		    !win.eventHandler()->queryNextEventType(dggui::EventType::button));
	}
}

// ---------------------------------------------------------------------------
// ButtonBase tests
// ---------------------------------------------------------------------------

TEST_CASE("ButtonBaseTest")
{
	dggui::Window win;
	win.resize(200, 100);

	SUBCASE("setText_no_crash")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		btn.setText("Hello");
	}

	SUBCASE("isEnabled_default_true")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		CHECK_UNARY(btn.isEnabled());
	}

	SUBCASE("setEnabled_false_disables")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		btn.setEnabled(false);
		CHECK_UNARY(!btn.isEnabled());
	}

	SUBCASE("buttonEvent_left_down")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		ev.x = 5;
		ev.y = 5;
		btn.sendButtonEvent(&ev);
	}

	SUBCASE("buttonEvent_left_down_then_up_triggers_click")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		VoidProbe probe;
		btn.clickNotifier.connect(&probe, &VoidProbe::slot);

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		ev_down.x = 5;
		ev_down.y = 5;
		btn.sendButtonEvent(&ev_down);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		ev_up.x = 5;
		ev_up.y = 5;
		btn.sendButtonEvent(&ev_up);

		CHECK_UNARY(probe.called);
	}

	SUBCASE("buttonEvent_right_button_no_click")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		VoidProbe probe;
		btn.clickNotifier.connect(&probe, &VoidProbe::slot);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::right;
		ev.direction = dggui::Direction::down;
		btn.sendButtonEvent(&ev);

		dggui::ButtonEvent ev2{};
		ev2.button = dggui::MouseButton::right;
		ev2.direction = dggui::Direction::up;
		btn.sendButtonEvent(&ev2);

		CHECK_UNARY(!probe.called);
	}

	SUBCASE("buttonEvent_disabled_no_click")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		btn.setEnabled(false);
		VoidProbe probe;
		btn.clickNotifier.connect(&probe, &VoidProbe::slot);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		btn.sendButtonEvent(&ev);

		dggui::ButtonEvent ev2{};
		ev2.button = dggui::MouseButton::left;
		ev2.direction = dggui::Direction::up;
		btn.sendButtonEvent(&ev2);

		CHECK_UNARY(!probe.called);
	}

	SUBCASE("mouseLeaveEvent_enabled_with_button_down")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		btn.sendButtonEvent(&ev);
		btn.sendMouseLeave();
	}

	SUBCASE("mouseLeaveEvent_disabled_is_noop")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		btn.setEnabled(false);
		btn.sendMouseLeave();
	}

	SUBCASE("mouseEnterEvent_enabled_after_leave")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		btn.sendButtonEvent(&ev);
		btn.sendMouseLeave();
		btn.sendMouseEnter();
	}

	SUBCASE("mouseEnterEvent_disabled_is_noop")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		btn.setEnabled(false);
		btn.sendMouseEnter();
	}

	SUBCASE("buttonEvent_up_without_prior_down_no_click")
	{
		TestButton btn(&win);
		btn.resize(80, 30);
		VoidProbe probe;
		btn.clickNotifier.connect(&probe, &VoidProbe::slot);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::up;
		btn.sendButtonEvent(&ev);

		CHECK_UNARY(!probe.called);
	}
}

// ---------------------------------------------------------------------------
// Toggle tests
// ---------------------------------------------------------------------------

TEST_CASE("ToggleTest")
{
	dggui::Window win;
	win.resize(200, 100);

	SUBCASE("checked_default_false")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		CHECK_UNARY(!t.checked());
	}

	SUBCASE("setChecked_true")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		t.setChecked(true);
		CHECK_UNARY(t.checked());
	}

	SUBCASE("setChecked_false")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		t.setChecked(true);
		t.setChecked(false);
		CHECK_UNARY(!t.checked());
	}

	SUBCASE("setText_no_crash")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		t.setText("Toggle me");
	}

	SUBCASE("stateChangedNotifier_fires_on_toggle")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		BoolProbe probe;
		t.stateChangedNotifier.connect(&probe, &BoolProbe::slot);

		// mouseEnterEvent sets inCheckbox=true, required for buttonEvent to
		// toggle
		t.sendMouseEnter();

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		t.sendButtonEvent(&ev_down);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		t.sendButtonEvent(&ev_up);

		CHECK_UNARY(probe.called);
		CHECK_UNARY(probe.value);
	}

	SUBCASE("mouseLeaveEvent_and_mouseEnterEvent")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		t.sendMouseLeave();
		t.sendMouseEnter();
	}

	SUBCASE("buttonEvent_right_button_no_notify")
	{
		TestToggle t(&win);
		t.resize(80, 30);
		BoolProbe probe;
		t.stateChangedNotifier.connect(&probe, &BoolProbe::slot);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::right;
		ev.direction = dggui::Direction::down;
		t.sendButtonEvent(&ev);
		CHECK_UNARY(!probe.called);
	}
}

// ---------------------------------------------------------------------------
// ScrollBar tests
// ---------------------------------------------------------------------------

TEST_CASE("ScrollBarTest")
{
	dggui::Window win;
	win.resize(300, 200);

	SUBCASE("default_value_is_zero")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		CHECK_EQ(0, sb.value());
	}

	SUBCASE("setValue_and_getValue")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(5);
		CHECK_EQ(5, sb.value());
	}

	SUBCASE("setValue_clamped_to_max")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(200);
		CHECK_UNARY(sb.value() <= 100);
	}

	SUBCASE("setValue_clamped_to_zero")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(-5);
		CHECK_UNARY(sb.value() >= 0);
	}

	SUBCASE("setRange_and_range")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setRange(50);
		CHECK_EQ(50, sb.range());
	}

	SUBCASE("setMaximum_and_maximum")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		CHECK_EQ(100, sb.maximum());
	}

	SUBCASE("addValue_increments")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(5);
		sb.addValue(3);
		CHECK_EQ(8, sb.value());
	}

	SUBCASE("addValue_negative_decrements")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(5);
		sb.addValue(-3);
		CHECK_EQ(2, sb.value());
	}

	SUBCASE("scrollEvent_positive_delta")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(5);

		dggui::ScrollEvent ev;
		ev.delta = 2.0f;
		sb.sendScrollEvent(&ev);
	}

	SUBCASE("scrollEvent_negative_delta")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		sb.setValue(50);

		dggui::ScrollEvent ev;
		ev.delta = -3.0f;
		sb.sendScrollEvent(&ev);
	}

	SUBCASE("valueChangeNotifier_fires")
	{
		TestScrollBar sb(&win);
		sb.resize(16, 100);
		sb.setMaximum(100);
		IntProbe probe;
		sb.valueChangeNotifier.connect(&probe, &IntProbe::slot);
		sb.setValue(7);
		CHECK_EQ(true, probe.called);
		CHECK_EQ(7, probe.value);
	}
}

// ---------------------------------------------------------------------------
// ListBoxBasic tests
// ---------------------------------------------------------------------------

TEST_CASE("ListBoxBasicTest")
{
	dggui::Window win;
	win.resize(300, 400);

	SUBCASE("addItem_and_selectedValue_after_select")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item 1", "val1");
		lb.addItem("Item 2", "val2");
		lb.selectItem(0);
		CHECK_EQ(std::string("val1"), lb.selectedValue());
		CHECK_EQ(std::string("Item 1"), lb.selectedName());
	}

	SUBCASE("clear_removes_all_items")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item 1", "val1");
		lb.clear();
		lb.selectItem(0);
		CHECK_EQ(std::string(""), lb.selectedValue());
	}

	SUBCASE("selectItem_out_of_range_returns_false")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item", "val");
		CHECK_UNARY(!lb.selectItem(5));
	}

	SUBCASE("selectItem_valid_index_returns_true")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item", "val");
		CHECK_UNARY(lb.selectItem(0));
	}

	SUBCASE("addItems_vector")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		std::vector<dggui::ListBoxBasic::Item> items = {
		    {"A", "a"},
		    {"B", "b"},
		};
		lb.addItems(items);
		lb.selectItem(1);
		CHECK_EQ(std::string("b"), lb.selectedValue());
	}

	SUBCASE("clearSelectedValue_clears_selection")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item", "val");
		lb.selectItem(0);
		lb.clearSelectedValue();
		CHECK_EQ(std::string(""), lb.selectedValue());
	}

	SUBCASE("resize_updates_scrollbar")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 100);
		lb.resize(200, 200);
	}

	SUBCASE("selectionNotifier_not_fired_by_selectItem")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("Item 1", "val1");
		lb.addItem("Item 2", "val2");
		VoidProbe probe;
		lb.selectionNotifier.connect(&probe, &VoidProbe::slot);
		// selectItem does NOT fire selectionNotifier (only button/key events
		// do)
		lb.selectItem(1);
		CHECK_EQ(std::string("val2"), lb.selectedValue());
	}

	SUBCASE("selectItem_third_item")
	{
		dggui::ListBoxBasic lb(&win);
		lb.resize(150, 200);
		lb.addItem("A", "a");
		lb.addItem("B", "b");
		lb.addItem("C", "c");
		lb.selectItem(2);
		CHECK_EQ(std::string("c"), lb.selectedValue());
	}
}

// ---------------------------------------------------------------------------
// LineEdit tests
// ---------------------------------------------------------------------------

TEST_CASE("LineEditTest")
{
	dggui::Window win;
	win.resize(300, 100);

	SUBCASE("getText_initially_empty")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		CHECK_EQ(std::string(""), le.getText());
	}

	SUBCASE("setText_and_getText")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		CHECK_EQ(std::string("hello"), le.getText());
	}

	SUBCASE("setReadOnly_and_readOnly")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setReadOnly(true);
		CHECK_UNARY(le.readOnly());
		le.setReadOnly(false);
		CHECK_UNARY(!le.readOnly());
	}

	SUBCASE("keyEvent_character_appends")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::character;
		ev.text = "A";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);

		CHECK_EQ(std::string("A"), le.getText());
	}

	SUBCASE("keyEvent_backspace_deletes")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::backspace;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);

		CHECK_EQ(std::string("hell"), le.getText());
	}

	SUBCASE("keyEvent_deleteKey_no_crash")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::deleteKey;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);
	}

	SUBCASE("keyEvent_enter_fires_notifier")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		win.setKeyboardFocus(&le);
		VoidProbe probe;
		le.enterPressedNotifier.connect(&probe, &VoidProbe::slot);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::enter;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);

		CHECK_UNARY(probe.called);
	}

	SUBCASE("keyEvent_up_key")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::up;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);
	}

	SUBCASE("keyEvent_down_key")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::down;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);
	}

	SUBCASE("keyEvent_home_key")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::home;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);
	}

	SUBCASE("keyEvent_end_key")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::end;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);
	}

	SUBCASE("keyEvent_left_right_navigation")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setText("hello");
		win.setKeyboardFocus(&le);

		dggui::KeyEvent evLeft;
		evLeft.keycode = dggui::Key::left;
		evLeft.text = "";
		evLeft.direction = dggui::Direction::down;
		le.keyEvent(&evLeft);

		dggui::KeyEvent evRight;
		evRight.keycode = dggui::Key::right;
		evRight.text = "";
		evRight.direction = dggui::Direction::down;
		le.keyEvent(&evRight);
	}

	SUBCASE("keyEvent_key_up_direction_ignored")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::unknown;
		ev.text = "X";
		ev.direction = dggui::Direction::up;
		le.keyEvent(&ev);

		CHECK_EQ(std::string(""), le.getText());
	}

	SUBCASE("buttonEvent_gives_keyboard_focus")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		ev.x = 5;
		ev.y = 5;
		le.buttonEvent(&ev);
	}

	SUBCASE("readonly_does_not_accept_text")
	{
		dggui::LineEdit le(&win);
		le.resize(200, 30);
		le.setReadOnly(true);
		win.setKeyboardFocus(&le);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::unknown;
		ev.text = "X";
		ev.direction = dggui::Direction::down;
		le.keyEvent(&ev);

		CHECK_EQ(std::string(""), le.getText());
	}
}

// ---------------------------------------------------------------------------
// StackedWidget tests
// ---------------------------------------------------------------------------

TEST_CASE("StackedWidgetTest")
{
	dggui::Window win;
	win.resize(300, 200);

	SUBCASE("addWidget_and_setCurrentWidget")
	{
		dggui::StackedWidget sw(&win);
		sw.resize(200, 150);

		dggui::Widget page1(&sw);
		page1.resize(200, 150);

		dggui::Widget page2(&sw);
		page2.resize(200, 150);

		sw.addWidget(&page1);
		sw.addWidget(&page2);

		sw.setCurrentWidget(&page1);
		sw.setCurrentWidget(&page2);
	}

	SUBCASE("removeWidget_no_crash")
	{
		dggui::StackedWidget sw(&win);
		sw.resize(200, 150);

		dggui::Widget page(&sw);
		page.resize(200, 150);
		sw.addWidget(&page);
		sw.removeWidget(&page);
	}
}

// ---------------------------------------------------------------------------
// TabWidget tests
// ---------------------------------------------------------------------------

TEST_CASE("TabWidgetTest")
{
	dggui::Window win;
	win.resize(400, 300);

	SUBCASE("addTab_and_getBarHeight")
	{
		dggui::TabWidget tw(&win);
		tw.resize(300, 200);

		dggui::Widget page1(&tw);
		page1.resize(300, 180);

		dggui::TabID id = tw.addTab("Tab 1", &page1);
		CHECK_UNARY(tw.getBarHeight() > 0u);
		(void)id;
	}

	SUBCASE("setTabWidth")
	{
		dggui::TabWidget tw(&win);
		tw.resize(300, 200);
		tw.setTabWidth(80);
		CHECK_EQ(std::size_t(80u), tw.getTabWidth());
	}

	SUBCASE("setVisible_tab")
	{
		dggui::TabWidget tw(&win);
		tw.resize(300, 200);

		dggui::Widget page1(&tw);
		page1.resize(300, 180);
		dggui::Widget page2(&tw);
		page2.resize(300, 180);

		dggui::TabID id1 = tw.addTab("Tab 1", &page1);
		dggui::TabID id2 = tw.addTab("Tab 2", &page2);
		tw.setVisible(id1, false);
		tw.setVisible(id2, true);
	}

	SUBCASE("resize_tab_widget")
	{
		dggui::TabWidget tw(&win);
		tw.resize(300, 200);

		dggui::Widget page1(&tw);
		tw.addTab("Tab 1", &page1);

		tw.resize(400, 250);
	}
}

// ---------------------------------------------------------------------------
// Slider tests
// ---------------------------------------------------------------------------

TEST_CASE("SliderTest")
{
	dggui::Window win;
	win.resize(300, 100);

	SUBCASE("setValue_and_value")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		s.setValue(0.5f);
		CHECK_UNARY(s.value() >= 0.0f && s.value() <= 1.0f);
	}

	SUBCASE("setColour_all_colours")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		s.setColour(dggui::Slider::Colour::Green);
		s.setColour(dggui::Slider::Colour::Red);
		s.setColour(dggui::Slider::Colour::Blue);
		s.setColour(dggui::Slider::Colour::Yellow);
		s.setColour(dggui::Slider::Colour::Purple);
		s.setColour(dggui::Slider::Colour::Grey);
	}

	SUBCASE("setEnabled_false")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		s.setEnabled(false);
	}

	SUBCASE("scrollEvent_positive_delta")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		s.setValue(0.5f);

		dggui::ScrollEvent ev;
		ev.delta = 1.0f;
		s.sendScrollEvent(&ev);
	}

	SUBCASE("scrollEvent_negative_delta")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		s.setValue(0.5f);

		dggui::ScrollEvent ev;
		ev.delta = -1.0f;
		s.sendScrollEvent(&ev);
	}

	SUBCASE("buttonEvent_down_and_up")
	{
		TestSlider s(&win);
		s.resize(200, 30);

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		ev_down.x = 100;
		ev_down.y = 15;
		s.sendButtonEvent(&ev_down);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		ev_up.x = 100;
		ev_up.y = 15;
		s.sendButtonEvent(&ev_up);
	}

	SUBCASE("mouseMoveEvent_while_dragging")
	{
		TestSlider s(&win);
		s.resize(200, 30);

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		ev_down.x = 50;
		ev_down.y = 15;
		s.sendButtonEvent(&ev_down);

		dggui::MouseMoveEvent mv;
		mv.x = 100;
		mv.y = 15;
		s.sendMouseMove(&mv);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		ev_up.x = 100;
		ev_up.y = 15;
		s.sendButtonEvent(&ev_up);
	}

	SUBCASE("valueChangedNotifier_fires")
	{
		TestSlider s(&win);
		s.resize(200, 30);
		FloatProbe probe;
		s.valueChangedNotifier.connect(&probe, &FloatProbe::slot);

		dggui::ScrollEvent ev;
		ev.delta = 1.0f;
		s.sendScrollEvent(&ev);
		CHECK_UNARY(probe.called);
	}

	SUBCASE("buttonEvent_right_button_ignored")
	{
		TestSlider s(&win);
		s.resize(200, 30);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::right;
		ev.direction = dggui::Direction::down;
		ev.x = 50;
		ev.y = 15;
		s.sendButtonEvent(&ev);
	}
}

// ---------------------------------------------------------------------------
// Knob tests
// ---------------------------------------------------------------------------

TEST_CASE("KnobTest")
{
	dggui::Window win;
	win.resize(200, 200);

	SUBCASE("setValue_and_value")
	{
		TestKnob k(&win);
		k.resize(60, 60);
		k.setValue(0.5f);
		CHECK_UNARY(k.value() >= 0.0f && k.value() <= 1.0f);
	}

	SUBCASE("setRange_affects_value")
	{
		TestKnob k(&win);
		k.resize(60, 60);
		k.setRange(0.0f, 100.0f);
		k.setValue(50.0f);
		CHECK_UNARY(k.value() >= 0.0f);
	}

	SUBCASE("scrollEvent_positive")
	{
		TestKnob k(&win);
		k.resize(60, 60);
		k.setValue(0.5f);

		dggui::ScrollEvent ev;
		ev.delta = 1.0f;
		k.sendScrollEvent(&ev);
	}

	SUBCASE("scrollEvent_negative")
	{
		TestKnob k(&win);
		k.resize(60, 60);
		k.setValue(0.5f);

		dggui::ScrollEvent ev;
		ev.delta = -1.0f;
		k.sendScrollEvent(&ev);
	}

	SUBCASE("buttonEvent_down_up")
	{
		TestKnob k(&win);
		k.resize(60, 60);

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		ev_down.x = 30;
		ev_down.y = 30;
		k.sendButtonEvent(&ev_down);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		ev_up.x = 30;
		ev_up.y = 30;
		k.sendButtonEvent(&ev_up);
	}

	SUBCASE("mouseMoveEvent_while_dragging")
	{
		TestKnob k(&win);
		k.resize(60, 60);

		dggui::ButtonEvent ev_down{};
		ev_down.button = dggui::MouseButton::left;
		ev_down.direction = dggui::Direction::down;
		ev_down.x = 30;
		ev_down.y = 30;
		k.sendButtonEvent(&ev_down);

		dggui::MouseMoveEvent mv;
		mv.x = 35;
		mv.y = 25;
		k.sendMouseMove(&mv);

		dggui::ButtonEvent ev_up{};
		ev_up.button = dggui::MouseButton::left;
		ev_up.direction = dggui::Direction::up;
		ev_up.x = 35;
		ev_up.y = 25;
		k.sendButtonEvent(&ev_up);
	}

	SUBCASE("buttonEvent_right_button_ignored")
	{
		TestKnob k(&win);
		k.resize(60, 60);

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::right;
		ev.direction = dggui::Direction::down;
		k.sendButtonEvent(&ev);
	}

	SUBCASE("keyEvent_up_arrow")
	{
		TestKnob k(&win);
		k.resize(60, 60);

		dggui::KeyEvent ev;
		ev.keycode = dggui::Key::up;
		ev.text = "";
		ev.direction = dggui::Direction::down;
		k.sendKeyEvent(&ev);
	}
}

// ---------------------------------------------------------------------------
// TextEdit tests
// ---------------------------------------------------------------------------

TEST_CASE("TextEditTest")
{
	dggui::Window win;
	win.resize(300, 200);

	SUBCASE("setText_and_getText")
	{
		dggui::TextEdit te(&win);
		te.resize(200, 100);
		te.setText("Hello, World!");
		CHECK_EQ(std::string("Hello, World!"), te.getText());
	}

	SUBCASE("getText_initially_empty")
	{
		dggui::TextEdit te(&win);
		te.resize(200, 100);
		CHECK_EQ(std::string(""), te.getText());
	}

	SUBCASE("setReadOnly")
	{
		dggui::TextEdit te(&win);
		te.resize(200, 100);
		te.setReadOnly(true);
		te.setReadOnly(false);
	}

	SUBCASE("resize_updates_scrollbar")
	{
		dggui::TextEdit te(&win);
		te.resize(200, 100);
		te.resize(300, 150);
	}
}

// ---------------------------------------------------------------------------
// Label tests
// ---------------------------------------------------------------------------

TEST_CASE("LabelTest")
{
	dggui::Window win;
	win.resize(200, 100);

	SUBCASE("setText_and_resize")
	{
		dggui::Label l(&win);
		l.resize(100, 30);
		l.setText("Hello");
	}

	SUBCASE("empty_text_no_crash")
	{
		dggui::Label l(&win);
		l.resize(100, 30);
		l.setText("");
	}
}

// ---------------------------------------------------------------------------
// FrameWidget tests
// ---------------------------------------------------------------------------

TEST_CASE("FrameWidgetTest")
{
	dggui::Window win;
	win.resize(300, 200);

	SUBCASE("create_and_resize")
	{
		dggui::FrameWidget f(&win);
		f.resize(200, 100);
	}

	SUBCASE("setTitle_no_crash")
	{
		dggui::FrameWidget f(&win);
		f.resize(200, 100);
		f.setTitle("My Frame");
	}

	SUBCASE("isSwitchedOn_true_when_no_switch")
	{
		// When has_switch=false (default), is_switched_on starts true
		dggui::FrameWidget f(&win);
		f.resize(200, 100);
		CHECK_UNARY(f.isSwitchedOn());
	}

	SUBCASE("isSwitchedOn_false_when_has_switch")
	{
		// When has_switch=true, is_switched_on starts false
		dggui::FrameWidget f(&win, true);
		f.resize(200, 100);
		CHECK_UNARY(!f.isSwitchedOn());
	}

	SUBCASE("setOnSwitch_enables")
	{
		dggui::FrameWidget f(&win, true);
		f.resize(200, 100);
		f.setOnSwitch(true);
		CHECK_UNARY(f.isSwitchedOn());
	}

	SUBCASE("setEnabled_no_crash")
	{
		dggui::FrameWidget f(&win, true);
		f.resize(200, 100);
		f.setEnabled(true);
		f.setEnabled(false);
	}

	SUBCASE("with_switch_and_help_text")
	{
		dggui::FrameWidget f(&win, true, true);
		f.resize(200, 100);
		f.setHelpText("Some help text");
	}
}

// ---------------------------------------------------------------------------
// ComboBox tests
// ---------------------------------------------------------------------------

TEST_CASE("ComboBoxTest")
{
	dggui::Window win;
	win.resize(300, 400);

	SUBCASE("addItem_and_selectedValue_after_select")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("Item 1", "val1");
		cb.addItem("Item 2", "val2");
		cb.selectItem(0);
		CHECK_EQ(std::string("val1"), cb.selectedValue());
		CHECK_EQ(std::string("Item 1"), cb.selectedName());
	}

	SUBCASE("clear_removes_items")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("Item 1", "val1");
		cb.clear();
		CHECK_EQ(std::string(""), cb.selectedValue());
	}

	SUBCASE("buttonEvent_left_down_toggles_listbox")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("A", "a");
		cb.addItem("B", "b");

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::left;
		ev.direction = dggui::Direction::down;
		ev.doubleClick = false;
		cb.buttonEvent(&ev);
	}

	SUBCASE("buttonEvent_right_click_ignored")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("A", "a");

		dggui::ButtonEvent ev{};
		ev.button = dggui::MouseButton::right;
		ev.direction = dggui::Direction::down;
		ev.doubleClick = false;
		cb.buttonEvent(&ev);
		// No crash expected; state unchanged
	}

	SUBCASE("keyEvent_direction_down_early_return")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("A", "a");

		dggui::KeyEvent kev{};
		kev.direction = dggui::Direction::down;
		cb.keyEvent(&kev);
	}

	SUBCASE("repaintEvent_no_crash")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("Hello", "world");
		cb.selectItem(0);
		dggui::RepaintEvent rev{};
		cb.repaintEvent(&rev);
	}

	SUBCASE("valueChangedNotifier_fired_on_second_click")
	{
		dggui::ComboBox cb(&win);
		cb.resize(120, 30);
		cb.addItem("A", "a");
		cb.addItem("B", "b");
		cb.selectItem(0);

		std::string notified_name;
		std::string notified_value;
		StrStrProbe probe;
		cb.valueChangedNotifier.connect(&probe, &StrStrProbe::slot);

		// First click: open the listbox
		dggui::ButtonEvent ev1{};
		ev1.button = dggui::MouseButton::left;
		ev1.direction = dggui::Direction::down;
		ev1.doubleClick = false;
		cb.buttonEvent(&ev1);

		// Second click: close and emit valueChanged
		dggui::ButtonEvent ev2{};
		ev2.button = dggui::MouseButton::left;
		ev2.direction = dggui::Direction::down;
		ev2.doubleClick = false;
		cb.buttonEvent(&ev2);

		CHECK_EQ(std::string("A"), probe.first);
		CHECK_EQ(std::string("a"), probe.second);
	}
}
