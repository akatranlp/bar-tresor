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
    Game(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer);

    // virtual void generateTouch() = 0;

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
