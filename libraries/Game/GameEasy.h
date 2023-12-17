#include "Game.h"

#pragma once

class GameEasy final : public Game
{
public:
    GameEasy(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer);

    virtual bool update(unsigned long delta, int distance) override;

private:
    // reihgenfolge
    // 1. Touchi
    // 2. Clock
    // 3. Tilt
    // 4. Distance
    // 5. Keypad
    enum class State
    {
        CLOCK,
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

    int m_rotate_segment;
    int m_left_Segment;
    int m_right_Segment;

    int m_distance;
    int m_duration;

    int m_hour;

    int m_keyIndex = 0;
    int m_tilt_index = 0;
    TiltPlayer::Tilt m_last_tilt_input = TiltPlayer::Tilt::None;

    unsigned long m_micros;

    int m_current_touch_index = 0;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;

    State m_state = State::CLOCK;

    TouchPlayer::Touch m_rightTouches[4];
};