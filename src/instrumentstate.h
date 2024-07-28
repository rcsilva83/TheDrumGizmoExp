/* -*- Mode: c++ -*- */
/***************************************************************************
 *            instrumentstate.h
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

enum class InstrumentStateKind {
    Openness,
    NoneOrAny
};

//! Tracks the persistent state of an instrument during play.
struct InstrumentState {

    //! Openness (typically for a hi-hat).
    //! 0.0-1.0, where 0.0 is closed and 1.0 is fully open.
    float openness = 0.0;
};