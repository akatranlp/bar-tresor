#include <Arduino.h>

#pragma once

class TouchPlayer
{
public:
    enum class Touch : u8
    {
        None = 0,
        Up = 1,
        Back = 2,
        Left = 4,
        Right = 8,
    };

#define TOUCH_NONE static_cast<u8>(TouchPlayer::Touch::None)
#define TOUCH_UP static_cast<u8>(TouchPlayer::Touch::Up)
#define TOUCH_BACK static_cast<u8>(TouchPlayer::Touch::Back)
#define TOUCH_LEFT static_cast<u8>(TouchPlayer::Touch::Left)
#define TOUCH_RIGHT static_cast<u8>(TouchPlayer::Touch::Right)

    TouchPlayer(int up_pin, int back_pin, int left_pin, int right_pin);

    u8 getTouchInput();

private:
    int m_up_pin;
    int m_back_pin;
    int m_left_pin;
    int m_right_pin;
};
