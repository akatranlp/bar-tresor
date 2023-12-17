#include "./Game.h"

#pragma once

class GameNormal final : public Game
{
public:
    GameNormal(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer);

    virtual bool update(unsigned long delta, int distance) override;

private:
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

    unsigned long m_micros;

    int m_left_segment;
    int m_right_segment;
    int m_left_rotate_segment;
    int m_right_rotate_segment;

    int m_hour;
    int m_minute;

    int m_distance;
    int m_duration;

    int m_keyIndex = 0;
    int m_tilt_index = 0;
    TiltPlayer::Tilt m_last_tilt_input = TiltPlayer::Tilt::None;

    int m_current_touch_index = 0;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;

    State m_state = State::CLOCK;
};