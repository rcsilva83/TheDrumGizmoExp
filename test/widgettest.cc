/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            widgettest.cc
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

#include <dggui/canvas.h>
#include <dggui/painter.h>
#include <dggui/widget.h>

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

class TestableCanvas : public dggui::Canvas
{
public:
	TestableCanvas(std::size_t width, std::size_t height)
	    : pixbuf(width, height)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

private:
	dggui::PixelBufferAlpha pixbuf;
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

	void testRepaintEvent(dggui::RepaintEvent* repaintEvent)
	{
		repaintEvent(repaintEvent);
	}
};

class MockWidget : public dggui::Widget
{
public:
	explicit MockWidget(dggui::Widget* parent) : dggui::Widget(parent)
	{
	}

	dggui::PixelBufferAlpha& getPixelBuffer() override
	{
		return pixbuf;
	}

	int repaintEventCount = 0;
	int mouseMoveEventCount = 0;
	int buttonEventCount = 0;
	int scrollEventCount = 0;

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
};

TEST_CASE("WidgetTest")
{
	SUBCASE("testWidgetCreation")
	{
		TestWindow window;
		TestWidget widget(&window);

		CHECK_EQ(widget.x(), 0);
		CHECK_EQ(widget.y(), 0);
		CHECK_EQ(widget.width(), 0u);
		CHECK_EQ(widget.height(), 0u);
		CHECK_UNARY(widget.visible());
	}

	SUBCASE("testSetVisible")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.setVisible(false);
		CHECK_UNARY(!widget.visible());

		widget.setVisible(true);
		CHECK_UNARY(widget.visible());
	}

	SUBCASE("testShowHide")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.hide();
		CHECK_UNARY(!widget.visible());

		widget.show();
		CHECK_UNARY(widget.visible());
	}

	SUBCASE("testResize")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.resize(100, 50);
		CHECK_EQ(widget.width(), 100u);
		CHECK_EQ(widget.height(), 50u);
	}

	SUBCASE("testResizeSameSize")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.resize(100, 50);
		widget.resize(100, 50);
		CHECK_EQ(widget.width(), 100u);
		CHECK_EQ(widget.height(), 50u);
	}

	SUBCASE("testResizeZero")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.resize(0, 0);
		CHECK_EQ(widget.width(), 0u);
		CHECK_EQ(widget.height(), 0u);
	}

	SUBCASE("testMove")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.move(10, 20);
		CHECK_EQ(widget.x(), 10);
		CHECK_EQ(widget.y(), 20);
	}

	SUBCASE("testMoveSamePosition")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.move(10, 20);
		widget.move(10, 20);
		CHECK_EQ(widget.x(), 10);
		CHECK_EQ(widget.y(), 20);
	}

	SUBCASE("testPosition")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.move(15, 25);
		auto pos = widget.position();
		CHECK_EQ(pos.x, 15);
		CHECK_EQ(pos.y, 25);
	}

	SUBCASE("testAddChild")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		child.resize(50, 50);

		CHECK_EQ(parent.children.size(), 1u);
	}

	SUBCASE("testRemoveChild")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget* child = new TestWidget(&parent);

		parent.resize(100, 100);
		child->resize(50, 50);

		CHECK_EQ(parent.children.size(), 1u);

		parent.removeChild(child);
		delete child;
	}

	SUBCASE("testReparent")
	{
		TestWindow window;
		TestWidget parent1(&window);
		TestWidget parent2(&window);
		TestWidget child(&parent1);

		parent1.resize(100, 100);
		parent2.resize(100, 100);
		child.resize(50, 50);

		CHECK_EQ(child.parent, &parent1);
		CHECK_EQ(parent1.children.size(), 1u);
		CHECK_EQ(parent2.children.size(), 0u);

		child.reparent(&parent2);

		CHECK_EQ(child.parent, &parent2);
		CHECK_EQ(parent1.children.size(), 0u);
		CHECK_EQ(parent2.children.size(), 1u);
	}

	SUBCASE("testReparentToSame")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		child.resize(50, 50);

		child.reparent(&parent);
		CHECK_EQ(child.parent, &parent);
	}

	SUBCASE("testReparentToNull")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		child.resize(50, 50);

		child.reparent(nullptr);
		CHECK_EQ(child.parent, nullptr);
	}

	SUBCASE("testFind")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		parent.move(0, 0);
		child.resize(50, 50);
		child.move(10, 10);

		auto found = parent.find(25, 25);
		CHECK_EQ(found, &child);
	}

	SUBCASE("testFindOutsideBounds")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		child.resize(50, 50);

		auto found = parent.find(100, 100);
		CHECK_EQ(found, &parent);
	}

	SUBCASE("testGetPixelBuffers")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.resize(100, 100);
		widget.getPixelBuffers();
	}

	SUBCASE("testPixelBufferVisible")
	{
		TestWindow window;
		TestWidget widget(&window);

		widget.resize(100, 100);
		widget.hide();
		CHECK_UNARY(!widget.getPixelBuffer().visible);

		widget.show();
		CHECK_UNARY(widget.getPixelBuffer().visible);
	}

	SUBCASE("testTranslateToWindowX")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		parent.move(0, 0);
		child.resize(50, 50);
		child.move(10, 0);

		CHECK_EQ(child.translateToWindowX(), 10u);
	}

	SUBCASE("testTranslateToWindowY")
	{
		TestWindow window;
		TestWidget parent(&window);
		TestWidget child(&parent);

		parent.resize(100, 100);
		parent.move(0, 0);
		child.resize(50, 50);
		child.move(0, 10);

		CHECK_EQ(child.translateToWindowY(), 10u);
	}

	SUBCASE("testWidgetDestructorRemovesFromParent")
	{
		TestWindow window;
		TestWidget parent(&window);

		parent.resize(100, 100);
		{
			TestWidget child(&parent);
			child.resize(50, 50);
			CHECK_EQ(parent.children.size(), 1u);
		}
		CHECK_EQ(parent.children.size(), 0u);
	}
}