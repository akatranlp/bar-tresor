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
    do
    {
        m_left_rotate_segment = random(0, 8);
    } while (m_left_rotate_segment == leftSegment);

    do
    {
        m_right_rotate_segment = random(0, 8);
    } while (m_right_rotate_segment == rightSegment);
}

void GameNormal::play_clock_melody(int hour, int minute)
{
    if (this->m_sound_player->isEmpty())
    {
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

        this->m_sound_player->playSound(melody, durations, m_hour + 1 + m_minute + 1);
    }
}

bool GameNormal::check_clock_time(int hour, int minute)
{
    return hour == m_hour && (minute / 3) == m_minute;
}

bool GameNormal::check_rotate(int leftSegment, int rightSegment)
{
    bool left_correct = leftSegment == m_left_rotate_segment;
    bool right_correct = rightSegment == m_right_rotate_segment;

    if (left_correct && right_correct)
    {
        return true;
    }
    else if (left_correct)
    {
        if (this->m_sound_player->isEmpty())
        {
            int melody[] = {m_rotate_melody[0], m_rotate_melody[1], 0};
            int durations[] = {m_rotate_melody_durations[0], m_rotate_melody_durations[1], 1};
            this->m_sound_player->playSound(melody, durations, 3);
        }
    }
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
    int touch = this->m_touch_player->getTouchInput();

    Serial.print("touch: ");
    Serial.println(touch);

    auto distance_check = distance >= m_distance - 1 && distance <= m_distance + 1;
    auto touch_check = (touch & 0b1111) == 0b1111;

    return distance_check && touch_check;
}
