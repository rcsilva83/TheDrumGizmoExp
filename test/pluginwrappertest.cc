/* -*- Mode: c++ -*- */
/***************************************************************************
 *            pluginwrappertest.cc
 *
 *  Tue Mar 24 00:00:00 CET 2026
 *  Copyright 2026 DrumGizmo Team
 ****************************************************************************/

/*
 *  This file is part of DrumGizmo.
 *
 *  DrumGizmo is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
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

#include <string>

#include <settings.h>

#include "../plugin/configstringio.h"

//
// Helper: produce minimal valid XML config understood by ConfigStringIO::set
//
static std::string makeConfig(const std::string& name, const std::string& value)
{
	return "<config version=\"1.0\">"
	       "<value name=\"" +
	       name + "\">" + value + "</value></config>";
}

// ---------------------------------------------------------------------------
// State serialisation (get) tests
// ---------------------------------------------------------------------------

TEST_CASE("ConfigStringIO::get produces parseable XML")
{
	Settings s;
	ConfigStringIO io(s);

	const std::string xml = io.get();
	CHECK_UNARY(!xml.empty());
	// Must start with a valid config tag
	CHECK_UNARY(xml.find("<config") != std::string::npos);
	CHECK_UNARY(xml.find("</config>") != std::string::npos);
}

TEST_CASE("ConfigStringIO::get contains expected field names")
{
	Settings s;
	ConfigStringIO io(s);

	const std::string xml = io.get();
	// A representative sample of the keys that must appear
	CHECK_UNARY(xml.find("drumkitfile") != std::string::npos);
	CHECK_UNARY(xml.find("midimapfile") != std::string::npos);
	CHECK_UNARY(xml.find("enable_velocity_modifier") != std::string::npos);
	CHECK_UNARY(xml.find("disk_cache_enable") != std::string::npos);
	CHECK_UNARY(xml.find("enable_powermap") != std::string::npos);
	CHECK_UNARY(xml.find("voice_limit_max") != std::string::npos);
}

TEST_CASE("ConfigStringIO::get reflects current settings values")
{
	Settings s;
	s.drumkit_file.store("/my/drumkit.xml");
	s.midimap_file.store("/my/midimap.xml");
	s.enable_velocity_modifier.store(false);
	s.disk_cache_enable.store(false);

	ConfigStringIO io(s);
	const std::string xml = io.get();

	CHECK_UNARY(xml.find("/my/drumkit.xml") != std::string::npos);
	CHECK_UNARY(xml.find("/my/midimap.xml") != std::string::npos);
	// false → "false" in the XML
	CHECK_UNARY(xml.find(">false<") != std::string::npos);
}

// ---------------------------------------------------------------------------
// State deserialisation (set) tests
// ---------------------------------------------------------------------------

TEST_CASE("ConfigStringIO::set returns false for invalid XML")
{
	Settings s;
	ConfigStringIO io(s);

	// Unclosed tag makes the XML invalid
	CHECK_UNARY(!io.set("<config version=\"1.0\"><value name=\"foo\">bar</config>"));
}

TEST_CASE("ConfigStringIO::set returns false for empty string")
{
	Settings s;
	ConfigStringIO io(s);

	CHECK_UNARY(!io.set(""));
}

TEST_CASE("ConfigStringIO::set returns false for garbage input")
{
	Settings s;
	ConfigStringIO io(s);

	CHECK_UNARY(!io.set("this is not xml at all!!!"));
}

TEST_CASE("ConfigStringIO::set returns true for minimal valid config")
{
	Settings s;
	ConfigStringIO io(s);

	CHECK_UNARY(io.set("<config version=\"1.0\"></config>"));
}

TEST_CASE("ConfigStringIO::set returns false for unsupported config version")
{
	Settings s;
	ConfigStringIO io(s);

	CHECK_UNARY(!io.set("<config version=\"2.0\"><value name=\"drumkitfile\">/a.xml</value></config>"));
}

TEST_CASE("ConfigStringIO::set ignores unknown fields gracefully")
{
	Settings s;
	ConfigStringIO io(s);

	// Unknown field name must not cause a failure
	CHECK_UNARY(io.set("<config version=\"1.0\">"
	                   "<value name=\"completely_unknown_key\">42</value>"
	                   "</config>"));
}

// ---------------------------------------------------------------------------
// State roundtrip tests
// ---------------------------------------------------------------------------

TEST_CASE("ConfigStringIO state roundtrip preserves drumkitfile path")
{
	Settings s;
	s.drumkit_file.store("/path/to/kit.xml");

	ConfigStringIO io(s);
	const std::string saved = io.get();

	// Restore into a fresh settings object
	Settings s2;
	ConfigStringIO io2(s2);
	REQUIRE_UNARY(io2.set(saved));

	CHECK_EQ(std::string("/path/to/kit.xml"), s2.drumkit_file.load());
}

TEST_CASE("ConfigStringIO state roundtrip preserves midimapfile path")
{
	Settings s;
	s.midimap_file.store("/path/to/midimap.xml");

	ConfigStringIO io(s);
	const std::string saved = io.get();

	Settings s2;
	ConfigStringIO io2(s2);
	REQUIRE_UNARY(io2.set(saved));

	CHECK_EQ(std::string("/path/to/midimap.xml"), s2.midimap_file.load());
}

TEST_CASE("ConfigStringIO state roundtrip preserves boolean settings")
{
	Settings s;
	s.enable_velocity_modifier.store(false);
	s.enable_velocity_randomiser.store(true);
	s.disk_cache_enable.store(false);
	s.enable_bleed_control.store(true);
	s.enable_latency_modifier.store(true);
	s.enable_powermap.store(true);
	s.powermap_shelf.store(false);
	s.enable_voice_limit.store(true);
	s.enable_resampling.store(false);

	ConfigStringIO io(s);
	const std::string saved = io.get();

	Settings s2;
	ConfigStringIO io2(s2);
	REQUIRE_UNARY(io2.set(saved));

	CHECK_EQ(false, s2.enable_velocity_modifier.load());
	CHECK_EQ(true, s2.enable_velocity_randomiser.load());
	CHECK_EQ(false, s2.disk_cache_enable.load());
	CHECK_EQ(true, s2.enable_bleed_control.load());
	CHECK_EQ(true, s2.enable_latency_modifier.load());
	CHECK_EQ(true, s2.enable_powermap.load());
	CHECK_EQ(false, s2.powermap_shelf.load());
	CHECK_EQ(true, s2.enable_voice_limit.load());
	CHECK_EQ(false, s2.enable_resampling.load());
}

TEST_CASE("ConfigStringIO state roundtrip preserves float settings")
{
	Settings s;
	s.velocity_modifier_falloff.store(0.3f);
	s.velocity_modifier_weight.store(0.7f);
	s.master_bleed.store(0.5f);
	s.voice_limit_rampdown.store(0.25f);

	ConfigStringIO io(s);
	const std::string saved = io.get();

	Settings s2;
	ConfigStringIO io2(s2);
	REQUIRE_UNARY(io2.set(saved));

	CHECK_EQ(doctest::Approx(0.3f).epsilon(1e-4), s2.velocity_modifier_falloff.load());
	CHECK_EQ(doctest::Approx(0.7f).epsilon(1e-4), s2.velocity_modifier_weight.load());
	CHECK_EQ(doctest::Approx(0.5f).epsilon(1e-4), s2.master_bleed.load());
	CHECK_EQ(doctest::Approx(0.25f).epsilon(1e-4), s2.voice_limit_rampdown.load());
}

TEST_CASE("ConfigStringIO state roundtrip preserves integer settings")
{
	Settings s;
	s.disk_cache_upper_limit.store(512 * 1024 * 1024LL);
	s.disk_cache_chunk_size.store(2 * 1024 * 1024);
	s.voice_limit_max.store(20);

	ConfigStringIO io(s);
	const std::string saved = io.get();

	Settings s2;
	ConfigStringIO io2(s2);
	REQUIRE_UNARY(io2.set(saved));

	CHECK_EQ(static_cast<std::size_t>(512 * 1024 * 1024LL),
	         s2.disk_cache_upper_limit.load());
	CHECK_EQ(static_cast<std::size_t>(2 * 1024 * 1024),
	         s2.disk_cache_chunk_size.load());
	CHECK_EQ(static_cast<std::size_t>(20), s2.voice_limit_max.load());
}

// ---------------------------------------------------------------------------
// Partial / selective restore tests
// ---------------------------------------------------------------------------

TEST_CASE("ConfigStringIO::set only updates fields present in config")
{
	Settings s;
	const float original_falloff = 0.42f;
	s.velocity_modifier_falloff.store(original_falloff);

	ConfigStringIO io(s);

	// Config string that does NOT contain velocity_modifier_falloff
	const bool ok = io.set(makeConfig("enable_velocity_modifier", "false"));
	REQUIRE_UNARY(ok);

	// Untouched field must retain original value
	CHECK_EQ(doctest::Approx(original_falloff).epsilon(1e-4),
	         s.velocity_modifier_falloff.load());
	// Updated field must reflect the config
	CHECK_EQ(false, s.enable_velocity_modifier.load());
}

TEST_CASE("ConfigStringIO::set handles non-numeric float gracefully")
{
	Settings s;
	const float original = s.velocity_modifier_falloff.load();
	ConfigStringIO io(s);

	// Non-numeric value -> str2float returns 0.0
	REQUIRE_UNARY(io.set(makeConfig("velocity_modifier_falloff", "not_a_number")));
	CHECK_EQ(doctest::Approx(0.0f).epsilon(1e-6),
	         s.velocity_modifier_falloff.load());
	(void)original;
}

TEST_CASE("ConfigStringIO::set handles non-numeric int gracefully")
{
	Settings s;
	ConfigStringIO io(s);

	// Non-numeric value -> str2int returns 0
	REQUIRE_UNARY(io.set(makeConfig("voice_limit_max", "not_a_number")));
	CHECK_EQ(static_cast<std::size_t>(0), s.voice_limit_max.load());
}

// ---------------------------------------------------------------------------
// Multiple save/restore lifecycle transition tests
// ---------------------------------------------------------------------------

TEST_CASE("ConfigStringIO multiple sequential saves produce identical output")
{
	Settings s;
	s.drumkit_file.store("/kit.xml");
	s.enable_velocity_modifier.store(true);

	ConfigStringIO io(s);
	const std::string first = io.get();
	const std::string second = io.get();

	CHECK_EQ(first, second);
}

TEST_CASE("ConfigStringIO save after restore reflects restored values")
{
	Settings s;
	s.drumkit_file.store("/original.xml");

	ConfigStringIO io(s);
	const std::string original_saved = io.get();

	// Restore a different drumkitfile
	REQUIRE_UNARY(io.set(makeConfig("drumkitfile", "/restored.xml")));

	// Second save must now reflect the restored drumkitfile
	const std::string second_saved = io.get();
	CHECK_UNARY(second_saved.find("/restored.xml") != std::string::npos);
	CHECK_UNARY(second_saved.find("/original.xml") == std::string::npos);
}

TEST_CASE("ConfigStringIO successive restores accumulate changes")
{
	Settings s;
	ConfigStringIO io(s);

	REQUIRE_UNARY(io.set(makeConfig("enable_velocity_modifier", "false")));
	REQUIRE_UNARY(io.set(makeConfig("enable_velocity_randomiser", "true")));

	CHECK_EQ(false, s.enable_velocity_modifier.load());
	CHECK_EQ(true, s.enable_velocity_randomiser.load());
}
