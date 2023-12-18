#include "Game.h"

Game::Game(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer, int tilt_sequence_size, int touch_sequence_size)
    : m_sound_player(soundPlayer), m_distance_player(distancePlayer), m_display_player(displayPlayer), m_touch_player(touchplayer), m_rotate_player(rotatePlayer), m_key_player(keyPlayer), m_tilt_player(tiltPlayer), m_tilt_sequence_size(tilt_sequence_size), m_touch_sequence_size(touch_sequence_size)
{
    Serial.println("Game1");

    // Distance
    int m_distance = random(10, 20);
    int m_duration = random(1, 5);

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

void Game::generate_key_sequence()
{
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
    case State::CLOCK:
    {
        auto segment = this->m_rotate_player->getSegments(1024 / 12);
        auto leftSegment = segment >> 4;
        auto rightSegment = segment & 0b1111;

        int hour = leftSegment == 11 ? 12 : 11 - leftSegment;
        int minute = (11 - rightSegment);

        play_clock_melody(hour, minute);

        if (check_clock_time(hour, minute))
        {
            m_sound_player->playSuccessSound();
            m_state = State::START_TILT;
        }
    }
    break;
    case State::START_TILT:
    {
        m_current_tilt_index = 0;
        m_state = State::TILT;
    }
    break;
    case State::TILT:
    {
        auto tilt = this->m_tilt_player->getTilt();
        if (tilt == TiltPlayer::Tilt::None)
        {
            return false;
        }
        if (tilt == m_last_tilt_input)
        {
            return false;
        }
        m_last_tilt_input = tilt;
        if (m_tilt_sequence[m_current_tilt_index] == tilt)
        {
            m_current_tilt_index++;
            if (m_current_tilt_index > m_tilt_sequence_size - 1)
            {
                // TODO: Next State
                m_sound_player->playSuccessSound();
                m_state = State::START_DISTANCE;
            }
            else
            {
                m_sound_player->playCorrectSound();
                m_state = State::NO_TILT;
            }
        }
        else
        {
            m_sound_player->playFailSound();
            m_current_tilt_index = 0;
            m_state = State::NO_TILT;
        }
    }
    break;
    case State::NO_TILT:
    {
        auto tilt = this->m_tilt_player->getTilt();
        if (tilt == TiltPlayer::Tilt::None)
        {
            m_state = State::TILT;
        }
    }
    break;
    case State::START_DISTANCE:
    {
        if (distance > -1 && distance < 120)
        {
            // Set Update Method for Display with current distance value
            Serial.print("distance: ");
            Serial.println(distance);
            if (distance >= m_distance - 1 && distance <= m_distance + 1)
            {
                m_micros = 0;
                m_state = State::WAIT_DISTANCE;
            }
        }
    }
    break;
    case State::WAIT_DISTANCE:
    {
        if (!check_distance(distance))
        {
            m_state = State::START_DISTANCE;
        }
        else if (m_micros > m_duration * 1000 * 1000)
        {
            // TODO: Next State
            m_sound_player->playSuccessSound();
            m_state = State::ROTATE;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    case State::ROTATE:
    {
        int segments = this->m_rotate_player->getSegments();
        int leftSegment = segments >> 4;
        int rightSegment = segments & 0b1111;

        // Check if we really need this (is used to check if the old values are the same as the new ones)
        m_left_segment = leftSegment;
        m_right_segment = rightSegment;

        if (check_rotate(leftSegment, rightSegment))
        {
            m_state = State::KEY_INPUT;
        }
    }
    break;
    case State::KEY_INPUT:
    {
        int segments = this->m_rotate_player->getSegments();
        int leftSegment = segments >> 4;
        int rightSegment = segments & 0b1111;

        // Check if we need this
        if (m_left_segment != leftSegment || m_right_segment != rightSegment)
        {
            m_state = State::ROTATE;
            return false;
        }

        if (m_sound_player->isEmpty())
        {
            m_sound_player->playSound(m_rotate_melody, m_rotate_melody_durations, m_rotate_melody_size);
        }

        if (this->m_key_player->isNewKey())
        {
            auto key = this->m_key_player->getKey();

            if (key == m_rotate_keys[m_current_rotate_key_index])
            {
                m_current_rotate_key_index++;
                if (m_current_rotate_key_index > 3)
                {
                    m_sound_player->playSuccessSound();
                    m_state = State::START_TOUCH;
                }
            }
            else
            {
                m_sound_player->playFailSound();
                m_current_rotate_key_index = 0;
            }
        }
    }
    break;
    case State::START_TOUCH:
    {
        Serial.println("START_TOUCH");
        m_state = State::TOUCH;
        m_current_touch_index = 0;
        m_last_touch_input = TouchPlayer::Touch::None;
    }
    break;
    case State::TOUCH:
    {
        u8 touch = this->m_touch_player->getTouchInput();

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
        }
        else
        {
            if (touch_count == 0)
            {
                return false;
            }

            if ((touch & static_cast<u8>(m_last_touch_input)) == 0)
            {
                if ((touch & static_cast<u8>(m_touch_sequence[m_current_touch_index])) != 0)
                {
                    m_last_touch_input = m_touch_sequence[m_current_touch_index];
                    m_current_touch_index++;
                    m_sound_player->playCorrectSound();
                    if (m_current_touch_index > m_touch_sequence_size - 1)
                    {
                        m_state = State::END;
                    }
                }
                else
                {
                    m_state = State::FAIL_TOUCH;
                }
            }
        }
    }
    break;
    case State::FAIL_TOUCH:
    {
        if (m_sound_player->isEmpty())
        {
            m_sound_player->playFailSound();
        }
        m_state = State::START_TOUCH;
    }
    break;
    case State::END:
    {
        Serial.println("END");
        // this->m_sound_player.playSound();
        return true;
    }
    break;
    }
    return false;
}
