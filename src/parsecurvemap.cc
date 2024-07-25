/* -*- Mode: c++ -*- */
/***************************************************************************
 *            parsecurvemap.cc
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
#include "parsecurvemap.h"

bool parse_curve_map(pugi::xml_node curvemap_node, CurveMap &out)
{
    CurveMap rval;

    if(curvemap_node.attribute("shelf")) {
        rval.setShelf(curvemap_node.attribute("shelf").as_bool(true));
    }
    if(curvemap_node.attribute("invert")) {
        rval.setInvert(curvemap_node.attribute("invert").as_bool(false));
    }
    if(curvemap_node.attribute("in2") && curvemap_node.attribute("out2")) {
        rval.setFixed2( CurveMap::CurveValuePair{
            curvemap_node.attribute("in2").as_float(0.0f),
            curvemap_node.attribute("out2").as_float(0.0f)
        });
    }
    if(curvemap_node.attribute("in1") && curvemap_node.attribute("out1")) {
        rval.setFixed1( CurveMap::CurveValuePair{
            curvemap_node.attribute("in1").as_float(0.0f),
            curvemap_node.attribute("out1").as_float(0.0f)
        });
    }
    if(curvemap_node.attribute("in0") && curvemap_node.attribute("out0")) {
        rval.setFixed0( CurveMap::CurveValuePair{
            curvemap_node.attribute("in0").as_float(0.0f),
            curvemap_node.attribute("out0").as_float(0.0f)
        });
    }    

    out = rval;
    return true;
}