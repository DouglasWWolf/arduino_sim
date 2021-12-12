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



//=============================================================================================
// map_led_to_pwm_reg() - Builds a map of which PWM register corresponds to each LED index
//=============================================================================================
int MAX_ROWS = 8;
int MAX_COLS = 16;

unsigned char pwm_reg[128];
static void map_led_to_pwm_reg()
{
    int PWM_BASE_REG = 0x24;

    int row, col;
    int starting_reg_l = MAX_ROWS - 1;
    int starting_reg_r = MAX_ROWS + 16 * 7;
    int index = 0;


    // Loop through every row the chip supports
    for (row = 0; row < MAX_ROWS; ++row)
    {
        // For the first 8 columns, register numbers are ascending
        for (col = 0; col < MAX_COLS / 2; ++col)
        {
            pwm_reg[index++] = PWM_BASE_REG + starting_reg_l + col * 16;
        }

        // For the next 8 columns, register numbers are descending
        for (col = 0; col < MAX_COLS / 2; ++col)
        {
            pwm_reg[index++] = PWM_BASE_REG + starting_reg_r - col * 16;
        }

        --starting_reg_l;
        ++starting_reg_r;
    }
}
//=============================================================================================



int main()
{
    map_led_to_pwm_reg();

    unsigned char* in = pwm_reg;
    for (int row = 0; row < MAX_ROWS; ++row)
    {
        for (int col = 0; col < MAX_COLS; ++col)
        {
            printf("%3i, ", *in++);
        }
        printf("\n");
    }


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

