#include "Game.h"

// These are the texts which are displayed on the display in the different stages
static const char *touch_text_rows[] = {
    "Die erste",
    "Aufgabe",
    "ist es",
    "mich in",
    "einer",
    "bestimmten",
    "Reihenfolge",
    "an den dafuer",
    "vorgesehenen",
    "Orten zu",
    "beruehren.",
};

static DisplayPlayer::Text touch_text = DisplayPlayer::Text{
    rows : touch_text_rows,
    size : 11,
    text_size : 2,
};

static const char *tilt_text_rows[] = {
    "Neige mich",
    "in die vier",
    "Himmelsricht-",
    "ungen, um",
    "jenes Raetsel",
    "zu bestehen.",
};

static DisplayPlayer::Text tilt_text = DisplayPlayer::Text{
    rows : tilt_text_rows,
    size : 6,
    text_size : 2,
};

static const char *distance_text_rows[] = {
    "Die Loesung",
    "liegt in der",
    "Hoehe und",
    "Zeit",
    "verborgen.",

    "Ausserdem ist",
    "es wichtig,",
    "mich stark",
    "festzuhalten",

    "und mich im",
    "rechten",
    "Winkel",
    "zu drehen.",
};

static DisplayPlayer::Text distance_text = DisplayPlayer::Text{
    rows : distance_text_rows,
    size : 13,
    text_size : 2,
};

static const char *rotate_text_rows[] = {
    "Stelle die",
    "Schalter so",
    "ein, dass die",
    "versteckten",
    "Toene zu",
    "hoeren sind.",
    "Die Toene",
    "helfen dir",
    "dabei die",
    "Kombination",
    "zum Oeffnen",
    "des Tresors",
    "herauszufinden.",
};

static DisplayPlayer::Text rotate_text = DisplayPlayer::Text{
    rows : rotate_text_rows,
    size : 13,
    text_size : 2,
};

static const char *end_text_rows[] = {
    "Du hast",
    "gewonnen, ",
    "herzlichen",
    "Glueckwunsch",
};

static DisplayPlayer::Text end_text = DisplayPlayer::Text{
    rows : end_text_rows,
    size : 4,
    text_size : 2,
};

Game::Game(SoundPlayer *soundPlayer,
           DistancePlayer *distancePlayer,
           DisplayPlayer *displayPlayer,
           TouchPlayer *touchplayer,
           RotatePlayer *rotatePlayer,
           KeyPlayer *keyPlayer,
           TiltPlayer *tiltPlayer,
           int tilt_sequence_size,
           int touch_sequence_size,
           int distance_text_lenght)
    : m_sound_player(soundPlayer),
      m_distance_player(distancePlayer),
      m_display_player(displayPlayer),
      m_touch_player(touchplayer),
      m_rotate_player(rotatePlayer),
      m_key_player(keyPlayer),
      m_tilt_player(tiltPlayer),
      m_tilt_sequence_size(tilt_sequence_size),
      m_touch_sequence_size(touch_sequence_size),
      m_distance_text_lenght(distance_text_lenght)
{
    // Distance
    m_distance = random(10, 20);
    m_duration = random(1, 5);

    // Touch
    generate_touch_sequence();

    // Tilt
    // generate_tilt_sequence will be called in the child classes

    // Clock
    // generate_clock_values will be called in the child classes

    // Rotate
    generate_key_sequence();
    // generate_rotate_values will be called in the child classes
}

// Generate a random sequence of 4 keys which need to be pressed at the last puzzle
void Game::generate_key_sequence()
{
    return;
    for (int i = 0; i < 4; i++)
    {
        int randomKey = random(0, 12);
        char chr;
        if (randomKey == 10)
        {
            chr = '*';
        }
        else if (randomKey == 11)
        {
            chr = '#';
        }
        else
        {
            chr = '0' + randomKey;
        }

        m_rotate_keys[i] = chr;
        m_rotate_melody[i] = map_key_to_note(chr);
    }
}

