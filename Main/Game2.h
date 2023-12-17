#pragma once

class Game2 : public Game
{
public:
    Game2(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer)
        : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer)
    {
        Serial.println("Game2");
        auto current_touch = 0;
        auto touch_count = 0;

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

        Serial.print("leftSegment: ");
        Serial.print(leftSegment);
        Serial.print(" - ");
        Serial.print("rightSegment: ");
        Serial.print(rightSegment);
        Serial.print(" - ");
        Serial.print("m_left_rotate_segment: ");
        Serial.print(m_left_rotate_segment);
        Serial.print(" - ");
        Serial.print("m_right_rotate_segment: ");
        Serial.println(m_right_rotate_segment);

        while (touch_count < 6)
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
        case State::ROTATE:
        {
            int segments = this->m_rotate_player->getSegments();
            int leftSegment = segments >> 4;
            int rightSegment = segments & 0b1111;

            m_left_segment = leftSegment;
            m_right_segment = rightSegment;

            int size = 0;
            bool left_correct = leftSegment == m_left_rotate_segment;
            bool right_correct = rightSegment == m_right_rotate_segment;

            if (left_correct && right_correct)
            {
                m_state = State::KEY_INPUT;
            }
            else if (left_correct)
            {
                if (this->m_sound_player->isEmpty())
                {
                    int melody[] = {rotateMelody[0], rotateMelody[1], 0};
                    int durations[] = {rotateNoteDurations[0], rotateNoteDurations[1], 1};
                    this->m_sound_player->playSound(melody, durations, 3);
                }
            }
            else if (right_correct)
            {
                if (this->m_sound_player->isEmpty())
                {
                    int melody[] = {rotateMelody[2], rotateMelody[3], 0};
                    int durations[] = {rotateNoteDurations[2], rotateNoteDurations[3], 1};
                    this->m_sound_player->playSound(melody, durations, 3);
                }
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
                        if (m_current_touch_index > 5)
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
        ROTATE,
        KEY_INPUT,
        START_TOUCH,
        TOUCH,
        FAIL_TOUCH,
        END,
    };

    int m_left_segment;
    int m_right_segment;
    int m_left_rotate_segment;
    int m_right_rotate_segment;

    int m_keyIndex = 0;

    int m_current_touch_index = 0;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;

    State m_state = State::ROTATE;
};