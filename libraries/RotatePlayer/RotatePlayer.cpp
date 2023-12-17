#include "RotatePlayer.h"

RotatePlayer::RotatePlayer(int rotate_left_pin, int rotate_right_pin)
    : m_rotate_left_pin(rotate_left_pin), m_rotate_right_pin(rotate_right_pin)
{
}

int RotatePlayer::getSegments(int rotate_step = 128)
{
    int leftValue = analogRead(m_rotate_left_pin);
    int rightValue = analogRead(m_rotate_right_pin);

    int leftSegment = leftValue / rotate_step;
    int rightSegment = rightValue / rotate_step;

    return (leftSegment << 4) | rightSegment;
}
