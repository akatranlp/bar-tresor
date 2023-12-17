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
    case State::WAIT_FOR_START:
        break;
    case State::BEFORE_PING:
        digitalWrite(m_trigger_pin, LOW);
        m_state = State::WAIT_FOR_PING;
        m_micros = 0;
        break;
    case State::WAIT_FOR_PING:
        if (m_micros >= 5000)
        {
            m_state = State::START_PING;
        }
        else
        {
            m_micros += delta;
        }
        break;
    case State::START_PING:
        digitalWrite(m_trigger_pin, HIGH);
        m_state = State::WAIT_FOR_ECHO;
        m_micros = 0;
        break;
    case State::WAIT_FOR_ECHO:
        if (m_micros >= 10000)
        {
            m_state = State::START_ECHO;
        }
        else
        {
            m_micros += delta;
        }
        break;
    case State::START_ECHO:
        digitalWrite(m_trigger_pin, LOW);
        m_state = State::BEFORE_PING;
        int duration = pulseIn(m_echo_pin, HIGH);
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
        break;
    }
    return -1;
}
