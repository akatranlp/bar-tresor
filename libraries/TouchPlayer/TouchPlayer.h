/* *******************************************************
 *  TouchPlayer.h
 *  This library is used to retrieve the input of our
 *  touch sensors
 ********************************************************/
#include <Arduino.h>

#pragma once

class TouchPlayer
{
public:
    // This enum is specifically designed to be used as bit flags
    enum class Touch : u8
    {
        None = 0,
        Up = 1,
        Back = 2,
        Left = 4,
        Right = 8,
    };

// Define some constants from the Touch enum
#define TOUCH_NONE static_cast<u8>(TouchPlayer::Touch::None)
#define TOUCH_UP static_cast<u8>(TouchPlayer::Touch::Up)
#define TOUCH_BACK static_cast<u8>(TouchPlayer::Touch::Back)
#define TOUCH_LEFT static_cast<u8>(TouchPlayer::Touch::Left)
#define TOUCH_RIGHT static_cast<u8>(TouchPlayer::Touch::Right)

    TouchPlayer(int up_pin, int back_pin, int left_pin, int right_pin);

    // Initialize the touch sensors
    void begin();

    // Get the current touch input
    u8 getTouchInput();

private:
    int m_up_pin;
    int m_back_pin;
    int m_left_pin;
    int m_right_pin;
};
