// sim.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>
#include "int_thread.h"
#include "arduino.h"
#include "globals.h"
#include "common.h"
#include "eeprom_manager.h"
#include "mstimer.h"

InterruptThread IntThread;

CEEPROM NVS;

CEEPROM::data_t& ee = NVS.data;


int main()
{
#if 0
    NVS.destroy();
    exit(1);
#endif

#if 1
    NVS.destroy();
    NVS.read();

    for (int i = 0; i < 6; ++i)
    {
        int hex = i + 1;
        ee.run_mode = (hex << 4) | hex;
        NVS.write();
    }

    exit(0);
#

#endif

    NVS.read();
    printf("%d\n", ee.run_mode);

    exit(1);

    Knob.init(CHANNEL_A, CHANNEL_B, CLICK_PIN);
    IntThread.spawn();


    msTimer timer;
    OneShot oneshot;

    oneshot.start(2000);

    while (true)
    {
        knob_event_t event;

        if (Knob.get_event(&event)) switch (event)
        {
            
            case KNOB_UP:
                printf("Button Up\n");
                break;

            case KNOB_LPRESS:
                printf("Button LongPress\n");
                break;

            case KNOB_LEFT:
                printf("Turn Left\n");
                break;

            case KNOB_RIGHT:
                printf("Turn Right\n");
                break;

        }

        Sleep(300);
        if (timer.is_expired()) printf("Timer expired\n");
        if (oneshot.is_expired()) printf("Oneshot expired\n");
    }

}

