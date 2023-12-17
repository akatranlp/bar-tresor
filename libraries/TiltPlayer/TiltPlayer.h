#include <Arduino.h>
#include <Wire.h>

#pragma once

class TiltPlayer
{
public:
    enum class Tilt
    {
        Left,
        Right,
        Forward,
        Backward,
        None,
    };

    TiltPlayer(uint8_t address);

    void begin();

    Tilt getTilt();

private:
    uint8_t m_address;
};
