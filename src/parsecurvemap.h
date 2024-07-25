/* -*- Mode: c++ -*- */
/***************************************************************************
 *            parsecurvemap.h
 *
 *  Wed Jul 24 12:55:00 CEST 2024
 *  Copyright 2024 Sander Vocke
 *
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
#pragma once

#include <pugixml.hpp>
#include "curvemap.h"

// Returns true if success (out is replaced), false if failure (out unchanged)
bool parse_curve_map(pugi::xml_node curvemap_node, CurveMap &out);