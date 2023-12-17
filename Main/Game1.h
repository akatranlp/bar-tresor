#pragma once

const char rotateKeys[] = {'1', '3', '3', '7'};
const int rotateMelody[] = {NOTE_C4, NOTE_E4, NOTE_E4, NOTE_B4, 0};
const int rotateNoteDurations[] = {4, 4, 4, 4, 1};
const int rotateMelodySize = sizeof(rotateMelody) / sizeof(rotateMelody[0]);

char currentKeys[] = {0, 0, 0, 0};

class Game1 : public Game
{
public:
    Game1(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer)
        : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer)
    {
        Serial.println("Game1");

        int segments = this->m_rotate_player->getSegments();
        int leftSegment = segments >> 4;
        int rightSegment = segments & 0b1111;

        do
        {
            m_rotate_segment = random(0, 8);
        } while (m_rotate_segment == leftSegment || m_rotate_segment == rightSegment);
        m_left_Segment = leftSegment;
        m_right_Segment = rightSegment;

        Serial.print("leftSegment: ");
        Serial.print(leftSegment);
        Serial.print(" - ");
        Serial.print("rightSegment: ");
        Serial.print(rightSegment);
        Serial.print(" - ");
        Serial.print("m_rotate_segment: ");
        Serial.println(m_rotate_segment);

        auto current_touch = 0;
        auto touch_count = 0;

        while (touch_count < 4)
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

            m_left_Segment = leftSegment;
            m_right_Segment = rightSegment;

            if (leftSegment == m_rotate_segment || rightSegment == m_rotate_segment)
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

            if (m_left_Segment != leftSegment || m_right_Segment != rightSegment)
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
                        this->m_sound_player->playCorrectSound();
                        if (m_current_touch_index > 3)
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

    int m_rotate_segment;
    int m_left_Segment;
    int m_right_Segment;

    int m_keyIndex = 0;

    int m_current_touch_index = 0;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;

    State m_state = State::ROTATE;
};