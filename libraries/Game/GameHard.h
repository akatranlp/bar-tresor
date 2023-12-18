/* *******************************************************
 *  GameHard.h
 *  This is a child class of Game.h
 ********************************************************/
#include "Game.h"

#pragma once

class GameHard : public Game
{
public:
    GameHard(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer);

protected:
    // In update functions
    virtual void play_clock_melody(int hour, int minute) override;
    virtual bool check_clock_time(int hour, int minute) override;
    virtual bool check_rotate(int leftSegment, int rightSegment) override;
    virtual bool check_distance(int distance) override;

private:
    // generate Values functions
    void generate_tilt_sequence();
    void generate_clock_values();
    void generate_rotate_values(int leftSegment, int rightSegment);

    int m_hour;
    int m_minute;

    int m_left_rotate_segment;
    int m_right_rotate_segment;
};
