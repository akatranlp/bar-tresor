/* *******************************************************
 *  TiltPlayer.h
 *  This library is used to get the Tilt of the Box with
 *  the MPU6050 / GY-521
 ********************************************************/
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

    // Initialize the MPU6050
    void begin();

    // Get the current tilt of the box
    Tilt getTilt();

private:
    uint8_t m_address;
};
