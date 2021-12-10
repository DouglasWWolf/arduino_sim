#pragma once

#include "rotary_knob.h"

extern CRotaryKnob Knob;


class CSleepMgr
{
public:
    void kick_timer() {};
    void on_knob_activity() {};
};


extern CSleepMgr SleepMgr;
