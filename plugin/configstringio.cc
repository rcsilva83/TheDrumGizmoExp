/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configstringio.cc
 *
 *  Tue Mar 24 00:00:00 CET 2026
 *  Copyright 2016 Bent Bisballe Nyeng
 *  deva@aasimon.org
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
#include "configstringio.h"

#include <string>

#include <hugin.hpp>

#include <configparser.h>
#include <nolocale.h>

namespace
{

std::string float2str(float a)
{
	char buf[256];
	snprintf_nol(buf, sizeof(buf) - 1, "%f", a);
	return buf;
}

std::string bool2str(bool a)
{
	return a ? "true" : "false";
}

std::string int2str(int a)
{
	char buf[256];
	snprintf(buf, sizeof(buf) - 1, "%d", a);
	return buf;
}

float str2float(std::string a)
{
	if(a == "")
	{
		return 0.0;
	}

	return atof_nol(a.c_str());
}

int str2int(std::string a)
{
	try
	{
		return std::stoi(a);
	}
	catch(...)
	{
		return 0;
	}
}

long long str2ll(std::string a)
{
	try
	{
		return std::stoll(a);
	}
	catch(...)
	{
		return 0;
	}
}

} // end anonymous namespace

ConfigStringIO::ConfigStringIO(Settings& settings)
	: settings(settings)
{
}

std::string ConfigStringIO::get()
{
	return
		"<config version=\"1.0\">\n"
		"  <value name=\"drumkitfile\">" + settings.drumkit_file.load() + "</value>\n"
		"  <value name=\"midimapfile\">" + settings.midimap_file.load() + "</value>\n"
		"  <value name=\"enable_velocity_modifier\">" +
		bool2str(settings.enable_velocity_modifier.load()) + "</value>\n"
		"  <value name=\"velocity_modifier_falloff\">" +
		float2str(settings.velocity_modifier_falloff.load()) + "</value>\n"
		"  <value name=\"velocity_modifier_weight\">" +
		float2str(settings.velocity_modifier_weight.load()) + "</value>\n"
		"  <value name=\"velocity_stddev\">" +
		float2str(settings.velocity_stddev.load()) + "</value>\n"
		"  <value name=\"sample_selection_f_close\">" +
		float2str(settings.sample_selection_f_close.load()) + "</value>\n"
		"  <value name=\"sample_selection_f_diverse\">" +
		float2str(settings.sample_selection_f_diverse.load()) + "</value>\n"
		"  <value name=\"sample_selection_f_random\">" +
		float2str(settings.sample_selection_f_random.load()) + "</value>\n"
		"  <value name=\"enable_velocity_randomiser\">" +
		bool2str(settings.enable_velocity_randomiser.load()) + "</value>\n"
		"  <value name=\"velocity_randomiser_weight\">" +
		float2str(settings.velocity_randomiser_weight.load()) + "</value>\n"
		"  <value name=\"enable_resampling\">" +
		bool2str(settings.enable_resampling.load()) + "</value>\n"
		"  <value name=\"resampling_quality\">" +
		float2str(settings.resampling_quality.load()) + "</value>\n"
		"  <value name=\"disk_cache_upper_limit\">" +
		int2str(settings.disk_cache_upper_limit.load()) + "</value>\n"
		"  <value name=\"disk_cache_chunk_size\">" +
		int2str(settings.disk_cache_chunk_size.load()) + "</value>\n"
		"  <value name=\"disk_cache_enable\">" +
		bool2str(settings.disk_cache_enable.load()) + "</value>\n"
		"  <value name=\"enable_bleed_control\">" +
		bool2str(settings.enable_bleed_control.load()) + "</value>\n"
		"  <value name=\"master_bleed\">" +
		float2str(settings.master_bleed.load()) + "</value>\n"
		"  <value name=\"enable_latency_modifier\">" +
		bool2str(settings.enable_latency_modifier.load()) + "</value>\n"
		// Do not store/reload this value
		//"  <value name=\"latency_max\">" +
		//int2str(settings.latency_max.load()) + "</value>\n"
		"  <value name=\"latency_laid_back_ms\">" +
		float2str(settings.latency_laid_back_ms.load()) + "</value>\n"
		"  <value name=\"latency_stddev\">" +
		float2str(settings.latency_stddev.load()) + "</value>\n"
		"  <value name=\"latency_regain\">" +
		float2str(settings.latency_regain.load()) + "</value>\n"
		"  <value name=\"enable_powermap\">" +
		bool2str(settings.enable_powermap.load()) + "</value>\n"
		"  <value name=\"powermap_fixed0_x\">" +
		float2str(settings.powermap_fixed0_x.load()) + "</value>\n"
		"  <value name=\"powermap_fixed0_y\">" +
		float2str(settings.powermap_fixed0_y.load()) + "</value>\n"
		"  <value name=\"powermap_fixed1_x\">" +
		float2str(settings.powermap_fixed1_x.load()) + "</value>\n"
		"  <value name=\"powermap_fixed1_y\">" +
		float2str(settings.powermap_fixed1_y.load()) + "</value>\n"
		"  <value name=\"powermap_fixed2_x\">" +
		float2str(settings.powermap_fixed2_x.load()) + "</value>\n"
		"  <value name=\"powermap_fixed2_y\">" +
		float2str(settings.powermap_fixed2_y.load()) + "</value>\n"
		"  <value name=\"powermap_shelf\">" +
		bool2str(settings.powermap_shelf.load()) + "</value>\n"
		"  <value name=\"enable_voice_limit\">" +
		bool2str(settings.enable_voice_limit.load()) + "</value>\n"
		"  <value name=\"voice_limit_max\">" +
		int2str(settings.voice_limit_max.load()) + "</value>\n"
		"  <value name=\"voice_limit_rampdown\">" +
		float2str(settings.voice_limit_rampdown.load()) + "</value>\n"
		"</config>";
}

