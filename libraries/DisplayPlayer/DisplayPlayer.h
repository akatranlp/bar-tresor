/* *******************************************************
 *  DisplayPlayer.h
 *  This library is used to display different pre defined
 *  things on the display
 *
 *  It uses the two libraries:
 *   - Adafruit_GFX.h
 *   - Adafruit_ST7735.h
 ********************************************************/
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

#pragma once

typedef void (*DisplayUpdateMethod)(Adafruit_ST7735 *tft);

class DisplayPlayer
{
public:
    // Display states
    enum class State
    {
        WAIT_FOR_START,

        REFRESH,
        WAIT_FOR_REFRESH,

        SCROLLING_TEXT_START,
        SCROLLING_TEXT,
        WAIT_FOR_SCROLLING_TEXT,
    };

    // This struct is used to store the text that should be displayed
    struct Text
    {
        const char **rows;
        int size;
        int text_size;
    };

    // This struct is used to store a rect and the text that should be displayed
    struct SubText
    {
        const char *text;
        int x;
        int y;
        int width;
        int height;
    };

    DisplayPlayer(int cs, int dc, int rst);

    // Initialize the display
    void begin();

    // Get the current state of the display
    State get_state() { return m_state; }

    void update(unsigned long delta);

    // With this method you can draw something immediatly on the display
    void drawImmediate(DisplayUpdateMethod updateMethod);

    // These methods are used to draw different things on the display which
    // are needed for the puzzles
    void draw_text(Text *text);
    void draw_sub_text(SubText *text);
    void draw_scrolling_text(Text *text);
    void draw_distance_and_time(int distance, int duration);
    void draw_clock(int hour, int minute);

    // These methods are used to change some values for the state machine
    void set_loop_text(bool loop) { m_loop_text = loop; }
    void set_refresh_rate(int millis) { m_refresh_rate = millis; }

private:
    unsigned long m_micros = 0;

    State m_state = State::WAIT_FOR_START;

    Adafruit_ST7735 m_tft;

    // This value is used to store the index of the current row
    // in the current text which is displayed at the top
    int m_current_row = 0;
    Text *m_current_text = nullptr;

    SubText *m_current_sub_text = nullptr;

    unsigned long m_refresh_rate = 1000;
    // This value is used to store if the current text should be looped
    bool m_loop_text = false;
};
