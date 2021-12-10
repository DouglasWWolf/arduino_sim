#include <arduino.h>
#include <Windows.h>


unsigned long millis()
{
    return GetTickCount();
}


static int input_signal[256];

void sim_input(int pin, int state)
{
    input_signal[pin] = state;
}

int digitalRead(int pin)
{
    return input_signal[pin];
}


void pinMode(int pin, int state) {}

void attachInterrupt(int a, void(*fn)(), int b) {}

void cli() {}
void sei() {}


int PORTA, PORTB, PORTC, PORTD;
int DDRA, DDRB, DDRC, DDRD;
int PCMSK0, PCMSK1, PCMSK2, PCMSK3;

int PORTB0;
int PCIFR;
int PCIF1;
int PCICR;
int PCIE1;
int PCINT8;

int EICRA;
int EIMSK;
int EIFR;



