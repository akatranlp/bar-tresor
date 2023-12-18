/* *******************************************************
 *  RotatePlayer.h
 *  This library is used to return the segment of two
 *  potentiometers connected to the Arduino.
 ********************************************************/
#include <Arduino.h>

#pragma once

class RotatePlayer
{
public:
    RotatePlayer(int rotate_left_pin, int rotate_right_pin);

    // The potientiometers return a value between 0 and 1023 so
    // we divide it into to 8 segments with the default step of 128
    int getSegments(int rotate_step = 128);

private:
    int m_rotate_left_pin;
    int m_rotate_right_pin;
};
