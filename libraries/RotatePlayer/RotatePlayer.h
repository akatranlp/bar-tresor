#include <Arduino.h>

#pragma once

class RotatePlayer
{
public:
    RotatePlayer(int rotate_left_pin, int rotate_right_pin);

    int getSegments(int rotate_step = 128);

private:
    int m_rotate_left_pin;
    int m_rotate_right_pin;
    int m_bit_shift;
};
