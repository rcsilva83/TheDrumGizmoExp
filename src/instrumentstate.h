#pragma once

//! Tracks the MIDI state of an instrument during play.
struct InstrumentState {

    // Physical position applied to the next hit.
    // 0.0-1.0, where 0.0 is the middle and 1.0 is closest to the rim.
    float position = 0.0;

    // Openness (typically for a hi-hat).
    // 0.0-1.0, where 0.0 is closed and 1.0 is fully open.
    float openness = 0.0;
};