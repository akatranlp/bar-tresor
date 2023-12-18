#include "Game.h"

#pragma once

class GameEasy final : public Game
{
public:
    GameEasy(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer);

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

    int m_rotate_segment;

    int m_hour;
};
