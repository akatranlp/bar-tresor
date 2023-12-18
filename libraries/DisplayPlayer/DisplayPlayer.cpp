#include "DisplayPlayer.h"
#include <SoftSPIB.h>

DisplayPlayer::DisplayPlayer(int cs, int dc, int rst)
    : m_tft(Adafruit_ST7735(cs, dc, rst))
{
}

void DisplayPlayer::begin()
{
    Serial.println("BEGIN");
    m_tft.initR(INITR_BLACKTAB); // initialize a ST7735S chip, black tab

    // or white
    m_tft.fillScreen(ST7735_BLACK);
    m_tft.setRotation(1);

    m_tft.setTextColor(ST7735_WHITE);
    m_tft.setTextSize(2);
    m_tft.setTextWrap(false);
}

void DisplayPlayer::update(unsigned long delta)
{
    switch (m_state)
    {
    case State::WAIT_FOR_START:
    {
    }
    break;
    case State::REFRESH:
    {
        if (m_current_sub_text == nullptr)
        {
            return;
        }

        Serial.println("REFRESH");
        Serial.println(m_current_sub_text->text);

        m_tft.fillRect(m_current_sub_text->x, m_current_sub_text->y, m_current_sub_text->width, m_current_sub_text->height, ST7735_BLACK);
        m_tft.setCursor(m_current_sub_text->x, m_current_sub_text->y);
        m_tft.println(m_current_sub_text->text);

        m_state = State::WAIT_FOR_REFRESH;
        m_micros = 0;
    }

    break;
    case State::WAIT_FOR_REFRESH:
    {
        if (m_micros > m_refresh_rate * 1000)
        {
            m_state = State::REFRESH;
        }
        else
        {
            m_micros += delta;
        }
    }

    break;
    case State::SCROLLING_TEXT_START:
    {
        m_current_row = 0;
        m_state = State::SCROLLING_TEXT;
    }
    break;
    case State::SCROLLING_TEXT:
    {
        if (m_current_row < m_current_text->size)
        {
            // Serial.println("DRAW");
            m_tft.fillScreen(ST7735_BLACK);

            m_tft.setTextSize(m_current_text->text_size);

            int space = (m_current_text->text_size == 2) ? 20 : 5;

            for (int i = m_current_row; i < m_current_text->size; i++)
            {
                if (i - m_current_row >= 6)
                {
                    break;
                }
                m_tft.setCursor(0, 0 + space * (i - m_current_row));
                m_tft.println(m_current_text->rows[i]);
            }

            m_current_row++;
            m_state = State::WAIT_FOR_SCROLLING_TEXT;
        }
        else
        {
            if (m_loop_text)
            {
                m_state = State::SCROLLING_TEXT_START;
            }
            else
            {
                m_state = State::WAIT_FOR_START;
            }
        }
    }
    break;
    case State::WAIT_FOR_SCROLLING_TEXT:
    {
        if (m_micros > m_refresh_rate * 1000)
        {
            m_state = State::SCROLLING_TEXT;
            m_micros = 0;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    }
}

void DisplayPlayer::draw_scrolling_text(DisplayPlayer::Text *text)
{
    m_state = State::SCROLLING_TEXT_START;
    m_current_text = text;
}

void DisplayPlayer::draw_text(DisplayPlayer::Text *text)
{
    m_tft.fillScreen(ST7735_BLACK);
    m_tft.setTextSize(text->text_size);

    int space = 0;
    if (text->text_size == 2)
    {
        space = 20;
    }
    else
    {
        space = 5;
    }

    for (int i = 0; i < text->size; i++)
    {
        m_tft.setCursor(0, 0 + space * i);
        m_tft.println(text->rows[i]);
    }

    m_state = State::WAIT_FOR_START;
}

void DisplayPlayer::draw_distance_and_time(int distance, int duration)
{
    m_state = State::WAIT_FOR_START;

    m_tft.fillScreen(ST7735_BLACK);
    m_tft.setTextSize(2);

    String distanceString = String(distance);
    String durationString = String(duration);

    m_tft.setCursor(0, 0);
    m_tft.print("Distance: ");
    m_tft.println(distanceString);
    m_tft.print("Time: ");
    m_tft.println(durationString);
};

void DisplayPlayer::draw_sub_text(SubText *text)
{
    m_current_sub_text = text;
    if (m_state != State::WAIT_FOR_REFRESH && m_state != State::REFRESH)
    {
        m_state = State::REFRESH;
    }
};

void DisplayPlayer::drawImmediate(DisplayUpdateMethod updateMethod)
{
    m_state = State::WAIT_FOR_START;
    updateMethod(&m_tft);
}

void DisplayPlayer::draw_clock(int hour, int minute)
{
    m_state = State::WAIT_FOR_START;

    m_tft.fillScreen(ST7735_BLACK);
    m_tft.drawCircle(80, 64, 60, ST7735_WHITE);

    float hour_angle = (hour % 12) * 30;
    float minute_angle = minute * 30;

    m_tft.drawLine(80, 64, 80 + 30 * cos((hour_angle - 90) * PI / 180), 64 + 30 * sin((hour_angle - 90) * PI / 180), ST7735_WHITE);
    m_tft.drawLine(80, 64, 80 + 50 * cos((minute_angle - 90) * PI / 180), 64 + 50 * sin((minute_angle - 90) * PI / 180), ST7735_WHITE);
}
