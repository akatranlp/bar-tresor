#include "RotatePlayer.h"

RotatePlayer::RotatePlayer(int rotate_left_pin, int rotate_right_pin)
    : m_rotate_left_pin(rotate_left_pin), m_rotate_right_pin(rotate_right_pin)
{
}

int RotatePlayer::getSegments(int rotate_step = 128)
{
    // Read the values
    int leftValue = analogRead(m_rotate_left_pin);
    int rightValue = analogRead(m_rotate_right_pin);

    // Divide the values into segments
    int leftSegment = leftValue / rotate_step;
    int rightSegment = rightValue / rotate_step;

    // Combine the segments into one value The first 4 bits are the leftSegment
    // and the last 4 bits are the rightSegment
    return (leftSegment << 4) | rightSegment;
}
