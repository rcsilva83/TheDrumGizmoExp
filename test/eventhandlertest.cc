/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            eventhandlertest.cc
 *
 *  Tue Apr 14 2026
 *  Copyright 2026 DrumGizmo Authors
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

#include <dggui/dialog.h>
#include <dggui/eventhandler.h>
#include <dggui/nativewindow.h>
#include <dggui/widget.h>
#include <dggui/window.h>

class MockNativeWindow : public dggui::NativeWindow
{
public:
	MockNativeWindow()
	{
	}

	void setFixedSize(std::size_t width, std::size_t height) override
	{
		width_ = width;
		height_ = height;
	}

	void setAlwaysOnTop(bool always_on_top) override
	{
	}

	void resize(std::size_t width, std::size_t height) override
	{
		width_ = width;
		height_ = height;
	}

	std::pair<std::size_t, std::size_t> getSize() const override
	{
		return {width_, height_};
	}

	void move(int x, int y) override
	{
		x_ = x;
		y_ = y;
	}

	std::pair<int, int> getPosition() const override
	{
		return {x_, y_};
	}

	void show() override
	{
		visible_ = true;
	}

	void hide() override
	{
		visible_ = false;
	}

	bool visible() const override
	{
		return visible_;
	}

	void setCaption(const std::string& caption) override
	{
		caption_ = caption;
	}

	void redraw(const dggui::Rect& dirty_rect) override
	{
	}

	void grabMouse(bool grab) override
	{
	}

	EventQueue getEvents() override
	{
		EventQueue result = events_;
		events_.clear();
		return result;
	}

	void* getNativeWindowHandle() const override
	{
		return nullptr;
	}

	dggui::Point translateToScreen(const dggui::Point& point) override
	{
		return point;
	}

	void addEvent(std::shared_ptr<dggui::Event> event)
	{
		events_.push_back(event);
	}

	void clearEvents()
	{
		events_.clear();
	}

private:
	std::size_t width_ = 100;
	std::size_t height_ = 100;
	int x_ = 0;
	int y_ = 0;
	bool visible_ = true;
	std::string caption_;
	EventQueue events_;
};

class TestWindow final : public dggui::Window
{
public:
	TestWindow() : dggui::Window(nullptr)
	{
	}

	void needsRedraw() override
	{
		needs_redraw = true;
	}
};

class TestWidget : public dggui::Widget
{
public:
	explicit TestWidget(dggui::Widget* parent) : dggui::Widget(parent)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

	bool isFocusable() override
	{
		return focusable_;
	}

	bool catchMouse() override
	{
		return catch_mouse_;
	}

	bool focusable_ = false;
	bool catch_mouse_ = false;

	int repaintEventCount = 0;
	int mouseMoveEventCount = 0;
	int buttonEventCount = 0;
	int scrollEventCount = 0;
	int keyEventCount = 0;
	int mouseLeaveEventCount = 0;
	int mouseEnterEventCount = 0;

	void repaintEvent(dggui::RepaintEvent* repaintEvent) override
	{
		repaintEventCount++;
	}

	void mouseMoveEvent(dggui::MouseMoveEvent* mouseMoveEvent) override
	{
		mouseMoveEventCount++;
	}

	void buttonEvent(dggui::ButtonEvent* buttonEvent) override
	{
		buttonEventCount++;
	}

	void scrollEvent(dggui::ScrollEvent* scrollEvent) override
	{
		scrollEventCount++;
	}

	void keyEvent(dggui::KeyEvent* keyEvent) override
	{
		keyEventCount++;
	}

	void mouseLeaveEvent() override
	{
		mouseLeaveEventCount++;
	}

	void mouseEnterEvent() override
	{
		mouseEnterEventCount++;
	}
};

