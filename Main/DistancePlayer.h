
#define STATE_BEFORE_PING 0
#define STATE_WAIT_FOR_START 1
#define STATE_START_PING 2
#define STATE_WAIT_FOR_ECHO 3
#define STATE_ECHO 4

class DistancePlayer
{
public:
    DistancePlayer(int echo_pin, int trigger_pin)
        : m_echo_pin(echo_pin), m_trigger_pin(trigger_pin)
    {
        pinMode(trigger_pin, OUTPUT);
        pinMode(echo_pin, INPUT);
    }

    int update(unsigned long delta)
    {
        switch (m_state)
        {
        case STATE_BEFORE_PING:
            digitalWrite(m_trigger_pin, LOW);
            m_state = STATE_WAIT_FOR_START;
            m_micros = 0;
            break;
        case STATE_WAIT_FOR_START:
            if (m_micros >= 5000)
            {
                m_state = STATE_START_PING;
            }
            else
            {
                m_micros += delta;
            }
            break;
        case STATE_START_PING:
            digitalWrite(m_trigger_pin, HIGH);
            m_state = STATE_WAIT_FOR_ECHO;
            m_micros = 0;
            break;
        case STATE_WAIT_FOR_ECHO:
            if (m_micros >= 10000)
            {
                m_state = STATE_ECHO;
            }
            else
            {
                m_micros += delta;
            }
            break;
        case STATE_ECHO:
            digitalWrite(m_trigger_pin, LOW);
            m_state = STATE_BEFORE_PING;
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

private:
    int m_echo_pin;
    int m_trigger_pin;
    unsigned long m_micros = 0;

    int m_last_height = 0;
    int m_state = STATE_BEFORE_PING;
};