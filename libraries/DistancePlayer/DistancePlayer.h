#include <Arduino.h>

#pragma once

class DistancePlayer
{
public:
    DistancePlayer(int echo_pin, int trigger_pin);

    int update(unsigned long delta);

private:
    enum class State
    {
        WAIT_FOR_START,
        BEFORE_PING,
        WAIT_FOR_PING,
        START_PING,
        WAIT_FOR_ECHO,
        START_ECHO
    };

    int m_echo_pin;
    int m_trigger_pin;
    unsigned long m_micros = 0;

    int m_last_height = 0;
    State m_state = State::BEFORE_PING;
};