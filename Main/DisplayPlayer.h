#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#define STATE_DISPLAY 1

class DisplayPlayer
{
public:
    DisplayPlayer(int cs, int rst, int dc)
        : m_tft(Adafruit_ST7735(cs, rst, dc))
    {
    }

    void update(unsigned long delta)
    {
        switch (m_state)
        {
        case STATE_DISPLAY:
            break;
        }
        m_micros += delta;
    }

private:
    unsigned long m_micros = 0;
    int m_state = STATE_DISPLAY;

    Adafruit_ST7735 m_tft;
};