/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/***************************************************************************
 *            configstringio.h
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
#pragma once

#include <string>

#include <settings.h>

//! Serialises and deserialises plugin state to/from an XML config string.
//! Used by the plugin wrapper to implement state save/restore lifecycle hooks.
class ConfigStringIO
{
public:
	ConfigStringIO(Settings& settings);

	//! Serialise current settings to an XML config string.
	std::string get();

	//! Deserialise settings from an XML config string.
	//! \return false if the string could not be parsed, true otherwise.
	bool set(std::string config_string);

private:
	Settings& settings;
};
