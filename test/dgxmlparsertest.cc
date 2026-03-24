/* -*- Mode: c++ -*- */
/***************************************************************************
 *            dgxmlparsertest.cc
 *
 *  Sat Jun  9 11:37:19 CEST 2018
 *  Copyright 2018 Bent Bisballe Nyeng
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

#include <vector>

#include "scopedfile.h"
#include <dgxmlparser.h>

TEST_CASE("DGXmlParserTest")
{
	SUBCASE("instrumentParserTest_v1")
	{
		ScopedFile scoped_file(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<instrument name=\"Snare\" description=\"A nice snare\">\n"
		    " <samples>\n"
		    "  <sample name=\"Snare-1\">\n"
		    "   <audiofile channel=\"AmbLeft\" file=\"1-Snare-1.wav\"/>\n"
		    "   <audiofile channel=\"AmbRight\" file=\"1-Snare-2.wav\"/>\n"
		    "   <audiofile channel=\"SnareBottom\" file=\"1-Snare-3.wav\"/>\n"
		    "   <audiofile channel=\"SnareTop\" file=\"1-Snare-4.wav\"/>\n"
		    "  </sample>\n"
		    "  <sample name=\"Snare-2\">\n"
		    "   <audiofile channel=\"AmbLeft\" file=\"2-Snare-1.wav\"/>\n"
		    "   <audiofile channel=\"AmbRight\" file=\"2-Snare-2.wav\"/>\n"
		    "   <audiofile channel=\"SnareBottom\" file=\"2-Snare-3.wav\"/>\n"
		    "   <audiofile channel=\"SnareTop\" file=\"2-Snare-4.wav\"/>\n"
		    "  </sample>\n"
		    " </samples>\n"
		    " <velocities>\n"
		    "  <velocity lower=\"0\" upper=\"0.6\">\n"
		    "   <sampleref probability=\"0.6\" name=\"Snare-1\"/>\n"
		    "   <sampleref probability=\"0.4\" name=\"Snare-2\"/>\n"
		    "  </velocity>"
		    "  <velocity lower=\"0.6\" upper=\"1.0\">"
		    "   <sampleref probability=\"0.4\" name=\"Snare-2\"/>"
		    "   <sampleref probability=\"0.6\" name=\"Snare-1\"/>"
		    "  </velocity>"
		    " </velocities>"
		    "</instrument>");

		InstrumentDOM dom;
		CHECK(probeInstrumentFile(scoped_file.filename()));
		CHECK(parseInstrumentFile(scoped_file.filename(), dom));

		CHECK_EQ(std::string("Snare"), dom.name);
		CHECK_EQ(std::string("1.0"), dom.version);
		CHECK_EQ(std::string("A nice snare"), dom.description);
		CHECK_EQ(std::size_t(2), dom.samples.size());

		{
			const auto& s = dom.samples[0];
			CHECK_EQ(std::string("Snare-1"), s.name);
			CHECK_EQ(std::size_t(4), s.audiofiles.size());

			CHECK_EQ(
			    std::string("AmbLeft"), s.audiofiles[0].instrument_channel);
			CHECK_EQ(std::string("1-Snare-1.wav"), s.audiofiles[0].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[0].filechannel);

			CHECK_EQ(
			    std::string("AmbRight"), s.audiofiles[1].instrument_channel);
			CHECK_EQ(std::string("1-Snare-2.wav"), s.audiofiles[1].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[1].filechannel);

			CHECK_EQ(
			    std::string("SnareBottom"), s.audiofiles[2].instrument_channel);
			CHECK_EQ(std::string("1-Snare-3.wav"), s.audiofiles[2].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[2].filechannel);

			CHECK_EQ(
			    std::string("SnareTop"), s.audiofiles[3].instrument_channel);
			CHECK_EQ(std::string("1-Snare-4.wav"), s.audiofiles[3].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[3].filechannel);
		}

		{
			const auto& s = dom.samples[1];
			CHECK_EQ(std::string("Snare-2"), s.name);
			CHECK_EQ(std::size_t(4), s.audiofiles.size());
			CHECK_EQ(
			    std::string("AmbLeft"), s.audiofiles[0].instrument_channel);
			CHECK_EQ(std::string("2-Snare-1.wav"), s.audiofiles[0].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[0].filechannel);

			CHECK_EQ(
			    std::string("AmbRight"), s.audiofiles[1].instrument_channel);
			CHECK_EQ(std::string("2-Snare-2.wav"), s.audiofiles[1].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[1].filechannel);

			CHECK_EQ(
			    std::string("SnareBottom"), s.audiofiles[2].instrument_channel);
			CHECK_EQ(std::string("2-Snare-3.wav"), s.audiofiles[2].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[2].filechannel);

			CHECK_EQ(
			    std::string("SnareTop"), s.audiofiles[3].instrument_channel);
			CHECK_EQ(std::string("2-Snare-4.wav"), s.audiofiles[3].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[3].filechannel);
		}

		CHECK_EQ(std::size_t(0), dom.instrument_channels.size());

		CHECK_EQ(std::size_t(2), dom.velocities.size());
		{
			const auto& velocity = dom.velocities[0];
			CHECK_EQ(0.0, velocity.lower);
			CHECK_EQ(0.6, velocity.upper);
			CHECK_EQ(std::size_t(2), velocity.samplerefs.size());
			CHECK_EQ(std::string("Snare-1"), velocity.samplerefs[0].name);
			CHECK_EQ(0.6, velocity.samplerefs[0].probability);
			CHECK_EQ(std::string("Snare-2"), velocity.samplerefs[1].name);
			CHECK_EQ(0.4, velocity.samplerefs[1].probability);
		}

		{
			const auto& velocity = dom.velocities[1];
			CHECK_EQ(0.6, velocity.lower);
			CHECK_EQ(1.0, velocity.upper);
			CHECK_EQ(std::size_t(2), velocity.samplerefs.size());
			CHECK_EQ(std::string("Snare-2"), velocity.samplerefs[0].name);
			CHECK_EQ(0.4, velocity.samplerefs[0].probability);
			CHECK_EQ(std::string("Snare-1"), velocity.samplerefs[1].name);
			CHECK_EQ(0.6, velocity.samplerefs[1].probability);
		}
	}

	SUBCASE("instrumentParserTest_v2")
	{
		ScopedFile scoped_file(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<instrument version=\"2.0\" name=\"Snare\" description=\"A nice "
		    "snare\">\n"
		    " <channels>\n"
		    "  <channel name=\"AmbLeft\" main=\"true\"/>\n"
		    "  <channel name=\"AmbRight\" main=\"false\"/>\n"
		    "  <channel name=\"SnareBottom\"/>\n"
		    " </channels>\n"
		    " <samples>\n"
		    "  <sample name=\"Snare-1\" power=\"0.00985718\">\n"
		    "   <audiofile channel=\"AmbLeft\" file=\"1-Snare.wav\" "
		    "filechannel=\"1\"/>\n"
		    "   <audiofile channel=\"AmbRight\" file=\"1-Snare.wav\" "
		    "filechannel=\"2\"/>\n"
		    "   <audiofile channel=\"SnareBottom\" file=\"1-Snare.wav\" "
		    "filechannel=\"12\"/>\n"
		    "   <audiofile channel=\"SnareTop\" file=\"1-Snare.wav\" "
		    "filechannel=\"13\"/>\n"
		    "  </sample>\n"
		    "  <sample name=\"Snare-2\" power=\"0.0124808\">\n"
		    "   <audiofile channel=\"AmbLeft\" file=\"2-Snare.wav\" "
		    "filechannel=\"1\"/>\n"
		    "   <audiofile channel=\"AmbRight\" file=\"2-Snare.wav\" "
		    "filechannel=\"2\"/>\n"
		    "   <audiofile channel=\"SnareBottom\" file=\"2-Snare.wav\" "
		    "filechannel=\"12\"/>\n"
		    "   <audiofile channel=\"SnareTop\" file=\"2-Snare.wav\" "
		    "filechannel=\"13\"/>\n"
		    "  </sample>\n"
		    " </samples>\n"
		    "</instrument>");

		InstrumentDOM dom;
		CHECK(probeInstrumentFile(scoped_file.filename()));
		CHECK(parseInstrumentFile(scoped_file.filename(), dom));

		CHECK_EQ(std::string("Snare"), dom.name);
		CHECK_EQ(std::string("2.0"), dom.version);
		CHECK_EQ(std::string("A nice snare"), dom.description);
		CHECK_EQ(std::size_t(2), dom.samples.size());

		{
			const auto& s = dom.samples[0];
			CHECK_EQ(std::string("Snare-1"), s.name);
			CHECK_EQ(0.00985718, s.power);
			CHECK_EQ(std::size_t(4), s.audiofiles.size());

			CHECK_EQ(
			    std::string("AmbLeft"), s.audiofiles[0].instrument_channel);
			CHECK_EQ(std::string("1-Snare.wav"), s.audiofiles[0].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[0].filechannel);

			CHECK_EQ(
			    std::string("AmbRight"), s.audiofiles[1].instrument_channel);
			CHECK_EQ(std::string("1-Snare.wav"), s.audiofiles[1].file);
			CHECK_EQ(std::size_t(2), s.audiofiles[1].filechannel);

			CHECK_EQ(
			    std::string("SnareBottom"), s.audiofiles[2].instrument_channel);
			CHECK_EQ(std::string("1-Snare.wav"), s.audiofiles[2].file);
			CHECK_EQ(std::size_t(12), s.audiofiles[2].filechannel);

			CHECK_EQ(
			    std::string("SnareTop"), s.audiofiles[3].instrument_channel);
			CHECK_EQ(std::string("1-Snare.wav"), s.audiofiles[3].file);
			CHECK_EQ(std::size_t(13), s.audiofiles[3].filechannel);
		}

		{
			const auto& s = dom.samples[1];
			CHECK_EQ(std::string("Snare-2"), s.name);
			CHECK_EQ(0.0124808, s.power);
			CHECK_EQ(std::size_t(4), s.audiofiles.size());
			CHECK_EQ(
			    std::string("AmbLeft"), s.audiofiles[0].instrument_channel);
			CHECK_EQ(std::string("2-Snare.wav"), s.audiofiles[0].file);
			CHECK_EQ(std::size_t(1), s.audiofiles[0].filechannel);

			CHECK_EQ(
			    std::string("AmbRight"), s.audiofiles[1].instrument_channel);
			CHECK_EQ(std::string("2-Snare.wav"), s.audiofiles[1].file);
			CHECK_EQ(std::size_t(2), s.audiofiles[1].filechannel);

			CHECK_EQ(
			    std::string("SnareBottom"), s.audiofiles[2].instrument_channel);
			CHECK_EQ(std::string("2-Snare.wav"), s.audiofiles[2].file);
			CHECK_EQ(std::size_t(12), s.audiofiles[2].filechannel);

			CHECK_EQ(
			    std::string("SnareTop"), s.audiofiles[3].instrument_channel);
			CHECK_EQ(std::string("2-Snare.wav"), s.audiofiles[3].file);
			CHECK_EQ(std::size_t(13), s.audiofiles[3].filechannel);
		}

		CHECK_EQ(std::size_t(3), dom.instrument_channels.size());
		CHECK_EQ(std::string("AmbLeft"), dom.instrument_channels[0].name);
		CHECK(main_state_t::is_main == dom.instrument_channels[0].main);
		CHECK_EQ(std::string("AmbRight"), dom.instrument_channels[1].name);
		CHECK(main_state_t::is_not_main == dom.instrument_channels[1].main);
		CHECK_EQ(std::string("SnareBottom"), dom.instrument_channels[2].name);
		CHECK(main_state_t::unset == dom.instrument_channels[2].main);

		CHECK_EQ(std::size_t(0), dom.velocities.size());
	}

	SUBCASE("drumkitParserTest")
	{
		ScopedFile scoped_file(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<drumkit samplerate=\"48000\" version=\"2.0\">\n"
		    "  <metadata>\n"
		    "    <version>1.2.3</version>\n"
		    "    <title>Test Kit</title>\n"
		    "    <logo src=\"LogoFile.png\"/>\n"
		    "    <description>This is the description of the "
		    "drumkit</description>\n"
		    "    <license>Creative Commons</license>\n"
		    "    <notes>These are general notes</notes>\n"
		    "    <author>Author of the drumkit</author>\n"
		    "    <email>author@email.org</email>\n"
		    "    <website>http://www.drumgizmo.org</website>\n"
		    "    <image src=\"DrumkitImage.png\" "
		    "map=\"DrumkitImageClickMap.png\">\n"
		    "      <clickmap colour=\"ff08a2\" instrument=\"China\"/>\n"
		    "      <clickmap colour=\"a218d7\" instrument=\"HihatClosed\"/>\n"
		    "    </image>\n"
		    "  </metadata>\n"
		    "  <channels>\n"
		    "   <channel name=\"AmbLeft\"/>\n"
		    "   <channel name=\"AmbRight\"/>\n"
		    "   <channel name=\"SnareTop\"/>\n"
		    "   <channel name=\"SnareBottom\"/>\n"
		    "  </channels>\n"
		    "  <instruments>\n"
		    "    <instrument name=\"Snare1\" group=\"somegroup\" "
		    "file=\"foo.wav\">\n"
		    "      <channelmap in=\"AmbLeft-in\" out=\"AmbLeft\" "
		    "main=\"true\"/>\n"
		    "      <channelmap in=\"AmbRight-in\" out=\"AmbRight\" "
		    "main=\"true\"/>\n"
		    "      <channelmap in=\"SnareTop-in\" out=\"SnareTop\"/>\n"
		    "      <channelmap in=\"SnareBottom-in\" out=\"SnareBottom\"/>\n"
		    "    </instrument>\n"
		    "    <instrument name=\"Snare2\" file=\"bar.wav\">\n"
		    "      <channelmap in=\"AmbLeft2-in\" out=\"AmbLeft\" "
		    "main=\"false\"/>\n"
		    "      <channelmap in=\"AmbRight2-in\" out=\"AmbRight\" "
		    "main=\"false\"/>\n"
		    "      <channelmap in=\"SnareTop2-in\" out=\"SnareTop\"/>\n"
		    "      <channelmap in=\"SnareBottom2-in\" out=\"SnareBottom\"/>\n"
		    "    </instrument>\n"
		    "  </instruments>\n"
		    "</drumkit>");

		DrumkitDOM dom;
		CHECK(probeDrumkitFile(scoped_file.filename()));
		CHECK(parseDrumkitFile(scoped_file.filename(), dom));

		CHECK_EQ(std::string("2.0"), dom.version);
		CHECK_EQ(48000.0, dom.samplerate);

		CHECK_EQ(std::string("1.2.3"), dom.metadata.version);
		CHECK_EQ(std::string("Test Kit"), dom.metadata.title);
		CHECK_EQ(std::string("LogoFile.png"), dom.metadata.logo);
		CHECK_EQ(std::string("This is the description of the drumkit"),
		    dom.metadata.description);
		CHECK_EQ(std::string("Creative Commons"), dom.metadata.license);
		CHECK_EQ(std::string("These are general notes"), dom.metadata.notes);
		CHECK_EQ(std::string("Author of the drumkit"), dom.metadata.author);
		CHECK_EQ(std::string("author@email.org"), dom.metadata.email);
		CHECK_EQ(std::string("http://www.drumgizmo.org"), dom.metadata.website);
		CHECK_EQ(std::string("DrumkitImage.png"), dom.metadata.image);
		CHECK_EQ(
		    std::string("DrumkitImageClickMap.png"), dom.metadata.image_map);

		CHECK_EQ(std::size_t(2), dom.metadata.clickmaps.size());
		CHECK_EQ(std::string("ff08a2"), dom.metadata.clickmaps[0].colour);
		CHECK_EQ(std::string("China"), dom.metadata.clickmaps[0].instrument);
		CHECK_EQ(std::string("a218d7"), dom.metadata.clickmaps[1].colour);
		CHECK_EQ(
		    std::string("HihatClosed"), dom.metadata.clickmaps[1].instrument);

		CHECK_EQ(std::size_t(2), dom.instruments.size());
		{
			const auto& instr = dom.instruments[0];
			CHECK_EQ(std::string("Snare1"), instr.name);
			CHECK_EQ(std::string("foo.wav"), instr.file);
			CHECK_EQ(std::string("somegroup"), instr.group);
			CHECK_EQ(std::size_t(4), instr.channel_map.size());

			CHECK_EQ(std::string("AmbLeft-in"), instr.channel_map[0].in);
			CHECK_EQ(std::string("AmbRight-in"), instr.channel_map[1].in);
			CHECK_EQ(std::string("SnareTop-in"), instr.channel_map[2].in);
			CHECK_EQ(std::string("SnareBottom-in"), instr.channel_map[3].in);

			CHECK_EQ(std::string("AmbLeft"), instr.channel_map[0].out);
			CHECK_EQ(std::string("AmbRight"), instr.channel_map[1].out);
			CHECK_EQ(std::string("SnareTop"), instr.channel_map[2].out);
			CHECK_EQ(std::string("SnareBottom"), instr.channel_map[3].out);

			CHECK(main_state_t::is_main == instr.channel_map[0].main);
			CHECK(main_state_t::is_main == instr.channel_map[1].main);
			CHECK(main_state_t::unset == instr.channel_map[2].main);
			CHECK(main_state_t::unset == instr.channel_map[3].main);
		}
		{
			const auto& instr = dom.instruments[1];
			CHECK_EQ(std::string("Snare2"), instr.name);
			CHECK_EQ(std::string("bar.wav"), instr.file);
			CHECK_EQ(std::string(""), instr.group);
			CHECK_EQ(std::size_t(4), instr.channel_map.size());

			CHECK_EQ(std::string("AmbLeft2-in"), instr.channel_map[0].in);
			CHECK_EQ(std::string("AmbRight2-in"), instr.channel_map[1].in);
			CHECK_EQ(std::string("SnareTop2-in"), instr.channel_map[2].in);
			CHECK_EQ(std::string("SnareBottom2-in"), instr.channel_map[3].in);

			CHECK_EQ(std::string("AmbLeft"), instr.channel_map[0].out);
			CHECK_EQ(std::string("AmbRight"), instr.channel_map[1].out);
			CHECK_EQ(std::string("SnareTop"), instr.channel_map[2].out);
			CHECK_EQ(std::string("SnareBottom"), instr.channel_map[3].out);

			CHECK(main_state_t::is_not_main == instr.channel_map[0].main);
			CHECK(main_state_t::is_not_main == instr.channel_map[1].main);
			CHECK(main_state_t::unset == instr.channel_map[2].main);
			CHECK(main_state_t::unset == instr.channel_map[3].main);
		}

		CHECK_EQ(std::size_t(4), dom.channels.size());
		CHECK_EQ(std::string("AmbLeft"), dom.channels[0].name);
		CHECK_EQ(std::string("AmbRight"), dom.channels[1].name);
		CHECK_EQ(std::string("SnareTop"), dom.channels[2].name);
		CHECK_EQ(std::string("SnareBottom"), dom.channels[3].name);
	}

	SUBCASE("drumkitDefaultsAndFailureRecovery")
	{
		ScopedFile scoped_file(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<drumkit name=\"Legacy kit\" description=\"Legacy description\">\n"
		    "  <channels>\n"
		    "    <channel name=\"OH\"/>\n"
		    "  </channels>\n"
		    "  <instruments>\n"
		    "    <instrument name=\"Hat\" file=\"hat.xml\">\n"
		    "      <channelmap in=\"HatIn\" out=\"OH\"/>\n"
		    "      <chokes>\n"
		    "        <choke instrument=\"OpenHat\"/>\n"
		    "      </chokes>\n"
		    "    </instrument>\n"
		    "  </instruments>\n"
		    "</drumkit>");

		DrumkitDOM dom;
		CHECK(parseDrumkitFile(scoped_file.filename(), dom));

		CHECK_EQ(std::string("1.0"), dom.version);
		CHECK_EQ(44100.0, dom.samplerate);
		CHECK_EQ(std::string("Legacy kit"), dom.metadata.title);
		CHECK_EQ(std::string("Legacy description"), dom.metadata.description);
		CHECK_EQ(std::size_t(1), dom.channels.size());
		CHECK_EQ(std::size_t(1), dom.instruments.size());

		const auto& instrument = dom.instruments[0];
		CHECK_EQ(std::string(""), instrument.group);
		CHECK_EQ(std::size_t(1), instrument.channel_map.size());
		CHECK(main_state_t::unset == instrument.channel_map[0].main);
		CHECK_EQ(std::size_t(1), instrument.chokes.size());
		CHECK_EQ(std::string("OpenHat"), instrument.chokes[0].instrument);
		CHECK_EQ(68.0, instrument.chokes[0].choketime);
	}

	SUBCASE("drumkitEdgeCaseMatrix")
	{
		struct TestCase
		{
			std::string name;
			std::string xml;
			bool expected_status;
			std::string expected_version;
			double expected_samplerate;
			std::string expected_title;
			std::string expected_description;
			std::size_t expected_channels;
			std::size_t expected_instruments;
			bool expect_first_channel_main;
			main_state_t expected_main;
			bool expect_first_choke;
			double expected_choketime;
		};

		const std::vector<TestCase> cases = {
		    {"near-valid legacy defaults",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<drumkit name=\"Legacy kit\" description=\"Fallback\">\n"
		        "  <channels>\n"
		        "    <channel name=\"OH\"/>\n"
		        "  </channels>\n"
		        "  <instruments>\n"
		        "    <instrument name=\"Hat\" file=\"hat.xml\">\n"
		        "      <channelmap in=\"HatIn\" out=\"OH\"/>\n"
		        "      <chokes>\n"
		        "        <choke instrument=\"OpenHat\"/>\n"
		        "      </chokes>\n"
		        "    </instrument>\n"
		        "  </instruments>\n"
		        "</drumkit>",
		        true, "1.0", 44100.0, "Legacy kit", "Fallback", 1u, 1u, true,
		        main_state_t::unset, true, 68.0},
		    {"conflicting duplicate chokes nodes",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<drumkit>\n"
		        "  <channels>\n"
		        "    <channel name=\"OH\"/>\n"
		        "  </channels>\n"
		        "  <instruments>\n"
		        "    <instrument name=\"Hat\" file=\"hat.xml\">\n"
		        "      <channelmap in=\"HatIn\" out=\"OH\"/>\n"
		        "      <chokes><choke instrument=\"One\"/></chokes>\n"
		        "      <chokes><choke instrument=\"Two\"/></chokes>\n"
		        "    </instrument>\n"
		        "  </instruments>\n"
		        "</drumkit>",
		        false, "1.0", 44100.0, "", "", 1u, 1u, true,
		        main_state_t::unset, false, 0.0},
		    {"partial invalid main attribute fails with recovery default",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<drumkit>\n"
		        "  <channels>\n"
		        "    <channel name=\"OH\"/>\n"
		        "  </channels>\n"
		        "  <instruments>\n"
		        "    <instrument name=\"Hat\" file=\"hat.xml\">\n"
		        "      <channelmap in=\"HatIn\" out=\"OH\" main=\"maybe\"/>\n"
		        "    </instrument>\n"
		        "  </instruments>\n"
		        "</drumkit>",
		        false, "1.0", 44100.0, "", "", 1u, 1u, true,
		        main_state_t::unset, false, 0.0},
		    {"wrong root parses as empty drumkit",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<kit>\n"
		        "  <channels>\n"
		        "    <channel name=\"OH\"/>\n"
		        "  </channels>\n"
		        "</kit>",
		        true, "1.0", 44100.0, "", "", 0u, 0u, false,
		        main_state_t::unset, false, 0.0}};

		for(const auto& test_case : cases)
		{
			INFO(test_case.name);

			ScopedFile scoped_file(test_case.xml);
			DrumkitDOM dom;
			CHECK_EQ(test_case.expected_status,
			    parseDrumkitFile(scoped_file.filename(), dom));

			CHECK_EQ(test_case.expected_version, dom.version);
			CHECK_EQ(test_case.expected_samplerate, dom.samplerate);
			CHECK_EQ(test_case.expected_title, dom.metadata.title);
			CHECK_EQ(test_case.expected_description, dom.metadata.description);
			CHECK_EQ(test_case.expected_channels, dom.channels.size());
			CHECK_EQ(test_case.expected_instruments, dom.instruments.size());

			if(test_case.expect_first_channel_main)
			{
				CHECK_EQ(std::size_t(1), dom.instruments[0].channel_map.size());
				CHECK(test_case.expected_main ==
				      dom.instruments[0].channel_map[0].main);
			}

			if(test_case.expect_first_choke)
			{
				CHECK_EQ(std::size_t(1), dom.instruments[0].chokes.size());
				CHECK_EQ(test_case.expected_choketime,
				    dom.instruments[0].chokes[0].choketime);
			}
			else if(dom.instruments.size() == 1)
			{
				CHECK_EQ(std::size_t(0), dom.instruments[0].chokes.size());
			}
		}
	}

	SUBCASE("instrumentVersionAndFailureMatrix")
	{
		struct TestCase
		{
			std::string name;
			std::string xml;
			bool expected_status;
			std::string expected_version;
			std::size_t expected_samples;
			std::size_t expected_velocities;
			std::size_t expected_filechannel;
			double expected_power;
			bool expect_sample_checks;
		};

		const std::vector<TestCase> cases = {
		    {"legacy version defaults parse velocity groups",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<instrument name=\"Snare\">\n"
		        "  <samples>\n"
		        "    <sample name=\"S1\">\n"
		        "      <audiofile channel=\"Top\" file=\"s.wav\"/>\n"
		        "    </sample>\n"
		        "  </samples>\n"
		        "  <velocities>\n"
		        "    <velocity lower=\"0\" upper=\"1\">\n"
		        "      <sampleref probability=\"1\" name=\"S1\"/>\n"
		        "    </velocity>\n"
		        "  </velocities>\n"
		        "</instrument>",
		        true, "1.0", 1u, 1u, 1u, 0.0, true},
		    {"near-valid v1.0.0 still parses velocity groups",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<instrument version=\"1.0.0\" name=\"Snare\">\n"
		        "  <samples>\n"
		        "    <sample name=\"S1\">\n"
		        "      <audiofile channel=\"Top\" file=\"s.wav\" "
		        "filechannel=\"3\"/>\n"
		        "    </sample>\n"
		        "  </samples>\n"
		        "  <velocities>\n"
		        "    <velocity lower=\"0\" upper=\"1\">\n"
		        "      <sampleref probability=\"1\" name=\"S1\"/>\n"
		        "    </velocity>\n"
		        "  </velocities>\n"
		        "</instrument>",
		        true, "1.0.0", 1u, 1u, 3u, 0.0, true},
		    {"v2 ignores velocity groups and requires power",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<instrument version=\"2.0\" name=\"Snare\">\n"
		        "  <samples>\n"
		        "    <sample name=\"S1\" power=\"0.5\">\n"
		        "      <audiofile channel=\"Top\" file=\"s.wav\"/>\n"
		        "    </sample>\n"
		        "  </samples>\n"
		        "  <velocities>\n"
		        "    <velocity lower=\"0\" upper=\"1\">\n"
		        "      <sampleref probability=\"1\" name=\"S1\"/>\n"
		        "    </velocity>\n"
		        "  </velocities>\n"
		        "</instrument>",
		        true, "2.0", 1u, 0u, 1u, 0.5, true},
		    {"partial v2 sample without power fails",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<instrument version=\"2.0\" name=\"Snare\">\n"
		        "  <samples>\n"
		        "    <sample name=\"S1\">\n"
		        "      <audiofile channel=\"Top\" file=\"s.wav\"/>\n"
		        "    </sample>\n"
		        "  </samples>\n"
		        "</instrument>",
		        false, "2.0", 1u, 0u, 1u, 0.0, false},
		    {"malformed xml fails and keeps parser defaults",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<instrument\n"
		        "  <samples><sample name=\"S1\"/></samples>\n"
		        "</instrument>",
		        false, "1.0", 0u, 0u, 0u, 0.0, false},
		    {"wrong root fails with empty instrument",
		        "<?xml version='1.0' encoding='UTF-8'?>\n"
		        "<drumkit>\n"
		        "  <samples><sample name=\"S1\"/></samples>\n"
		        "</drumkit>",
		        false, "1.0", 0u, 0u, 0u, 0.0, false}};

		for(const auto& test_case : cases)
		{
			INFO(test_case.name);

			ScopedFile scoped_file(test_case.xml);
			InstrumentDOM dom;
			CHECK_EQ(test_case.expected_status,
			    parseInstrumentFile(scoped_file.filename(), dom));

			CHECK_EQ(test_case.expected_version, dom.version);
			CHECK_EQ(test_case.expected_samples, dom.samples.size());
			CHECK_EQ(test_case.expected_velocities, dom.velocities.size());

			if(test_case.expect_sample_checks)
			{
				CHECK_EQ(std::size_t(1), dom.samples[0].audiofiles.size());
				CHECK_EQ(test_case.expected_filechannel,
				    dom.samples[0].audiofiles[0].filechannel);
				CHECK_EQ(test_case.expected_power, dom.samples[0].power);
			}
		}
	}

	SUBCASE("instrumentAttributeMatrix")
	{
		struct TestCase
		{
			std::string main_attr;
			std::string filechannel_attr;
			bool expected_status;
			main_state_t expected_main;
			std::size_t expected_filechannel;
		};

		const std::vector<TestCase> cases = {
		    {"", "", true, main_state_t::unset, 1u},
		    {"true", "2", true, main_state_t::is_main, 2u},
		    {"false", "3", true, main_state_t::is_not_main, 3u},
		    {"maybe", "4", false, main_state_t::unset, 4u},
		    {"true", "-1", false, main_state_t::is_main, 1u},
		    {"true", "1.5", false, main_state_t::is_main, 1u}};

		for(const auto& test_case : cases)
		{
			std::string main_attr =
			    test_case.main_attr == ""
			        ? ""
			        : " main=\"" + test_case.main_attr + "\"";
			std::string filechannel_attr =
			    test_case.filechannel_attr == ""
			        ? ""
			        : " filechannel=\"" + test_case.filechannel_attr + "\"";

			ScopedFile scoped_file(
			    "<?xml version='1.0' encoding='UTF-8'?>\n"
			    "<instrument version=\"2.0\" name=\"Snare\">\n"
			    "  <channels>\n"
			    "    <channel name=\"Top\"" +
			    main_attr +
			    "/>\n"
			    "  </channels>\n"
			    "  <samples>\n"
			    "    <sample name=\"S1\" power=\"0.5\">\n"
			    "      <audiofile channel=\"Top\" file=\"s.wav\"" +
			    filechannel_attr +
			    "/>\n"
			    "    </sample>\n"
			    "  </samples>\n"
			    "</instrument>");

			InstrumentDOM dom;
			CHECK_EQ(test_case.expected_status,
			    parseInstrumentFile(scoped_file.filename(), dom));

			CHECK_EQ(std::size_t(1), dom.instrument_channels.size());
			CHECK(test_case.expected_main == dom.instrument_channels[0].main);

			CHECK_EQ(std::size_t(1), dom.samples.size());
			CHECK_EQ(std::size_t(1), dom.samples[0].audiofiles.size());
			CHECK_EQ(test_case.expected_filechannel,
			    dom.samples[0].audiofiles[0].filechannel);
		}
	}
}