bool ConfigStringIO::set(std::string config_string)
{
	DEBUG(config, "Load config: %s\n", config_string.c_str());

	ConfigParser p;
	if(!p.parseString(config_string))
	{
		ERR(config, "Config parse error.\n");
		return false;
	}

	if(p.value("enable_velocity_modifier") != "")
	{
		settings.enable_velocity_modifier.store(
		    p.value("enable_velocity_modifier") == "true");
	}

	if(p.value("velocity_modifier_falloff") != "")
	{
		settings.velocity_modifier_falloff.store(
		    str2float(p.value("velocity_modifier_falloff")));
	}

	if(p.value("velocity_modifier_weight") != "")
	{
		settings.velocity_modifier_weight.store(
		    str2float(p.value("velocity_modifier_weight")));
	}

	if(p.value("velocity_stddev") != "")
	{
		settings.velocity_stddev.store(str2float(p.value("velocity_stddev")));
	}

	if(p.value("sample_selection_f_close") != "")
	{
		settings.sample_selection_f_close.store(
		    str2float(p.value("sample_selection_f_close")));
	}

	if(p.value("sample_selection_f_diverse") != "")
	{
		settings.sample_selection_f_diverse.store(
		    str2float(p.value("sample_selection_f_diverse")));
	}

	if(p.value("sample_selection_f_random") != "")
	{
		settings.sample_selection_f_random.store(
		    str2float(p.value("sample_selection_f_random")));
	}

	if(p.value("enable_velocity_randomiser") != "")
	{
		settings.enable_velocity_randomiser.store(
		    p.value("enable_velocity_randomiser") == "true");
	}

	if(p.value("velocity_randomiser_weight") != "")
	{
		settings.velocity_randomiser_weight.store(
		    str2float(p.value("velocity_randomiser_weight")));
	}

	if(p.value("enable_resampling") != "")
	{
		settings.enable_resampling.store(p.value("enable_resampling") == "true");
	}

	if(p.value("resampling_quality") != "")
	{
		settings.resampling_quality.store(
		    str2float(p.value("resampling_quality")));
	}

	if(p.value("disk_cache_upper_limit") != "")
	{
		settings.disk_cache_upper_limit.store(
		    str2ll(p.value("disk_cache_upper_limit")));
	}

	if(p.value("disk_cache_chunk_size") != "")
	{
		settings.disk_cache_chunk_size.store(
		    str2int(p.value("disk_cache_chunk_size")));
	}

	if(p.value("disk_cache_enable") != "")
	{
		settings.disk_cache_enable.store(
		    p.value("disk_cache_enable") == "true");
	}

	if(p.value("enable_bleed_control") != "")
	{
		settings.enable_bleed_control.store(
		    p.value("enable_bleed_control") == "true");
	}

	if(p.value("master_bleed") != "")
	{
		settings.master_bleed.store(str2float(p.value("master_bleed")));
	}

	if(p.value("enable_latency_modifier") != "")
	{
		settings.enable_latency_modifier.store(
		    p.value("enable_latency_modifier") == "true");
	}

	// Do not store/reload this value
	//if(p.value("latency_max") != "")
	//{
	//	settings.latency_max.store(str2int(p.value("latency_max")));
	//}

	if(p.value("latency_laid_back_ms") != "")
	{
		settings.latency_laid_back_ms.store(
		    str2float(p.value("latency_laid_back_ms")));
	}

	if(p.value("latency_stddev") != "")
	{
		settings.latency_stddev.store(str2float(p.value("latency_stddev")));
	}

	if(p.value("latency_regain") != "")
	{
		settings.latency_regain.store(str2float(p.value("latency_regain")));
	}

	if(p.value("enable_powermap") != "")
	{
		settings.enable_powermap.store(p.value("enable_powermap") == "true");
	}

	if(p.value("powermap_fixed0_x") != "")
	{
		settings.powermap_fixed0_x.store(
		    str2float(p.value("powermap_fixed0_x")));
	}

	if(p.value("powermap_fixed0_y") != "")
	{
		settings.powermap_fixed0_y.store(
		    str2float(p.value("powermap_fixed0_y")));
	}

	if(p.value("powermap_fixed1_x") != "")
	{
		settings.powermap_fixed1_x.store(
		    str2float(p.value("powermap_fixed1_x")));
	}

	if(p.value("powermap_fixed1_y") != "")
	{
		settings.powermap_fixed1_y.store(
		    str2float(p.value("powermap_fixed1_y")));
	}

	if(p.value("powermap_fixed2_x") != "")
	{
		settings.powermap_fixed2_x.store(
		    str2float(p.value("powermap_fixed2_x")));
	}

	if(p.value("powermap_fixed2_y") != "")
	{
		settings.powermap_fixed2_y.store(
		    str2float(p.value("powermap_fixed2_y")));
	}

	if(p.value("powermap_shelf") != "")
	{
		settings.powermap_shelf.store(p.value("powermap_shelf") == "true");
	}

	if(p.value("enable_voice_limit") != "")
	{
		settings.enable_voice_limit.store(
		    p.value("enable_voice_limit") == "true");
	}

	if(p.value("voice_limit_max") != "")
	{
		settings.voice_limit_max.store(str2int(p.value("voice_limit_max")));
	}

	if(p.value("voice_limit_rampdown") != "")
	{
		settings.voice_limit_rampdown.store(
		    str2float(p.value("voice_limit_rampdown")));
	}

	std::string newkit = p.value("drumkitfile");
	if(newkit != "")
	{
		settings.drumkit_file.store(newkit);
	}

	std::string newmidimap = p.value("midimapfile");
	if(newmidimap != "")
	{
		settings.midimap_file.store(newmidimap);
	}

	return true;
}