TEST_CASE("EventHandlerTest")
{
	SUBCASE("testHasEvent")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		CHECK_UNARY(!handler.hasEvent());
	}

	SUBCASE("testQueryNextEventType")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto mouseMoveEvent = std::make_shared<dggui::MouseMoveEvent>();
		mouseMoveEvent->x = 50;
		mouseMoveEvent->y = 50;
		nativeWindow.addEvent(mouseMoveEvent);

		CHECK_UNARY(handler.queryNextEventType(dggui::EventType::mouseMove));
		CHECK_UNARY(!handler.queryNextEventType(dggui::EventType::button));
	}

	SUBCASE("testGetNextEvent")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto mouseMoveEvent = std::make_shared<dggui::MouseMoveEvent>();
		mouseMoveEvent->x = 50;
		mouseMoveEvent->y = 50;
		nativeWindow.addEvent(mouseMoveEvent);

		auto event = handler.getNextEvent();
		CHECK_UNARY(event != nullptr);
		CHECK_EQ(event->type(), dggui::EventType::mouseMove);
	}

	SUBCASE("testGetNextEventEmpty")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto event = handler.getNextEvent();
		CHECK_UNARY(event == nullptr);
	}

	SUBCASE("testProcessEventsRepaint")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto repaintEvent = std::make_shared<dggui::RepaintEvent>();
		repaintEvent->x = 0;
		repaintEvent->y = 0;
		repaintEvent->width = 100;
		repaintEvent->height = 100;
		nativeWindow.addEvent(repaintEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsMove")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto moveEvent = std::make_shared<dggui::MoveEvent>();
		moveEvent->x = 10;
		moveEvent->y = 20;
		nativeWindow.addEvent(moveEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsResize")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto resizeEvent = std::make_shared<dggui::ResizeEvent>();
		resizeEvent->width = 200;
		resizeEvent->height = 150;
		nativeWindow.addEvent(resizeEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsResizeSameSize")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto resizeEvent1 = std::make_shared<dggui::ResizeEvent>();
		resizeEvent1->width = 100;
		resizeEvent1->height = 100;
		nativeWindow.addEvent(resizeEvent1);

		handler.processEvents();

		auto resizeEvent2 = std::make_shared<dggui::ResizeEvent>();
		resizeEvent2->width = 100;
		resizeEvent2->height = 100;
		nativeWindow.addEvent(resizeEvent2);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsButtonDown")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);
		widget.focusable_ = true;

		auto buttonEvent = std::make_shared<dggui::ButtonEvent>();
		buttonEvent->x = 50;
		buttonEvent->y = 50;
		buttonEvent->direction = dggui::Direction::down;
		buttonEvent->button = dggui::MouseButton::left;
		buttonEvent->doubleClick = false;
		nativeWindow.addEvent(buttonEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsButtonDoubleClick")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);

		auto buttonEvent1 = std::make_shared<dggui::ButtonEvent>();
		buttonEvent1->x = 50;
		buttonEvent1->y = 50;
		buttonEvent1->direction = dggui::Direction::down;
		buttonEvent1->button = dggui::MouseButton::left;
		buttonEvent1->doubleClick = true;
		nativeWindow.addEvent(buttonEvent1);

		handler.processEvents();

		auto buttonEvent2 = std::make_shared<dggui::ButtonEvent>();
		buttonEvent2->x = 50;
		buttonEvent2->y = 50;
		buttonEvent2->direction = dggui::Direction::down;
		buttonEvent2->button = dggui::MouseButton::left;
		buttonEvent2->doubleClick = false;
		nativeWindow.addEvent(buttonEvent2);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsButtonUp")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);
		widget.catch_mouse_ = true;

		auto buttonDownEvent = std::make_shared<dggui::ButtonEvent>();
		buttonDownEvent->x = 50;
		buttonDownEvent->y = 50;
		buttonDownEvent->direction = dggui::Direction::down;
		buttonDownEvent->button = dggui::MouseButton::left;
		buttonDownEvent->doubleClick = false;
		nativeWindow.addEvent(buttonDownEvent);

		handler.processEvents();

		auto buttonUpEvent = std::make_shared<dggui::ButtonEvent>();
		buttonUpEvent->x = 50;
		buttonUpEvent->y = 50;
		buttonUpEvent->direction = dggui::Direction::up;
		buttonUpEvent->button = dggui::MouseButton::left;
		buttonUpEvent->doubleClick = false;
		nativeWindow.addEvent(buttonUpEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsMouseMove")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);

		auto moveEvent = std::make_shared<dggui::MouseMoveEvent>();
		moveEvent->x = 50;
		moveEvent->y = 50;
		nativeWindow.addEvent(moveEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsScroll")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);

		auto scrollEvent = std::make_shared<dggui::ScrollEvent>();
		scrollEvent->x = 50;
		scrollEvent->y = 50;
		scrollEvent->delta = 0.1f;
		nativeWindow.addEvent(scrollEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsKey")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);
		widget.focusable_ = true;

		window.setKeyboardFocus(&widget);

		auto keyEvent = std::make_shared<dggui::KeyEvent>();
		keyEvent->direction = dggui::Direction::down;
		keyEvent->keycode = dggui::Key::character;
		keyEvent->text = "a";
		nativeWindow.addEvent(keyEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsClose")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		auto closeEvent = std::make_shared<dggui::CloseEvent>();
		nativeWindow.addEvent(closeEvent);

		bool closeNotified = false;
		handler.closeNotifier.connect(
		    [&closeNotified]() { closeNotified = true; });

		handler.processEvents();
	}

	SUBCASE("testProcessEventsMouseEnter")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);

		auto enterEvent = std::make_shared<dggui::MouseEnterEvent>();
		enterEvent->x = 50;
		enterEvent->y = 50;
		nativeWindow.addEvent(enterEvent);

		handler.processEvents();
	}

	SUBCASE("testProcessEventsMouseLeave")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		TestWidget widget(&window);
		widget.resize(100, 100);

		window.setMouseFocus(&widget);

		auto leaveEvent = std::make_shared<dggui::MouseLeaveEvent>();
		nativeWindow.addEvent(leaveEvent);

		handler.processEvents();
	}

	SUBCASE("testRegisterUnregisterDialog")
	{
		MockNativeWindow nativeWindow;
		TestWindow window;
		dggui::EventHandler handler(nativeWindow, window);

		dggui::Dialog* dialogPtr = nullptr;
		{
			class TestDialog : public dggui::Dialog
			{
			public:
				explicit TestDialog(dggui::Widget* parent)
				    : dggui::Dialog(parent)
				{
				}

				dggui::PixelBufferAlpha& getPixelBuffer() override
				{
					return pixbuf;
				}

				dggui::PixelBufferAlpha pixbuf{100, 100};
			};

			TestDialog dialog(&window);
			dialogPtr = &dialog;
			handler.registerDialog(&dialog);
			handler.unregisterDialog(&dialog);
		}
		(void)dialogPtr;
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

	dggui::PixelBufferAlpha pixbuf{100, 100};
};

TestDialog dialog(&window);
handler.registerDialog(&dialog);
handler.unregisterDialog(&dialog);
}

SUBCASE("testMultipleMouseMoveEvents")
{
	MockNativeWindow nativeWindow;
	TestWindow window;
	dggui::EventHandler handler(nativeWindow, window);

	TestWidget widget(&window);
	widget.resize(100, 100);

	auto moveEvent1 = std::make_shared<dggui::MouseMoveEvent>();
	moveEvent1->x = 10;
	moveEvent1->y = 10;
	nativeWindow.addEvent(moveEvent1);

	auto moveEvent2 = std::make_shared<dggui::MouseMoveEvent>();
	moveEvent2->x = 20;
	moveEvent2->y = 20;
	nativeWindow.addEvent(moveEvent2);

	auto moveEvent3 = std::make_shared<dggui::MouseMoveEvent>();
	moveEvent3->x = 30;
	moveEvent3->y = 30;
	nativeWindow.addEvent(moveEvent3);

	handler.processEvents();
}
}