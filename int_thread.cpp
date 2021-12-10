#include "int_thread.h"
#include <Windows.h>
#include <conio.h>
#include "arduino.h"
#include "common.h"
#include "rotary_knob.h"
#include "globals.h"


void knob_rotate_isr();
void PCINT1_vect();
void INT1_vect();

static void click_interrupt(int state)
{
    sim_input(CLICK_PIN, state);
    PCINT1_vect();
}

#ifdef OLDINT
static void turn_interrupt(knob_event_t state)
{
    int b_state = (state == KNOB_LEFT) ? 1 : 0;

    sim_input(CHANNEL_A, 0);
    sim_input(CHANNEL_B, b_state);
    knob_rotate_isr();
    Sleep(1);
    sim_input(CHANNEL_A, 1);
    knob_rotate_isr();
}
#else
static void turn_interrupt(knob_event_t state)
{
    int b_state = (state == KNOB_LEFT) ? 1 : 0;

    sim_input(CHANNEL_B, b_state);
    INT1_vect();
}


#endif


//===========================================================================================================
// main() - Waits for messages and simulates interrupts
//===========================================================================================================
void InterruptThread::main()
{
    while (true)
    {
        // Fetch a character and convert to uppercase
        int c = _getch();
        if (c >= 'a' && c <= 'z') c -= 32;

        switch (c)
        {
        case 'D':
            click_interrupt(0);
            Sleep(8);
            click_interrupt(1);
            Sleep(8);
            click_interrupt(0);
            break;

        case 'U':
            click_interrupt(1);
            Sleep(8);
            click_interrupt(0);
            Sleep(8);
            click_interrupt(1);
            break;

        case 'L':
            turn_interrupt(KNOB_LEFT);
            turn_interrupt(KNOB_RIGHT);
            turn_interrupt(KNOB_LEFT);
            break;

        case 'R':
            turn_interrupt(KNOB_RIGHT);
            turn_interrupt(KNOB_LEFT);
            turn_interrupt(KNOB_RIGHT);
            break;

        case 'T':
            Knob.throw_away_next_event();
            break;

        }


    }
}
//===========================================================================================================



//=========================================================================================================
// launch_cthread() - This is a helper function responsible for actually bringing the thread into existence.
//=========================================================================================================
static unsigned int launch_cthread(void* pointer)
{
    // Get a pointer to our ComputeThread object
    InterruptThread* p_object = (InterruptThread*)pointer;

    // Call main() on our object
    p_object->main();

    // And tell the caller all went well
    return 0;
}
//=========================================================================================================


//=========================================================================================================
// spawn() - Spawns the thread
//=========================================================================================================
void InterruptThread::spawn()
{

    // Spawn this thread
    auto new_thread = std::thread(launch_cthread, this);

    // And let it run freely, detached from the scope of the variable "new_thread"
    new_thread.detach();
}
//=========================================================================================================