// Generate a random sequence of up until 8 sensors to touch (depending on the subclass) at the first puzzle
void Game::generate_touch_sequence()
{
    auto current_touch = 0;
    auto touch_count = 0;

    while (touch_count < m_touch_sequence_size)
    {
        int randomTouch = random(1, 5);
        if (randomTouch == current_touch)
        {
            continue;
        }
        if (randomTouch == 1)
        {
            m_touch_sequence[touch_count] = TouchPlayer::Touch::Up;
        }
        else if (randomTouch == 2)
        {
            m_touch_sequence[touch_count] = TouchPlayer::Touch::Back;
        }
        else if (randomTouch == 3)
        {
            m_touch_sequence[touch_count] = TouchPlayer::Touch::Left;
        }
        else if (randomTouch == 4)
        {
            m_touch_sequence[touch_count] = TouchPlayer::Touch::Right;
        }

        current_touch = randomTouch;
        touch_count++;
    }
}

bool Game::update(unsigned long delta, int distance)
{
    switch (m_state)
    {
    // ------------------------------------
    // --------STATE: INIT TOUCH-----------
    // ------------------------------------
    case State::INIT_TOUCH:
    {
        // Display the text for the touch puzzle
        m_display_player->set_loop_text(true);
        m_display_player->set_refresh_rate(1500);
        m_display_player->draw_scrolling_text(&touch_text);
        m_state = State::START_TOUCH;
    }
    break;
    // ------------------------------------
    // --------STATE: START TOUCH----------
    // ------------------------------------
    case State::START_TOUCH:
    {
        m_state = State::TOUCH;
        m_current_touch_index = 0;
        m_last_touch_input = TouchPlayer::Touch::None;
    }
    break;
    // ------------------------------------
    // -----------STATE: TOUCH-------------
    // ------------------------------------
    case State::TOUCH:
    {
        // Get the value of sensors which are touched
        u8 touch = this->m_touch_player->getTouchInput();

        // Check if more than one sensor is touched
        int touch_count = 0;
        if (touch & TOUCH_UP)
        {
            touch_count++;
        }
        if (touch & TOUCH_BACK)
        {
            touch_count++;
        }
        if (touch & TOUCH_LEFT)
        {
            touch_count++;
        }
        if (touch & TOUCH_RIGHT)
        {
            touch_count++;
        }

        if (touch_count > 1)
        {
            m_state = State::FAIL_TOUCH;
            return false;
        }

        if (touch_count == 0)
        {
            return false;
        }

        // Check if the sensor which is touched is the same as the last one
        if ((touch & static_cast<u8>(m_last_touch_input)) == 0)
        {
            return false;
        }

        // Check if the sensor which is touched is the next one in the sequence
        if ((touch & static_cast<u8>(m_touch_sequence[m_current_touch_index])) == 0)
        {
            m_state = State::FAIL_TOUCH;
            return false;
        }

        m_last_touch_input = m_touch_sequence[m_current_touch_index];
        m_current_touch_index++;
        m_sound_player->playCorrectSound();

        // Check if the sensor which is touched is the last one in the sequence
        if (m_current_touch_index > m_touch_sequence_size - 1)
        {
            m_sound_player->playSuccessSound();
            m_state = State::CLOCK;
        }
    }
    break;
    // ------------------------------------
    // --------STATE: FAIL TOUCH-----------
    // ------------------------------------
    case State::FAIL_TOUCH:
    {
        if (m_sound_player->isEmpty())
        {
            m_sound_player->playFailSound();
        }
        m_state = State::START_TOUCH;
    }
    break;
    // ------------------------------------
    // ----------STATE: CLOCK--------------
    // ------------------------------------
    case State::CLOCK:
    {
        // Get the current value of both potentiometers
        auto segment = m_rotate_player->getSegments(1024 / 12);
        auto leftSegment = segment >> 4;
        auto rightSegment = segment & 0b1111;

        // Calculate the hour and minute value
        int hour = leftSegment == 11 ? 12 : 11 - leftSegment;
        int minute = (11 - rightSegment);

        // Check if the values have changed
        if (leftSegment != m_left_segment || rightSegment != m_right_segment)
        {
            m_display_player->draw_clock(hour, minute);
        }

        play_clock_melody(hour, minute);

        if (check_clock_time(hour, minute))
        {
            m_sound_player->playSuccessSound();
            m_state = State::START_TILT;
        }

        m_left_segment = leftSegment;
        m_right_segment = rightSegment;
    }
    break;
    // ------------------------------------
    // --------STATE: START TILT-----------
    // ------------------------------------
    case State::START_TILT:
    {
        m_display_player->draw_text(&tilt_text);
        m_current_tilt_index = 0;
        m_state = State::TILT;
    }
    break;
    // ------------------------------------
    // -----------STATE: TILT--------------
    // ------------------------------------
    case State::TILT:
    {
        // Get the current tilt value
        auto tilt = m_tilt_player->getTilt();

        // Check if the vault is tilted in any direction
        if (tilt == TiltPlayer::Tilt::None)
        {
            return false;
        }

        // Check if the tilt value has changed
        if (tilt == m_last_tilt_input)
        {
            return false;
        }

        m_last_tilt_input = tilt;

        // Check if the tilt value is the next one in the sequence
        if (m_tilt_sequence[m_current_tilt_index] != tilt)
        {
            m_sound_player->playFailSound();
            m_current_tilt_index = 0;
            m_state = State::NO_TILT;
            return false;
        }

        m_current_tilt_index++;
        // Check if the tilt value is the last one in the sequence
        if (m_current_tilt_index > m_tilt_sequence_size - 1)
        {
            m_sound_player->playSuccessSound();
            m_state = State::INIT_DISTANCE;
        }
        else
        {
            m_sound_player->playCorrectSound();
            m_state = State::NO_TILT;
        }
    }
    break;
    // ------------------------------------
    // ----------STATE: NO TILT------------
    // ------------------------------------
    case State::NO_TILT:
    {
        // After we tilted the vault we need to wait until it is in the upright again
        if (m_tilt_player->getTilt() == TiltPlayer::Tilt::None)
        {
            m_state = State::TILT;
        }
    }
    break;
    // ------------------------------------
    // -------STATE: INIT DISTANCE---------
    // ------------------------------------
    case State::INIT_DISTANCE:
    {
        // Display the text for the distance puzzle
        distance_text.size = m_distance_text_lenght;
        m_display_player->draw_text(&distance_text);
        m_micros = 0;
        m_state = State::DISTANCE_TEXT_DELAY;
    }
    break;
    // ------------------------------------
    // -----STATE: DISTANCE TEXT DELAY-----
    // ------------------------------------
    case State::DISTANCE_TEXT_DELAY:
    {
        // wait 2 seconds before scrolling the text
        if (m_micros > 2000000)
        {
            distance_text.size = m_distance_text_lenght;
            m_display_player->set_loop_text(false);
            m_display_player->set_refresh_rate(2000);
            m_display_player->draw_scrolling_text(&distance_text);
            m_state = State::WAIT_FOR_DISTANCE_TEXT;
            m_micros = 0;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    // ------------------------------------
    // ---STATE: WAIT FOR DISTANCE TEXT----
    // ------------------------------------
    case State::WAIT_FOR_DISTANCE_TEXT:
    {
        // Wait until the text is finished with scrolling
        if (m_display_player->get_state() == DisplayPlayer::State::WAIT_FOR_START)
        {
            m_state = State::START_DISTANCE;
            m_display_player->set_refresh_rate(1000);
            m_display_player->draw_distance_and_time(m_distance, m_duration);
        }
    }
    break;
    // ------------------------------------
    // -------STATE: START DISTANCE--------
    // ------------------------------------
    case State::START_DISTANCE:
    {
        if (distance < 0 || distance > 120)
        {
            return false;
        }

        // Create a string with the current distance value
        char text[5];
        if (distance > 9)
        {
            text[0] = '0' + (distance / 10);
            text[1] = '0' + (distance % 10);
        }
        else
        {
            text[0] = '0';
            text[1] = '0' + distance;
        }
        text[2] = 'c';
        text[3] = 'm';
        text[4] = '\0';

        DisplayPlayer::SubText subText = {
            text : text,
            x : 60,
            y : 64,
            width : 40,
            height : 40,
        };

        // Display the distance value in the middle of the display
        m_display_player->draw_sub_text(&subText);

        // Check if the distance value is in the range of the random value
        if (distance >= m_distance - 1 && distance <= m_distance + 1)
        {
            m_micros = 0;
            m_state = State::WAIT_DISTANCE;
        }
    }
    break;
    // ------------------------------------
    // --------STATE: WAIT DISTANCE--------
    // ------------------------------------
    case State::WAIT_DISTANCE:
    {

        if (distance < 0 || distance > 120)
        {
            m_micros += delta;
            return false;
        }

        if (!check_distance(distance))
        {
            m_state = State::START_DISTANCE;
        }
        else if (m_micros > (unsigned long)m_duration * 1000 * 1000)
        {
            m_sound_player->playSuccessSound();
            m_state = State::INIT_ROTATE;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    // ------------------------------------
    // ---------STATE: INIT ROTATE---------
    // ------------------------------------
    case State::INIT_ROTATE:
    {
        // Display the text for the rotate puzzle
        m_display_player->set_loop_text(true);
        m_display_player->set_refresh_rate(1500);
        m_display_player->draw_scrolling_text(&rotate_text);
        m_state = State::ROTATE;
    }
    break;
    // ------------------------------------
    // -----------STATE: ROTATE------------
    // ------------------------------------
    case State::ROTATE:
    {
        int segments = this->m_rotate_player->getSegments();
        int leftSegment = segments >> 4;
        int rightSegment = segments & 0b1111;

        m_left_segment = leftSegment;
        m_right_segment = rightSegment;

        if (check_rotate(leftSegment, rightSegment))
        {
            m_state = State::KEY_INPUT;
        }
    }
    break;
    // ------------------------------------
    // ---------STATE: KEY INPUT-----------
    // ------------------------------------
    case State::KEY_INPUT:
    {
        int segments = this->m_rotate_player->getSegments();
        int leftSegment = segments >> 4;
        int rightSegment = segments & 0b1111;

        // Check if the segments have changed
        if (m_left_segment != leftSegment || m_right_segment != rightSegment)
        {
            m_state = State::ROTATE;
            return false;
        }

        if (m_sound_player->isEmpty())
        {
            m_sound_player->playSound(m_rotate_melody, m_rotate_melody_durations, m_rotate_melody_size);
        }

        if (!m_key_player->isNewKey())
        {
            return false;
        }

        auto key = m_key_player->getKey();

        // Check if the key is the next one in the sequence
        if (key == m_rotate_keys[m_current_rotate_key_index])
        {
            m_current_rotate_key_index++;

            // Check if the key is the last one in the sequence
            if (m_current_rotate_key_index > 3)
            {
                m_sound_player->playSuccessSound();
                m_state = State::END;
            }
        }
        else
        {
            m_sound_player->playFailSound();
            m_current_rotate_key_index = 0;
        }
    }
    break;
    // ------------------------------------
    // ------------STATE: END--------------
    // ------------------------------------
    case State::END:
    {
        // Display the text for the end and win the game by returning true
        m_display_player->draw_text(&end_text);
        return true;
    }
    break;
    }
    return false;
}
