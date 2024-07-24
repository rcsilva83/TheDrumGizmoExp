/* -*- Mode: c++ -*- */
/***************************************************************************
 *            position_power.h
 *
 *  Wed Jul 24 15:05:26 CEST 2024
 *  Copyright 2024 Bent Bisballe Nyeng
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
#pragma once

#include "instrument.h"

#include <vector>

class Sample;

//!
//! Consider samples in the samplelist as a two dimensional "point cloud" - each
//! also pointing to sample data (which is not used here):
//!
//!   (vel)
//!    S_v
//! max ^
//!     |x           x
//!     |  x      x      x
//!     |      x      x
//!     |   x           x
//!     | x       x x
//! min |________________> S_p (pos)
//!   center            rim
//!
//! N: is the total number of samples
//!
//! S, is a specific sample
//! S_v, is the sample's velocity
//! S_p, is the sample's position
//!
//! I_v, is the input note velocity
//! I_p, is the input note position
//!
//! -----
//!
//! Define the range R with width R_w around I_p, such that at least N/4 samples are
//! included (note the count N/4 probably needs narrowing):
//!
//!   (vel)
//!    S_v
//! max ^
//!     |x           x
//!     |  x .    +.     x
//!     |    . +   .  x
//!     |   x.     .    x
//!     | x  .    +.x
//! min |____._____._____> S_p (pos)
//!   center .  ^  .    rim
//!          . I_p .
//!          {  R  }
//! x is a sample that is not included in the set, + is a sample that is.
//!
//! Now for the range R, find the R_max velocity and the R_min velocity.
//! Use these as the boundaries for the velocity [0; 1] range.
//!
//! If no position information is available, the range will include all samples in
//! the range, because all have the default value 0.
//! This mimics perfectly the behaviour we have today.
//!
//! \param samplelist is the search space, \param position is the search origo (S_p)
//! \returns a tuple {R_min, R_max}, which defaults to {0, 1} if the samplelist is empty
Instrument::PowerRange positionPower(const std::vector<Sample*>& samplelist, double position);
