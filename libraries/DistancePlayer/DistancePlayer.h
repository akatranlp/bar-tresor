/* *******************************************************
 *  DistancePlayer.h
 *  This library is used to measure the distance from the
 *  bottom of our vault to any surface underneath it
 *
 *  It uses the ultrasonic sensor HR-SR04
 *  to measure the distance
 ********************************************************/
#include <Arduino.h>

#pragma once

class DistancePlayer
{
public:
    DistancePlayer(int echo_pin, int trigger_pin);

    // update the state machine and return the measured height if available
    int update(unsigned long delta);

private:
    enum class State
    {
        BEFORE_PING,
        WAIT_FOR_PING,
        START_PING,
        PINGING,
        ECHO,
    };

    int m_echo_pin;
    int m_trigger_pin;
    unsigned long m_micros = 0;

    int m_last_height = 0;
    State m_state = State::BEFORE_PING;
};