#include "GameHard.h"

GameHard::GameHard(SoundPlayer *soundPlayer, DistancePlayer *distancePlayer, DisplayPlayer *displayPlayer, TouchPlayer *touchplayer, RotatePlayer *rotatePlayer, KeyPlayer *keyPlayer, TiltPlayer *tiltPlayer)
    : Game(soundPlayer, distancePlayer, displayPlayer, touchplayer, rotatePlayer, keyPlayer, tiltPlayer, 8, 8, 13)
{
    Serial.println("GameHard");

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

void GameHard::generate_tilt_sequence()
{
    int last_orientation = random(0, 4);
    m_tilt_sequence[0] = static_cast<TiltPlayer::Tilt>(last_orientation);

    for (int i = 1; i < m_tilt_sequence_size; i++)
    {
        int orientation;
        do
        {
            orientation = random(0, 4);
        } while (orientation == last_orientation);

        last_orientation = orientation;
        m_tilt_sequence[i] = static_cast<TiltPlayer::Tilt>(orientation);
    }
}

void GameHard::generate_clock_values()
{
    m_hour = random(1, 13);
    m_minute = random(0, 12);
}

void GameHard::generate_rotate_values(int leftSegment, int rightSegment)
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

void GameHard::play_clock_melody(int hour, int minute)
{
    if (this->m_sound_player->isEmpty())
    {
        int melody[25] = {};
        int durations[25] = {};
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

bool GameHard::check_clock_time(int hour, int minute)
{
    return hour == m_hour && minute == m_minute;
}

bool GameHard::check_rotate(int leftSegment, int rightSegment)
{
    return leftSegment == m_left_rotate_segment && rightSegment == m_right_rotate_segment;
}

bool GameHard::check_distance(int distance)
{
    int touch = this->m_touch_player->getTouchInput();
    TiltPlayer::Tilt tilt = this->m_tilt_player->getTilt();

    Serial.print("touch: ");
    Serial.println(touch);

    auto distance_check = distance >= m_distance - 1 && distance <= m_distance + 1;
    auto touch_check = (touch & 0b1111) == 0b1111;
    auto tilt_check = tilt != TiltPlayer::Tilt::None;

    return distance_check && touch_check && tilt_check;
}
