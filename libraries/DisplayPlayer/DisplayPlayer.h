#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#pragma once

typedef void (*DisplayUpdateMethod)(Adafruit_ST7735 *tft);

class DisplayPlayer
{
public:
    enum class State
    {
        WAIT_FOR_START,
        REFRESH,
        WAIT_FOR_REFRESH,

        SCROLLING_TEXT_START,
        SCROLLING_TEXT,
        WAIT_FOR_SCROLLING_TEXT,
    };

    struct Text
    {
        const char **rows;
        int size;
        int text_size;
    };

    struct SubText
    {
        const char *text;
        int x;
        int y;
        int width;
        int height;
    };

    DisplayPlayer(int cs, int dc, int rst);

    void begin();
    State get_state() { return m_state; }

    void update(unsigned long delta);

    void drawImmediate(DisplayUpdateMethod updateMethod);

    void draw_text(Text *text);
    void draw_sub_text(SubText *text);
    void draw_scrolling_text(Text *text);
    void draw_distance_and_time(int distance, int duration);

    void set_loop_text(bool loop) { m_loop_text = loop; }
    void set_refresh_rate(int millis) { m_refresh_rate = millis; }

    void draw_clock(int hour, int minute);

private:
    unsigned long m_micros = 0;
    State m_state = State::WAIT_FOR_START;

    unsigned long m_refresh_rate = 1000;

    Adafruit_ST7735 m_tft;

    // Scrolling Text
    int m_current_row = 0;
    Text *m_current_text = nullptr;
    SubText *m_current_sub_text = nullptr;

    bool m_loop_text = false;
};
