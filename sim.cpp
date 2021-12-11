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

const CEEPROM::data_t& ee = NVS.data;

struct aa_t
{
    uint16_t x;
    uint8_t  y;
    uint32_t z;
};


class CTEST
{
public:

    void    set_x(uint16_t value) { set(data.x, value); }
    void    set_y(uint8_t value)  { set(data.y, value); }
    void    set_z(uint32_t value) { set(data.z, value); }

    const aa_t data;



protected:
    bool    m_flag;

    template < class T>
    void set(const T& dest, T value)
    {
        *(T*)&dest = value;
        m_flag = true;
    }


} TEST;



int main()
{
    TEST.set_x(3);
    TEST.set_z(41);

    printf("%i  %i\n", TEST.data.x, TEST.data.z);
    exit(1);

#if 0
    NVS.destroy();
    exit(1);
#endif

#if 0
    NVS.destroy();
    NVS.read();

    for (int i = 0; i < 6; ++i)
    {
        int hex = i + 1;
        ee.run_mode = (hex << 4) | hex;
        NVS.write();
    }

    exit(0);

#endif

#if 1
    NVS.read();
    printf("%d\n", ee.run_mode);
    exit(1);
#endif

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

