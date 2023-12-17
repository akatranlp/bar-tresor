#include "Game.h"

Game::Game(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
    : m_sound_player(soundPlayer), m_distance_player(distancePlayer), m_display_player(displayPlayer), m_touch_player(touchplayer), m_rotate_player(rotatePlayer), m_key_player(keyPlayer), m_tilt_player(tiltPlayer)
{
}
