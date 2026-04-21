/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            paintertest.cc
 *
 *  Fri Nov 29 18:08:57 CET 2013
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

#include <ostream>

#include <dggui/canvas.h>
#include <dggui/font.h>
#include <dggui/image.h>
#include <dggui/painter.h>

class TestColour
{
public:
	TestColour(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	    : colour(r, g, b, a)
	{
	}
	// cppcheck-suppress noExplicitConstructor
	TestColour(const dggui::Colour& colour) : colour(colour)
	{
	}

	bool operator==(const TestColour& other) const
	{
		return !(*this != other);
	}

	bool operator!=(const TestColour& other) const
	{
		return colour.red() != other.colour.red() ||
		       colour.green() != other.colour.green() ||
		       colour.blue() != other.colour.blue() ||
		       colour.alpha() != other.colour.alpha();
	}

	const dggui::Colour colour;
};

std::ostream& operator<<(std::ostream& stream, const TestColour& col)
{
	stream << "(" << static_cast<int>(col.colour.red()) << ", "
	       << static_cast<int>(col.colour.green()) << ", "
	       << static_cast<int>(col.colour.blue()) << ", "
	       << static_cast<int>(col.colour.alpha()) << ")";
	return stream;
}

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

class TestImage : public dggui::Image
{
public:
	TestImage(std::uint8_t width, std::uint8_t height, bool alpha)
	    : dggui::Image(":resources/logo.png") // just load some default image
	{
		_width = width;
		_height = height;
		has_alpha = alpha;

		image_data.resize(_width * _height);
		image_data_raw.resize(_width * _height * 4);

		// Store x and y coordinates as red and green colour components
		for(std::uint8_t x = 0; x < _width; ++x)
		{
			for(std::uint8_t y = 0; y < _height; ++y)
			{
				image_data[x + _width * y] = dggui::Colour(std::uint8_t(x),
				    std::uint8_t(y), std::uint8_t(0),
				    alpha ? std::uint8_t(128) : std::uint8_t(255));
				image_data_raw[4 * (x + _width * y) + 0] = x;
				image_data_raw[4 * (x + _width * y) + 1] = y;
				image_data_raw[4 * (x + _width * y) + 2] = 0;
				image_data_raw[4 * (x + _width * y) + 3] = alpha ? 128 : 255;
			}
		}

		valid = true;
	}

	// Helper to set pixel data for testing
	void setPixel(std::size_t x, std::size_t y, const dggui::Colour& c)
	{
		if(x < _width && y < _height)
		{
			image_data[x + _width * y] = c;
			image_data_raw[4 * (x + _width * y) + 0] = c.red();
			image_data_raw[4 * (x + _width * y) + 1] = c.green();
			image_data_raw[4 * (x + _width * y) + 2] = c.blue();
			image_data_raw[4 * (x + _width * y) + 3] = c.alpha();
		}
	}

