#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#pragma once

typedef void (*DisplayUpdateMethod)(Adafruit_ST7735 *tft);

class DisplayPlayer
{
public:
    DisplayPlayer(int cs, int dc, int rst);

    void begin();

    void update(unsigned long delta);

    void drawImmediate(DisplayUpdateMethod updateMethod);

    // Set update method for next refresh iteration
    void setUpdateMethod(DisplayUpdateMethod updateMethod);

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
