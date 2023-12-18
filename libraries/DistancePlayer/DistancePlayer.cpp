// This code is based on the following tutorial:
// https://funduino.de/nr-10-entfernung-messen

#include "DistancePlayer.h"

DistancePlayer::DistancePlayer(int echo_pin, int trigger_pin)
    : m_echo_pin(echo_pin), m_trigger_pin(trigger_pin)
{
    pinMode(trigger_pin, OUTPUT);
    pinMode(echo_pin, INPUT);
}

int DistancePlayer::update(unsigned long delta)
{
    switch (m_state)
    {
    // ------------------------------------
    // --------STATE: BEFORE PING----------
    // ------------------------------------
    case State::BEFORE_PING:
    {
        // Set the trigger pin to low to get a clean signal
        digitalWrite(m_trigger_pin, LOW);
        m_state = State::WAIT_FOR_PING;
        m_micros = 0;
    }
    break;
    // ------------------------------------
    // -------STATE: WAIT FOR PING---------
    // ------------------------------------
    case State::WAIT_FOR_PING:
    {
        // Wait for 5ms before sending the ping
        if (m_micros >= 5000)
        {
            m_state = State::START_PING;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    // ------------------------------------
    // --------STATE: START PING-----------
    // ------------------------------------
    case State::START_PING:
    {
        // Send the ping
        digitalWrite(m_trigger_pin, HIGH);
        m_state = State::PINGING;
        m_micros = 0;
    }
    break;
    // ------------------------------------
    // ---------STATE: PINGING-------------
    // ------------------------------------
    case State::PINGING:
    {
        // Sending the ping for 10ms
        if (m_micros >= 10000)
        {
            m_state = State::ECHO;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    // ------------------------------------
    // --------STATE: START ECHO-----------
    // ------------------------------------
    case State::ECHO:
    {
        // disable the signal
        digitalWrite(m_trigger_pin, LOW);

        // Wait for the echo
        int duration = pulseIn(m_echo_pin, HIGH);

        // Calculate the distance in cm with a specified formula
        int distance = (duration / 2) * 0.03432;
        if (distance >= 500 || distance <= 0)
        {
            return m_last_height;
        }
        else
        {
            m_last_height = distance;
            return m_last_height;
        }
        m_state = State::BEFORE_PING;
    }
    break;
    }
    return -1;
}