	// Helper to fill raw data for testing alpha with line path
	void fillRawData(
	    std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a)
	{
		for(std::size_t i = 0; i < image_data_raw.size(); i += 4)
		{
			image_data_raw[i + 0] = r;
			image_data_raw[i + 1] = g;
			image_data_raw[i + 2] = b;
			image_data_raw[i + 3] = a;
		}
	}
};

// Returns true if the pixel buffer contains at least one non-transparent pixel.
static bool hasAnyDrawnPixels(TestableCanvas& canvas)
{
	const auto& pixbuf = canvas.getPixelBuffer();
	for(std::size_t x = 0; x < pixbuf.width; ++x)
	{
		for(std::size_t y = 0; y < pixbuf.height; ++y)
		{
			const auto& c = pixbuf.pixel(x, y);
			if(c.red() > 0 || c.green() > 0 || c.blue() > 0 || c.alpha() > 0)
			{
				return true;
			}
		}
	}
	return false;
}

TEST_CASE("PainterTest")
{
	SUBCASE("testDrawImage")
	{
		dggui::Image image(":resources/logo.png");
		REQUIRE(image.isValid());

		{ // Image fits in pixelbuffer - something must be drawn
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(0, 0, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image fits, negative offset - part is still in bounds
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(-10, -10, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image too big for pixelbuffer - clipped, but still draws into it
			TestableCanvas canvas(image.width() / 2, image.height() / 2);
			dggui::Painter painter(canvas);
			painter.drawImage(0, 0, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image fits in pixelbuffer but offset so it is drawn over the edge.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(10, 10, image);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Image is offset to the right and down so nothing is to be drawn.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(image.width() + 1, image.height() + 1, image);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Image is offset to the left and up so nothing is to be drawn.
			TestableCanvas canvas(image.width(), image.height());
			dggui::Painter painter(canvas);
			painter.drawImage(
			    -1 * (image.width() + 1), -1 * (image.height() + 1), image);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawText")
	{
		dggui::Font font;
		// a string with unicode characters
		std::string someText = "Hello World - лæ Библиотека";
		std::size_t width = font.textWidth(someText);
		std::size_t height = font.textHeight(someText);

		{ // Text fits in pixelbuffer - glyphs must be rendered into the buffer
			// drawText y is the baseline; passing y=height places it at the
			// bottom of the canvas so the full text is rendered inside it.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(0, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text fits, negative x offset - part is still in bounds
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(-10, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text too big for pixelbuffer - clipped, but still draws into it
			// Canvas is half-size; pass y=height/2 as baseline so the bottom
			// half of the text is visible in the smaller canvas.
			TestableCanvas canvas(width / 2, height / 2);
			dggui::Painter painter(canvas);
			painter.drawText(0, height / 2, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text fits in pixelbuffer but offset so it is drawn over the edge.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(10, height, font, someText);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Text is offset to the right and down so nothing is to be drawn.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(width + 1, height + 1, font, someText);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}

		{ // Text is offset to the left and up so nothing is to be drawn.
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(
			    // cppcheck-suppress signConversion
			    -1 * (width + 1), -1 * (height + 1), font, someText);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	// Test rendering images outside the container is being clipped correctly.
	SUBCASE("testClipping")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);

		{ // Without alpha
			TestImage image(16, 16, false);
			painter.clear();
			painter.drawImage(-10, -10, image);
			auto& pixbuf = canvas.getPixelBuffer();

			// Top left corner pixel should have the RGBA value (10, 10, 0, 255)
			CHECK_EQ(
			    TestColour(10, 10, 0, 255), TestColour(pixbuf.pixel(0, 0)));
		}

		{ // With alpha (different pipeline)
			TestImage image(16, 16, true);
			painter.clear();
			painter.drawImage(-10, -10, image);
			auto& pixbuf = canvas.getPixelBuffer();

			// Top left corner pixel should have the RGBA value (10, 10, 0, 128)
			CHECK_EQ(
			    TestColour(10, 10, 0, 128), TestColour(pixbuf.pixel(0, 0)));
		}
	}

	SUBCASE("testSetColour")
	{
		TestableCanvas canvas(10, 10);
		dggui::Painter painter(canvas);

		dggui::Colour red(1.0f, 0.0f, 0.0f, 1.0f);
		painter.setColour(red);
		painter.drawPoint(5, 5);

		auto& pixbuf = canvas.getPixelBuffer();
		CHECK_EQ(TestColour(255, 0, 0, 255), TestColour(pixbuf.pixel(5, 5)));

		// Change colour and draw again
		dggui::Colour blue(0.0f, 0.0f, 1.0f, 1.0f);
		painter.setColour(blue);
		painter.drawPoint(6, 6);

		CHECK_EQ(TestColour(0, 0, 255, 255), TestColour(pixbuf.pixel(6, 6)));
	}

	SUBCASE("testClear")
	{
		TestableCanvas canvas(10, 10);
		dggui::Painter painter(canvas);

		// Draw something first
		painter.drawFilledRectangle(0, 0, 9, 9);
		CHECK_UNARY(hasAnyDrawnPixels(canvas));

		// Clear should remove everything
		painter.clear();
		CHECK_UNARY(!hasAnyDrawnPixels(canvas));

		// Verify a specific pixel is transparent
		auto& pixbuf = canvas.getPixelBuffer();
		CHECK_EQ(TestColour(0, 0, 0, 0), TestColour(pixbuf.pixel(5, 5)));
	}

	SUBCASE("testDrawPoint")
	{
		TestableCanvas canvas(10, 10);
		dggui::Painter painter(canvas);

		dggui::Colour green(0.0f, 1.0f, 0.0f, 1.0f);
		painter.setColour(green);

		{ // Draw a point inside bounds
			painter.drawPoint(5, 5);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_EQ(
			    TestColour(0, 255, 0, 255), TestColour(pixbuf.pixel(5, 5)));
		}

		{ // Draw at edge (0,0)
			painter.drawPoint(0, 0);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_EQ(
			    TestColour(0, 255, 0, 255), TestColour(pixbuf.pixel(0, 0)));
		}

		{ // Draw at opposite edge
			painter.drawPoint(9, 9);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_EQ(
			    TestColour(0, 255, 0, 255), TestColour(pixbuf.pixel(9, 9)));
		}

		{ // Draw outside bounds (should be clipped)
			painter.clear();
			painter.drawPoint(-1, 5);
			painter.drawPoint(5, -1);
			painter.drawPoint(100, 5);
			painter.drawPoint(5, 100);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawLine")
	{
		TestableCanvas canvas(50, 50);
		dggui::Painter painter(canvas);
		dggui::Colour white(1.0f, 1.0f, 1.0f, 1.0f);
		painter.setColour(white);

		{ // Horizontal line
			painter.clear();
			painter.drawLine(10, 25, 40, 25);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(10, 25).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(25, 25).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(40, 25).alpha() > 0);
		}

		{ // Vertical line
			painter.clear();
			painter.drawLine(25, 10, 25, 40);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(25, 10).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(25, 25).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(25, 40).alpha() > 0);
		}

		{ // Diagonal line (steep)
			painter.clear();
			painter.drawLine(10, 10, 40, 40);
			auto& pixbuf = canvas.getPixelBuffer();
			// Start and end should have something drawn
			CHECK_UNARY(pixbuf.pixel(10, 10).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(40, 40).alpha() > 0);
		}

		{ // Diagonal line (shallow)
			painter.clear();
			painter.drawLine(10, 40, 40, 10);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(10, 40).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(40, 10).alpha() > 0);
		}

		{ // Line going backwards (x1 > x2)
			painter.clear();
			painter.drawLine(40, 25, 10, 25);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(40, 25).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(10, 25).alpha() > 0);
		}

		{ // Line going backwards (y1 > y2)
			painter.clear();
			painter.drawLine(25, 40, 25, 10);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(25, 40).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(25, 10).alpha() > 0);
		}

		{ // Very short line (single point)
			painter.clear();
			painter.drawLine(25, 25, 25, 25);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(25, 25).alpha() > 0);
		}
	}

	SUBCASE("testDrawRectangle")
	{
		TestableCanvas canvas(50, 50);
		dggui::Painter painter(canvas);
		dggui::Colour red(1.0f, 0.0f, 0.0f, 1.0f);
		painter.setColour(red);

		painter.drawRectangle(10, 10, 40, 40);
		auto& pixbuf = canvas.getPixelBuffer();

		// Corners should have pixels
		CHECK_UNARY(pixbuf.pixel(10, 10).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(40, 10).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(10, 40).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(40, 40).alpha() > 0);

		// Center should be empty (not filled)
		CHECK_EQ(TestColour(0, 0, 0, 0), TestColour(pixbuf.pixel(25, 25)));

		// Edges should have pixels
		CHECK_UNARY(pixbuf.pixel(25, 10).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(10, 25).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(40, 25).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(25, 40).alpha() > 0);
	}

	SUBCASE("testDrawFilledRectangle")
	{
		TestableCanvas canvas(50, 50);
		dggui::Painter painter(canvas);
		dggui::Colour blue(0.0f, 0.0f, 1.0f, 1.0f);
		painter.setColour(blue);

		painter.drawFilledRectangle(10, 10, 40, 40);
		auto& pixbuf = canvas.getPixelBuffer();

		// Corners should be filled
		CHECK_UNARY(pixbuf.pixel(10, 10).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(40, 10).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(10, 40).alpha() > 0);
		CHECK_UNARY(pixbuf.pixel(40, 40).alpha() > 0);

		// Center should also be filled
		CHECK_UNARY(pixbuf.pixel(25, 25).alpha() > 0);

		// Inside pixels should all be blue
		CHECK_EQ(TestColour(0, 0, 255, 255), TestColour(pixbuf.pixel(25, 25)));
	}

	SUBCASE("testDrawCircle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		dggui::Colour white(1.0f, 1.0f, 1.0f, 1.0f);
		painter.setColour(white);

		{ // Circle with integer radius
			painter.clear();
			painter.drawCircle(50, 50, 20.0);
			auto& pixbuf = canvas.getPixelBuffer();

			// Points on the circle should be drawn
			CHECK_UNARY(pixbuf.pixel(50, 30).alpha() > 0); // Top
			CHECK_UNARY(pixbuf.pixel(50, 70).alpha() > 0); // Bottom
			CHECK_UNARY(pixbuf.pixel(30, 50).alpha() > 0); // Left
			CHECK_UNARY(pixbuf.pixel(70, 50).alpha() > 0); // Right

			// Center should be empty
			CHECK_EQ(TestColour(0, 0, 0, 0), TestColour(pixbuf.pixel(50, 50)));
		}

		{ // Very small circle (radius 1)
			painter.clear();
			painter.drawCircle(50, 50, 1.0);
			// Should at least draw the center point
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Circle with fractional radius
			painter.clear();
			painter.drawCircle(50, 50, 5.5);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Circle at edge of canvas
			painter.clear();
			painter.drawCircle(10, 10, 15.0);
			// Should draw partial circle without crashing
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawFilledCircle")
	{
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);
		dggui::Colour green(0.0f, 1.0f, 0.0f, 1.0f);
		painter.setColour(green);

		{ // Filled circle
			painter.clear();
			painter.drawFilledCircle(50, 50, 20);
			auto& pixbuf = canvas.getPixelBuffer();

			// Center should be filled
			CHECK_EQ(
			    TestColour(0, 255, 0, 255), TestColour(pixbuf.pixel(50, 50)));

			// Points on the edge should also be filled
			CHECK_UNARY(pixbuf.pixel(50, 30).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(30, 50).alpha() > 0);
		}

		{ // Small filled circle
			painter.clear();
			painter.drawFilledCircle(50, 50, 5);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_EQ(
			    TestColour(0, 255, 0, 255), TestColour(pixbuf.pixel(50, 50)));
		}

		{ // Filled circle at edge
			painter.clear();
			painter.drawFilledCircle(10, 10, 15);
			// Should draw partial filled circle without crashing
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawImageStretched")
	{
		dggui::Image image(":resources/logo.png");
		REQUIRE(image.isValid());

		{ // Stretch image to larger size
			TestableCanvas canvas(200, 200);
			dggui::Painter painter(canvas);
			painter.drawImageStretched(0, 0, image, 200, 200);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Stretch image to smaller size
			TestableCanvas canvas(50, 50);
			dggui::Painter painter(canvas);
			painter.drawImageStretched(0, 0, image, 50, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Stretch with negative offset
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawImageStretched(-10, -10, image, 100, 100);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Stretch with offset that goes out of bounds
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawImageStretched(50, 50, image, 100, 100);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Zero or negative dimensions (should return early)
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawImageStretched(0, 0, image, 0, 100);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawRestrictedImage")
	{
		TestableCanvas canvas(50, 50);
		dggui::Painter painter(canvas);

		// Create a test image with specific colors
		TestImage image(16, 16, false);

		// Draw with restriction to only black pixels
		dggui::Colour restriction(std::uint8_t(0), std::uint8_t(0),
		    std::uint8_t(0), std::uint8_t(255));
		painter.drawRestrictedImage(0, 0, restriction, image);

		// Pixel at (0,0) in TestImage has colour (0, 0, 0, 255) which is black,
		// so it matches the restriction and should be drawn
		auto& pixbuf = canvas.getPixelBuffer();
		CHECK_EQ(TestColour(0, 0, 0, 255), TestColour(pixbuf.pixel(0, 0)));

		// Now create an image with some black pixels and test again
		TestImage image2(16, 16, false);
		// Set first pixel to black - using TestImage's public interface
		image2.setPixel(0, 0,
		    dggui::Colour(std::uint8_t(0), std::uint8_t(0), std::uint8_t(0),
		        std::uint8_t(255)));

		painter.clear();
		painter.drawRestrictedImage(0, 0, restriction, image2);

		// The black pixel should now be drawn
		CHECK_EQ(TestColour(0, 0, 0, 255), TestColour(pixbuf.pixel(0, 0)));

		{ // Test with offset
			painter.clear();
			painter.drawRestrictedImage(10, 10, restriction, image2);
			CHECK_EQ(
			    TestColour(0, 0, 0, 255), TestColour(pixbuf.pixel(10, 10)));
		}

		{ // Test clipping with negative offset
			painter.clear();
			painter.drawRestrictedImage(-5, -5, restriction, image2);
			// Should still work without crashing
			// The pixel at (0,0) in image should be at (-5,-5) but clipped
		}

		{ // Test with dimensions that cause early return
			painter.clear();
			painter.drawRestrictedImage(100, 100, restriction, image2);
			CHECK_UNARY(!hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawBox")
	{
		// Create a Box with test images
		dggui::Image corner(":resources/logo.png");
		REQUIRE(corner.isValid());

		dggui::Painter::Box box;
		box.topLeft = &corner;
		box.top = &corner;
		box.topRight = &corner;
		box.left = &corner;
		box.right = &corner;
		box.bottomLeft = &corner;
		box.bottom = &corner;
		box.bottomRight = &corner;
		box.center = &corner;

		{ // Draw box at normal size
			TestableCanvas canvas(400, 400);
			dggui::Painter painter(canvas);
			painter.drawBox(10, 10, box, 300, 300);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Draw box with negative coordinates (early returns)
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawBox(-50, -50, box, 100, 100);
			// Should handle gracefully
		}

		{ // Draw very small box
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawBox(10, 10, box, 20, 20);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawBar")
	{
		// Create a Bar with test images
		dggui::Image segment(":resources/logo.png");
		REQUIRE(segment.isValid());

		dggui::Painter::Bar bar;
		bar.left = &segment;
		bar.right = &segment;
		bar.center = &segment;

		{ // Draw bar at normal size
			TestableCanvas canvas(400, 100);
			dggui::Painter painter(canvas);
			painter.drawBar(10, 10, bar, 300, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Draw bar smaller than left+right width
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			int small_width = segment.width() + segment.width() - 10;
			painter.drawBar(10, 10, bar, small_width, 50);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}

		{ // Draw bar at edge
			TestableCanvas canvas(100, 100);
			dggui::Painter painter(canvas);
			painter.drawBar(0, 0, bar, 100, 100);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawTemplate")
	{
		// Test the template draw function with a vector of points
		struct Point
		{
			int x;
			int y;
		};

		TestableCanvas canvas(50, 50);
		dggui::Painter painter(canvas);
		dggui::Colour red(1.0f, 0.0f, 0.0f, 1.0f);

		std::vector<Point> points = {{10, 10}, {20, 20}, {30, 30}, {40, 40}};

		painter.draw(points.begin(), points.end(), 0, 0, red);

		auto& pixbuf = canvas.getPixelBuffer();
		CHECK_EQ(TestColour(255, 0, 0, 255), TestColour(pixbuf.pixel(10, 10)));
		CHECK_EQ(TestColour(255, 0, 0, 255), TestColour(pixbuf.pixel(20, 20)));
		CHECK_EQ(TestColour(255, 0, 0, 255), TestColour(pixbuf.pixel(30, 30)));
		CHECK_EQ(TestColour(255, 0, 0, 255), TestColour(pixbuf.pixel(40, 40)));
	}

	SUBCASE("testDrawTextWithRotate")
	{
		dggui::Font font;

		{ // Draw rotated text
			std::string text = "AB";
			std::size_t width = font.textWidth(text);
			std::size_t height = font.textHeight(text);

			TestableCanvas canvas(width + height, width + height);
			dggui::Painter painter(canvas);
			painter.drawText(0, 0, font, text, false, true);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawTextWithNocolour")
	{
		dggui::Font font;
		std::string text = "Test";
		std::size_t width = font.textWidth(text);
		std::size_t height = font.textHeight(text);

		{ // Draw with nocolour flag
			TestableCanvas canvas(width, height);
			dggui::Painter painter(canvas);
			painter.drawText(0, height, font, text, true, false);
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}

	SUBCASE("testDrawImageWithAlphaAndLine")
	{
		// Test drawImage with alpha where image.line(0) is not null
		TestableCanvas canvas(100, 100);
		dggui::Painter painter(canvas);

		// Create test image with alpha
		TestImage image(16, 16, true);
		// Make sure line() returns valid data by filling image_data_raw
		image.fillRawData(128, 64, 32, 200);

		painter.drawImage(0, 0, image);
		auto& pixbuf = canvas.getPixelBuffer();
		// With the blendLine path, we should get blended pixels
		CHECK_UNARY(pixbuf.pixel(0, 0).alpha() > 0);
	}

	SUBCASE("testEdgeCases")
	{
		// Test various edge cases that might not be covered
		TestableCanvas canvas(10, 10);
		dggui::Painter painter(canvas);
		dggui::Colour white(1.0f, 1.0f, 1.0f, 1.0f);
		painter.setColour(white);

		{ // Filled rectangle covering entire canvas
			painter.drawFilledRectangle(0, 0, 9, 9);
			auto& pixbuf = canvas.getPixelBuffer();
			CHECK_UNARY(pixbuf.pixel(0, 0).alpha() > 0);
			CHECK_UNARY(pixbuf.pixel(9, 9).alpha() > 0);
		}

		{ // Rectangle at exact boundary
			painter.clear();
			painter.drawRectangle(0, 0, 9, 9);
			auto& pixbuf = canvas.getPixelBuffer();
			// Edges should have pixels
			CHECK_UNARY(pixbuf.pixel(0, 0).alpha() > 0);
		}

		{ // Circle at exact boundary
			painter.clear();
			painter.drawCircle(5, 5, 5.0);
			// Should not crash and should draw something
			CHECK_UNARY(hasAnyDrawnPixels(canvas));
		}
	}
}
