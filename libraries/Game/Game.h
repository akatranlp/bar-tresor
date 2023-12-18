#include <TouchPlayer.h>
#include <KeyPlayer.h>
#include <TiltPlayer.h>
#include <SoundPlayer.h>
#include <DistancePlayer.h>
#include <DisplayPlayer.h>
#include <RotatePlayer.h>

#pragma once

class Game
{
public:
    Game(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer, int tilt_sequence_size, int touch_sequence_size);

    bool update(unsigned long delta, int distance);

protected:
    // In update functions
    virtual void play_clock_melody(int hour, int minute) = 0;
    virtual bool check_clock_time(int hour, int minute) = 0;
    virtual bool check_rotate(int leftSegment, int rightSegment) = 0;
    virtual bool check_distance(int distance) = 0;

    // Tilt
    TiltPlayer::Tilt m_tilt_sequence[8];
    int m_tilt_sequence_size;

    // Rotate
    int m_rotate_melody[5] = {NOTE_C4, NOTE_E4, NOTE_E4, NOTE_B4, 0};
    const int m_rotate_melody_durations[5] = {4, 4, 4, 4, 1};
    const int m_rotate_melody_size = sizeof(m_rotate_melody) / sizeof(m_rotate_melody[0]);

    int m_left_segment;
    int m_right_segment;

    // Distance
    int m_distance;
    int m_duration;

    SoundPlayer *m_sound_player;
    DistancePlayer *m_distance_player;
    DisplayPlayer *m_display_player;
    TouchPlayer *m_touch_player;
    RotatePlayer *m_rotate_player;
    KeyPlayer *m_key_player;
    TiltPlayer *m_tilt_player;

private:
    void generate_touch_sequence();
    void generate_key_sequence();

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

    State m_state = State::CLOCK;

    // Base
    unsigned long m_micros;

    // Tilt
    TiltPlayer::Tilt m_last_tilt_input = TiltPlayer::Tilt::None;
    int m_current_tilt_index = 0;

    // Rotate
    char m_rotate_keys[4] = {'1', '3', '3', '7'};
    int m_current_rotate_key_index = 0;

    // Touch
    TouchPlayer::Touch m_touch_sequence[8];
    int m_current_touch_index = 0;
    int m_touch_sequence_size;
    TouchPlayer::Touch m_last_touch_input = TouchPlayer::Touch::None;
};
