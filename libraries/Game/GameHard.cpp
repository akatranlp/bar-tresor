#include "GameHard.h"

static const char rotateKeys[] = {'1', '3', '3', '7'};
static const int rotateMelody[] = {NOTE_C4, NOTE_E4, NOTE_E4, NOTE_B4, 0};
static const int rotateNoteDurations[] = {4, 4, 4, 4, 1};
static const int rotateMelodySize = sizeof(rotateMelody) / sizeof(rotateMelody[0]);

static TouchPlayer::Touch rightTouches[8] = {};
static TiltPlayer::Tilt tiltSequence[8] = {};

GameHard::GameHard(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
    : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer, tiltPlayer)
{
    Serial.println("Game3");

    int m_distance = random(10, 20);
    int m_duration = random(1, 5);

    int segments = this->m_rotate_player->getSegments();
    int leftSegment = segments >> 4;
    int rightSegment = segments & 0b1111;

    do
    {
        m_left_rotate_segment = random(0, 8);
    } while (m_left_rotate_segment == leftSegment);
    m_left_segment = leftSegment;

    do
    {
        m_right_rotate_segment = random(0, 8);
    } while (m_right_rotate_segment == rightSegment);
    m_right_segment = rightSegment;

    int last_orientation = random(0, 4);
    tiltSequence[0] = static_cast<TiltPlayer::Tilt>(last_orientation);

    for (int i = 1; i < 8; i++)
    {
        int orientation;
        do
        {
            orientation = random(0, 4);
        } while (orientation == last_orientation);
        last_orientation = orientation;
        tiltSequence[i] = static_cast<TiltPlayer::Tilt>(orientation);
    }

    m_hour = random(1, 13);
    m_minute = random(0, 12);

    auto current_touch = 0;
    auto touch_count = 0;

    while (touch_count < 8)
    {
        int randomTouch = random(1, 5);
        if (randomTouch == current_touch)
        {
            continue;
        }
        if (randomTouch == 1)
        {
            rightTouches[touch_count] = TouchPlayer::Touch::Up;
        }
        else if (randomTouch == 2)
        {
            rightTouches[touch_count] = TouchPlayer::Touch::Back;
        }
        else if (randomTouch == 3)
        {
            rightTouches[touch_count] = TouchPlayer::Touch::Left;
        }
        else if (randomTouch == 4)
        {
            rightTouches[touch_count] = TouchPlayer::Touch::Right;
        }

        current_touch = randomTouch;
        touch_count++;
    }
}

bool GameHard::update(unsigned long delta, int distance)
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

        if (this->m_sound_player->isEmpty())
        {
            int melody[25] = {};
            int durations[25] = {};
            for (int i = 0; i < m_hour; i++)
            {
                melody[i] = NOTE_C2;
                durations[i] = 4;
            }
            melody[m_hour] = 0;
            durations[m_hour] = 1;

            for (int i = m_hour + 1; i < m_hour + 1 + m_minute; i++)
            {
                melody[i] = NOTE_C4;
                durations[i] = 4;
            }
            melody[m_hour + 1 + m_minute] = 0;
            durations[m_hour + 1 + m_minute] = 1;

            this->m_sound_player->playSound(melody, durations, m_hour + 1 + m_minute + 1);
        }

        /* Serial.print("hour: ");
        Serial.print(hour);
        Serial.print(" - ");
        Serial.print("minute: ");
        Serial.println(minute); */

        if (hour == m_hour && minute == m_minute)
        {
            this->m_sound_player->playSuccessSound();
            m_state = State::START_TILT;
        }
    }
    break;
    case State::START_TILT:
    {
        m_tilt_index = 0;
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
        if (tiltSequence[m_tilt_index] == tilt)
        {
            m_tilt_index++;
            if (m_tilt_index > 7)
            {
                this->m_sound_player->playSuccessSound();
                m_state = State::START_DISTANCE;
            }
            else
            {
                this->m_sound_player->playCorrectSound();
                m_state = State::NO_TILT;
            }
        }
        else
        {
            this->m_sound_player->playFailSound();
            m_tilt_index = 0;
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
        if (distance == -1)
        {
        }
        else if (distance > 120)
        {
        }
        else
        {
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
        int touch = this->m_touch_player->getTouchInput();
        TiltPlayer::Tilt tilt = this->m_tilt_player->getTilt();

        Serial.print("touch: ");
        Serial.println(touch);

        if ((distance != -1 && distance < m_distance - 1 && distance > m_distance + 1) || (touch & 0b1111) != 0b1111 || tilt == TiltPlayer::Tilt::None)
        {
            m_state = State::START_DISTANCE;
        }

        if (m_micros > m_duration * 1000 * 1000)
        {
            this->m_sound_player->playSuccessSound();
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

        m_left_segment = leftSegment;
        m_right_segment = rightSegment;

        if (leftSegment == m_left_rotate_segment && rightSegment == m_right_rotate_segment)
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

        if (m_left_segment != leftSegment || m_right_segment != rightSegment)
        {
            m_state = State::ROTATE;
        }

        if (this->m_sound_player->isEmpty())
        {
            this->m_sound_player->playSound(rotateMelody, rotateNoteDurations, rotateMelodySize);
        }

        if (this->m_key_player->isNewKey())
        {
            auto key = this->m_key_player->getKey();
            Serial.print("key: ");
            Serial.println(key);

            if (key == rotateKeys[m_keyIndex])
            {
                m_keyIndex++;
                if (m_keyIndex > 3)
                {
                    this->m_sound_player->playSuccessSound();
                    m_state = State::START_TOUCH;
                }
            }
            else
            {
                this->m_sound_player->playFailSound();
                m_keyIndex = 0;
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

            if ((touch & static_cast<u8>(this->m_last_touch_input)) == 0)
            {
                if ((touch & static_cast<u8>(rightTouches[m_current_touch_index])) != 0)
                {
                    m_last_touch_input = rightTouches[m_current_touch_index];
                    m_current_touch_index++;
                    if (m_current_touch_index > 7)
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
            this->m_sound_player->playFailSound();
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
    }
    return false;
}