#pragma once

TouchPlayer::Touch rightTouches[12] = {TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None, TouchPlayer::Touch::None};

TiltPlayer::Tilt tiltSequence[8] = {TiltPlayer::Tilt::Backward, TiltPlayer::Tilt::Right, TiltPlayer::Tilt::Forward, TiltPlayer::Tilt::Left, TiltPlayer::Tilt::None, TiltPlayer::Tilt::None, TiltPlayer::Tilt::None, TiltPlayer::Tilt::None};

class Game
{
public:
    Game(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
        : m_sound_player(soundPlayer), m_distance_player(distancePlayer), m_display_player(displayPlayer), m_touch_player(touchplayer), m_rotate_player(rotatePlayer), m_key_player(keyPlayer), m_tilt_player(tiltPlayer)

    {
    }
    virtual bool update(unsigned long delta, int distance) = 0;

protected:
    SoundPlayer *m_sound_player;
    DistancePlayer *m_distance_player;
    DisplayPlayer *m_display_player;
    TouchPlayer *m_touch_player;
    RotatePlayer *m_rotate_player;
    KeyPlayer *m_key_player;
    TiltPlayer *m_tilt_player;
};