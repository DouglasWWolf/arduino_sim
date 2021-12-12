#pragma once
#include <wire.h>

#define PROGMEM

unsigned char pgm_read_byte_near(const unsigned char* ptr);


unsigned long millis();

int digitalRead(int pin);

void sim_input(int pin, int state);

void pinMode(int pin, int state);

void attachInterrupt(int a, void(*fn)(), int b);

void cli();
void sei();

#define ISR(x) void x()

extern int PORTA, PORTB, PORTC, PORTD;
extern int DDRA, DDRB, DDRC, DDRD;
extern int PCMSK0, PCMSK1, PCMSK2, PCMSK3;



extern int PORTB0;
extern int PCIFR;
extern int PCIF1;
extern int PCICR;
extern int PCIE1;
extern int PCINT8;

extern int EICRA;
extern int EIMSK;
extern int EIFR;

extern CArduinoWire Wire;
