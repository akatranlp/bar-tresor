#include "TouchPlayer.h"

TouchPlayer::TouchPlayer(int up_pin, int back_pin, int left_pin, int right_pin)
    : m_up_pin(up_pin), m_back_pin(back_pin), m_left_pin(left_pin), m_right_pin(right_pin)
{
    pinMode(m_up_pin, INPUT);
    pinMode(m_back_pin, INPUT);
    pinMode(m_left_pin, INPUT);
    pinMode(m_right_pin, INPUT);
}

u8 TouchPlayer::getTouchInput()
{
    u8 touch = 0;
    if (digitalRead(m_up_pin))
    {
        touch |= TOUCH_UP;
    }
    if (digitalRead(m_back_pin))
    {
        touch |= TOUCH_BACK;
    }
    if (digitalRead(m_left_pin))
    {
        touch |= TOUCH_LEFT;
    }
    if (digitalRead(m_right_pin))
    {
        touch |= TOUCH_RIGHT;
    }
    return touch;
}