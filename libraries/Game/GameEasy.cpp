#include "GameEasy.h"

GameEasy::GameEasy(
    SoundPlayer *soundPlayer,
    DistancePlayer *distancePlayer,
    DisplayPlayer *displayPlayer,
    TouchPlayer *touchplayer,
    RotatePlayer *rotatePlayer,
    KeyPlayer *keyPlayer,
    TiltPlayer *tiltPlayer)
    : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer, tiltPlayer, 4, 4, 5)
{
    // Tilt
    generate_tilt_sequence();

    // Clock
    generate_clock_values();

    // Rotate
    int segments = this->m_rotate_player->getSegments();
    int leftSegment = segments >> 4;
    int rightSegment = segments & 0b1111;

    generate_rotate_values(leftSegment, rightSegment);
    m_left_segment = leftSegment;
    m_right_segment = rightSegment;
}

void GameEasy::generate_tilt_sequence()
{
    m_tilt_sequence[0] = TiltPlayer::Tilt::Backward;
    m_tilt_sequence[1] = TiltPlayer::Tilt::Right;
    m_tilt_sequence[2] = TiltPlayer::Tilt::Forward;
    m_tilt_sequence[3] = TiltPlayer::Tilt::Left;
}

void GameEasy::generate_clock_values()
{
    m_hour = random(1, 13);
}

void GameEasy::generate_rotate_values(int leftSegment, int rightSegment)
{
    // Generate a random segment that is not the current left or right segment
    do
    {
        m_rotate_segment = random(0, 8);
    } while (m_rotate_segment == leftSegment || m_rotate_segment == rightSegment);
}

void GameEasy::play_clock_melody(int hour, int minute)
{
    if (!m_sound_player->isEmpty())
    {
        return;
    }

    // Play as many notes as the hour is
    int melody[13] = {};
    int durations[13] = {};
    for (int i = 0; i < m_hour; i++)
    {
        melody[i] = NOTE_C2;
        durations[i] = 4;
    }
    melody[m_hour] = 0;
    durations[m_hour] = 1;

    m_sound_player->playSound(melody, durations, m_hour + 1);
}

bool GameEasy::check_clock_time(int hour, int minute)
{
    // The hour must be correct and the minute must be within the first 15 minutes (first 3 segments);
    return hour == m_hour && (minute / 3) == 0;
}

bool GameEasy::check_rotate(int leftSegment, int rightSegment)
{
    // The left or right segment must be in the right segment to hear the sound for the keypad puzzle
    return leftSegment == m_rotate_segment || rightSegment == m_rotate_segment;
}

bool GameEasy::check_distance(int distance)
{
    // The distance must be within 1 of the correct distance
    return distance >= m_distance - 1 && distance <= m_distance + 1;
}
