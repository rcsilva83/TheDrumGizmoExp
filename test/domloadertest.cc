/* -*- Mode: c++ -*- */
/***************************************************************************
 *            domloadertest.cc
 *
 *  Sun Jun 10 17:48:04 CEST 2018
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

#include <DGDOM.h>
#include <dgxmlparser.h>
#include <domloader.h>
#include <drumkit.h>
#include <random.h>
#include <settings.h>

#include "path.h"
#include "scopedfile.h"

struct DOMLoaderTest
{
	Settings settings;
	Random random;

	//! This just creates some drumkit.
	void testTest()
	{
		ScopedFile scoped_instrument_file1(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<instrument version=\"2.0\" name=\"Snare1\">\n"
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

		// Version 1.0 format
		ScopedFile scoped_instrument_file2(
		    "<?xml version='1.0' encoding='UTF-8'?>\n"
		    "<instrument name=\"Snare2\">\n"
		    " <samples>\n"
		    "  <sample name=\"Snare-1\">\n"
		    "   <audiofile channel=\"AmbLeft2\" file=\"1-Snare-1.wav\"/>\n"
		    "   <audiofile channel=\"AmbRight2\" file=\"1-Snare-2.wav\"/>\n"
		    "   <audiofile channel=\"SnareBottom2\" file=\"1-Snare-3.wav\"/>\n"
		    "   <audiofile channel=\"SnareTop2\" file=\"1-Snare-4.wav\"/>\n"
		    "  </sample>\n"
		    "  <sample name=\"Snare-2\">\n"
		    "   <audiofile channel=\"AmbLeft2\" file=\"2-Snare-1.wav\"/>\n"
		    "   <audiofile channel=\"AmbRight2\" file=\"2-Snare-2.wav\"/>\n"
		    "   <audiofile channel=\"SnareBottom2\" file=\"2-Snare-3.wav\"/>\n"
		    "   <audiofile channel=\"SnareTop2\" file=\"2-Snare-4.wav\"/>\n"
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

		ScopedFile scoped_file(
		    std::string("<?xml version='1.0' encoding='UTF-8'?>\n"
		                "<drumkit samplerate=\"48000\" version=\"2.0.0\">\n"
		                "  <channels>\n"
		                "   <channel name=\"AmbLeft\"/>\n"
		                "   <channel name=\"AmbRight\"/>\n"
		                "   <channel name=\"SnareTop\"/>\n"
		                "   <channel name=\"SnareBottom\"/>\n"
		                "  </channels>\n"
		                "  <instruments>\n"
		                "    <instrument name=\"Snare1\" file=\"") +
		    getFile(scoped_instrument_file1.filename()) +
		    std::string(
		        "\">\n"
		        "      <channelmap in=\"AmbLeft\" out=\"AmbLeft\" "
		        "main=\"true\"/>\n"
		        "      <channelmap in=\"AmbRight\" out=\"AmbRight\" "
		        "main=\"true\"/>\n"
		        "      <channelmap in=\"SnareTop\" out=\"SnareTop\"/>\n"
		        "      <channelmap in=\"SnareBottom\" out=\"SnareBottom\"/>\n"
		        "    </instrument>\n"
		        "    <instrument name=\"Snare2\" file=\"") +
		    getFile(scoped_instrument_file2.filename()) +
		    std::string(
		        "\">\n"
		        "      <channelmap in=\"AmbLeft2\" out=\"AmbLeft\" "
		        "main=\"true\"/>\n"
		        "      <channelmap in=\"AmbRight2\" out=\"AmbRight\" "
		        "main=\"true\"/>\n"
		        "      <channelmap in=\"SnareTop2\" out=\"SnareTop\"/>\n"
		        "      <channelmap in=\"SnareBottom2\" out=\"SnareBottom\"/>\n"
		        "    </instrument>\n"
		        "  </instruments>\n"
		        "</drumkit>"));

		DrumKit drumkit;

		DrumkitDOM drumkitdom;
		std::vector<InstrumentDOM> instrumentdoms;
		CHECK(parseDrumkitFile(scoped_file.filename(), drumkitdom));
		auto basepath = getPath(scoped_file.filename());
		for(const auto& ref : drumkitdom.instruments)
		{
			instrumentdoms.emplace_back();
			CHECK(parseInstrumentFile(
			    basepath + "/" + ref.file, instrumentdoms.back()));
		}

		DOMLoader domloader(settings, random);
		CHECK(domloader.loadDom(basepath, drumkitdom, instrumentdoms, drumkit));

		//
		// Drumkit:
		//

		CHECK_EQ(std::size_t(2), drumkit.instruments.size());
		CHECK_EQ(std::size_t(4), drumkit.channels.size());

		CHECK_EQ(std::string("AmbLeft"), drumkit.channels[0].name);
		CHECK_EQ(std::string("AmbRight"), drumkit.channels[1].name);
		CHECK_EQ(std::string("SnareTop"), drumkit.channels[2].name);
		CHECK_EQ(std::string("SnareBottom"), drumkit.channels[3].name);

		CHECK_EQ(doctest::Approx(48000.0f), drumkit.metadata._samplerate);

		CHECK(VersionStr("2.0.0") == drumkit.metadata._version);

		//
		// Instrument1 'Snare1':
		//
		{
			auto& instrument = *drumkit.instruments[0];
			CHECK_EQ(std::string(""), instrument._group);
			CHECK_EQ(std::string("Snare1"), instrument._name);
			CHECK_EQ(std::string(""), instrument._description);

			CHECK(VersionStr("2.0.0") == instrument.version);

			// NOTE: instrument.samples are the sample map belonging to
			// version 1.0
			CHECK_EQ(std::size_t(2), instrument.samplelist.size());
			{
				const auto& sample = *instrument.samplelist[0];
				CHECK_EQ(std::string("Snare-1"), sample.name);
				CHECK_EQ(doctest::Approx(0.00985718f), sample.power);
				CHECK_EQ(std::size_t(4), sample.audiofiles.size());
				for(const auto& audiofile : sample.audiofiles)
				{
					CHECK_EQ(std::string("/tmp/1-Snare.wav"),
					    audiofile.second->filename);
					switch(audiofile.second->filechannel)
					{
						// NOTE: Channel numbers are zero based - they are 1
						// based in the xml
					case 0:
						CHECK_EQ(std::string("AmbLeft"),
						    audiofile.second->instrument_channel->name);
						break;
					case 1:
						CHECK_EQ(std::string("AmbRight"),
						    audiofile.second->instrument_channel->name);
						break;
					case 11:
						CHECK_EQ(std::string("SnareBottom"),
						    audiofile.second->instrument_channel->name);
						break;
					case 12:
						CHECK_EQ(std::string("SnareTop"),
						    audiofile.second->instrument_channel->name);
						break;
					default:
						CHECK(false);
						break;
					}
				}
			}

			{
				const auto& sample = *instrument.samplelist[1];
				CHECK_EQ(std::string("Snare-2"), sample.name);
				CHECK_EQ(doctest::Approx(0.0124808f), sample.power);
				CHECK_EQ(std::size_t(4), sample.audiofiles.size());
				for(const auto& audiofile : sample.audiofiles)
				{
					CHECK_EQ(std::string("/tmp/2-Snare.wav"),
					    audiofile.second->filename);
					switch(audiofile.second->filechannel)
					{
						// NOTE: Channel numbers are zero based - they are 1
						// based in the xml
					case 0:
						CHECK_EQ(std::string("AmbLeft"),
						    audiofile.second->instrument_channel->name);
						break;
					case 1:
						CHECK_EQ(std::string("AmbRight"),
						    audiofile.second->instrument_channel->name);
						break;
					case 11:
						CHECK_EQ(std::string("SnareBottom"),
						    audiofile.second->instrument_channel->name);
						break;
					case 12:
						CHECK_EQ(std::string("SnareTop"),
						    audiofile.second->instrument_channel->name);
						break;
					default:
						CHECK(false);
						break;
					}
				}
			}
		}

		//
		// Instrument2 'Snare2' (version 1.0 instrument):
		//

		{
			auto& instrument = *drumkit.instruments[1];
			CHECK_EQ(std::string(""), instrument._group);
			CHECK_EQ(std::string("Snare2"), instrument._name);
			CHECK_EQ(std::string(""), instrument._description);

			CHECK(VersionStr("1.0.0") == instrument.version);

			// NOTE: instrument.samples are the sample map belonging to
			// version 1.0
			CHECK_EQ(std::size_t(2), instrument.samplelist.size());
			{
				const auto& sample = *instrument.samplelist[0];
				CHECK_EQ(std::string("Snare-1"), sample.name);
				CHECK_EQ(std::size_t(4), sample.audiofiles.size());
				auto afile = sample.audiofiles.begin();
				CHECK_EQ(
				    std::string("/tmp/1-Snare-1.wav"), afile->second->filename);
				CHECK_EQ(std::string("AmbLeft"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/1-Snare-2.wav"), afile->second->filename);
				CHECK_EQ(std::string("AmbRight"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/1-Snare-3.wav"), afile->second->filename);
				CHECK_EQ(std::string("SnareBottom"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/1-Snare-4.wav"), afile->second->filename);
				CHECK_EQ(std::string("SnareTop"),
				    afile->second->instrument_channel->name);
			}

			{
				const auto& sample = *instrument.samplelist[1];
				CHECK_EQ(std::string("Snare-2"), sample.name);
				CHECK_EQ(std::size_t(4), sample.audiofiles.size());
				auto afile = sample.audiofiles.begin();
				CHECK_EQ(
				    std::string("/tmp/2-Snare-1.wav"), afile->second->filename);
				CHECK_EQ(std::string("AmbLeft"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/2-Snare-2.wav"), afile->second->filename);
				CHECK_EQ(std::string("AmbRight"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/2-Snare-3.wav"), afile->second->filename);
				CHECK_EQ(std::string("SnareBottom"),
				    afile->second->instrument_channel->name);
				++afile;
				CHECK_EQ(
				    std::string("/tmp/2-Snare-4.wav"), afile->second->filename);
				CHECK_EQ(std::string("SnareTop"),
				    afile->second->instrument_channel->name);
			}

			CHECK_EQ(std::size_t(4), instrument.samples.values.size());
			auto value = instrument.samples.values.begin();
			CHECK_EQ(doctest::Approx(0.0), value->first.first);  // lower
			CHECK_EQ(doctest::Approx(0.6), value->first.second); // upper
			CHECK_EQ(std::string("Snare-1"), value->second->name);
			++value;
			CHECK_EQ(doctest::Approx(0.0), value->first.first);  // lower
			CHECK_EQ(doctest::Approx(0.6), value->first.second); // upper
			CHECK_EQ(std::string("Snare-2"), value->second->name);
			++value;
			CHECK_EQ(doctest::Approx(0.6), value->first.first);  // lower
			CHECK_EQ(doctest::Approx(1.0), value->first.second); // upper
			CHECK_EQ(std::string("Snare-2"), value->second->name);
			++value;
			CHECK_EQ(doctest::Approx(0.6), value->first.first);  // lower
			CHECK_EQ(doctest::Approx(1.0), value->first.second); // upper
			CHECK_EQ(std::string("Snare-1"), value->second->name);
		}
	}
};

TEST_CASE_FIXTURE(DOMLoaderTest, "DOMLoaderTest")
{
	SUBCASE("testTest")
	{
		testTest();
	}
}
