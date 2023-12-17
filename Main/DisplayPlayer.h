#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#pragma once

typedef void (*DisplayUpdateMethod)(Adafruit_ST7735 *tft);

class DisplayPlayer
{
public:
    DisplayPlayer(int cs, int dc, int rst)
        : m_tft(Adafruit_ST7735(cs, dc, rst))
    {
    }

    void begin()
    {
        Serial.println("BEGIN");
        m_tft.initR(INITR_BLACKTAB); // initialize a ST7735S chip, black tab
        m_tft.fillScreen(ST7735_BLACK);
        m_tft.setRotation(1);
    }

    void update(unsigned long delta)
    {
        switch (m_state)
        {
        case State::WAIT_FOR_START:
            break;
        case State::REFRESH:
            m_tft.fillScreen(ST7735_BLACK);
            if (m_updateMethod != nullptr)
            {
                m_updateMethod(&m_tft);
            }
            m_state = State::WAIT_FOR_REFRESH;
            m_micros = 0;
            break;
        case State::WAIT_FOR_REFRESH:
            if (m_micros > 1000000)
            {
                m_state = State::REFRESH;
            }
            else
            {
                m_micros += delta;
            }
            break;
        }
    }

    void drawImmediate(DisplayUpdateMethod updateMethod)
    {
        m_tft.fillScreen(ST7735_BLACK);
        m_state = State::WAIT_FOR_START;
        updateMethod(&m_tft);
    }

    // Set update method for next refresh iteration
    void setUpdateMethod(DisplayUpdateMethod updateMethod)
    {
        m_updateMethod = updateMethod;
    }

private:
    enum class State
    {
        WAIT_FOR_START,
        REFRESH,
        WAIT_FOR_REFRESH,
    };
    unsigned long m_micros = 0;
    State m_state = State::WAIT_FOR_START;

    DisplayUpdateMethod m_updateMethod = nullptr;

    Adafruit_ST7735 m_tft;
};
