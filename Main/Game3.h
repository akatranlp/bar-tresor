#pragma once

class Game3 : public Game
{
public:
    Game3(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
        : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer, tiltPlayer)
    {
        Serial.println("Game3");

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

    virtual bool update(unsigned long delta, int distance) override
    {
        switch (m_state)
        {

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
                if (distance > 10 && distance < 12)
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

            if ((distance != -1 && distance < 10 && distance > 12) || (touch & 0b1111) != 0b1111 || tilt == TiltPlayer::Tilt::None)
            {
                m_state = State::START_DISTANCE;
            }

            if (m_micros > 3000000)
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
                    currentKeys[m_keyIndex] = key;
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

private:
    enum class State
    {
        START_TILT,
        TILT,
        NO_TILT,
        START_DISTANCE,
        WAIT_DISTANCE,
        ROTATE,
        KEY_INPUT,
        START_TOUCH,
        TOUCH,
        FAIL_TOUCH,
        END,
    };

    unsigned long m_micros;

    int m_left_segment;
    int m_right_segment;
    int m_left_rotate_segment;
    int m_right_rotate_segment;

    int m_keyIndex = 0;
    int m_tilt_index = 0;
    TiltPlayer::Tilt m_last_tilt_input = TiltPlayer::Tilt::None;

    int m_current_touch_index = 0;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;

    State m_state = State::START_TILT;
};