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

CEEPROM NVS(5, 256);

CEEPROM::data_t& ee = NVS.data;


template <class T> class noconst
{
    T& operator()(const T& var)
    {
        return *(T*)&var;
    }
};

class uc_int
{
public:
    int& operator[](const int& var)
    {
        return *(int*)&var;
    }
};

#define nonconst(T,v) *(T*)&v


struct data_t
{
    const int flag1 = 0;
    const int flag2 = 0;
} data;

int main()
{
#if 0
    NVS.destroy();
    NVS.read();

    ee.run_mode = 11;
    NVS.write();

    ee.run_mode = 22;
    NVS.write();

    ee.run_mode = 23;
    NVS.write();
#endif



    const int ax = 4;

    //noconst<int>(ax) = 42;
    nonconst(int, ax) = 42;
    printf("ax = %d\n", ax);
    *(int*)&ax = 44;
    printf("ax = %d\n", ax);
    *const_cast<int*>(&data.flag1) = 13;
    printf("ax = %d\n", data.flag1);
    
    *const_cast<int*>(&ax) = 99;
    *(int*)&data.flag1 = 93;
    printf("ax = %d\n", data.flag1);
    exit(1);


    enum class mode_t {FOO, BAR};
    enum xmod_t {FOO, BAR};

    mode_t x = mode_t::FOO;

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

