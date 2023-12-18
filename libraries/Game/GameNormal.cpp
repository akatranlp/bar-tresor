#include "GameNormal.h"

GameNormal::GameNormal(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
    : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer, tiltPlayer, 4, 6, 9)
{
    Serial.println("GameNormal");

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

void GameNormal::generate_tilt_sequence()
{
    // Each direction should be in the sequence once in a random order
    int orientation1 = random(0, 4);
    int orientation2;
    do
    {
        orientation2 = random(0, 4);
    } while (orientation2 == orientation1);

    int orientation3;
    do
    {
        orientation3 = random(0, 4);
    } while (orientation3 == orientation2 || orientation3 == orientation1);

    int orientation4;
    do
    {
        orientation4 = random(0, 4);
    } while (orientation3 == orientation2 || orientation3 == orientation1 || orientation4 == orientation3);

    m_tilt_sequence[0] = static_cast<TiltPlayer::Tilt>(orientation1);
    m_tilt_sequence[1] = static_cast<TiltPlayer::Tilt>(orientation2);
    m_tilt_sequence[2] = static_cast<TiltPlayer::Tilt>(orientation3);
    m_tilt_sequence[3] = static_cast<TiltPlayer::Tilt>(orientation4);
}

void GameNormal::generate_clock_values()
{
    m_hour = random(1, 13);
    m_minute = random(0, 4);
}

void GameNormal::generate_rotate_values(int leftSegment, int rightSegment)
{
    // Generate a random left segment that is not the current left segment
    do
    {
        m_left_rotate_segment = random(0, 8);
    } while (m_left_rotate_segment == leftSegment);

    // Generate a random right segment that is not the current right segment
    do
    {
        m_right_rotate_segment = random(0, 8);
    } while (m_right_rotate_segment == rightSegment);
}

void GameNormal::play_clock_melody(int hour, int minute)
{
    if (!m_sound_player->isEmpty())
    {
        return;
    }

    // Play as many notes as the hour is and then one note for every 15 minutes
    int melody[17] = {};
    int durations[17] = {};
    for (int i = 0; i < m_hour; i++)
    {
        melody[i] = NOTE_C2;
        durations[i] = 4;
    }
    melody[m_hour] = 0;
    durations[m_hour] = 1;

    for (int i = m_hour + 1; i < m_hour + 1 + m_minute; i++)
    {
        melody[i] = NOTE_C4;
        durations[i] = 4;
    }
    melody[m_hour + 1 + m_minute] = 0;
    durations[m_hour + 1 + m_minute] = 1;

    m_sound_player->playSound(melody, durations, m_hour + 1 + m_minute + 1);
}

bool GameNormal::check_clock_time(int hour, int minute)
{
    // The hour must be correct and the minute must be within the the right 15 minutes (first 3 segments);
    return hour == m_hour && (minute / 3) == m_minute;
}

bool GameNormal::check_rotate(int leftSegment, int rightSegment)
{
    bool left_correct = leftSegment == m_left_rotate_segment;
    bool right_correct = rightSegment == m_right_rotate_segment;

    // If both segments are correct, return true
    if (left_correct && right_correct)
    {
        return true;
    }
    // If left segment is correct, play the first two notes of the keypad notes
    else if (left_correct)
    {
        if (this->m_sound_player->isEmpty())
        {
            int melody[] = {m_rotate_melody[0], m_rotate_melody[1], 0};
            int durations[] = {m_rotate_melody_durations[0], m_rotate_melody_durations[1], 1};
            this->m_sound_player->playSound(melody, durations, 3);
        }
    }
    // If right segment is correct, play the last two notes of the keypad notes
    else if (right_correct)
    {
        if (this->m_sound_player->isEmpty())
        {
            int melody[] = {m_rotate_melody[2], m_rotate_melody[3], 0};
            int durations[] = {m_rotate_melody_durations[2], m_rotate_melody_durations[3], 1};
            this->m_sound_player->playSound(melody, durations, 3);
        }
    }
    return false;
}

bool GameNormal::check_distance(int distance)
{
    int touch = m_touch_player->getTouchInput();

    // The distance must be within 1 of the correct distance
    auto distance_check = distance >= m_distance - 1 && distance <= m_distance + 1;

    // All toch fields must be touched
    auto touch_check = (touch & 0b1111) == 0b1111;

    return distance_check && touch_check;
}
