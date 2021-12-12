#pragma once


class CArduinoWire
{
public:

    void begin() {}
    void beginTransmission(int address) {}
    void flush() {}
    void endTransmission() {}
    void write(const void* buffer, size_t length) {};
    void setClock(int speed) {}

};
