#pragma once

//! Tracks the MIDI state of an instrument during play.
struct InstrumentState {

    //! Openness (typically for a hi-hat).
    //! 0.0-1.0, where 0.0 is closed and 1.0 is fully open.
    float openness = 0.0;
};