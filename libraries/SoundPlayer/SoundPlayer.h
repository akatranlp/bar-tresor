#include <pitches.h>
#include <Arduino.h>

#pragma once

#define NOTE_CAPACITY 128

struct Note
{
    long note;
    long duration;
};

class SoundPlayer
{
public:
    SoundPlayer(int sound_pin);

    void playSound(const int melody[], const int noteDurations[], int arr_len);

    void playFailSound();

    void playSuccessSound();

    void playCorrectSound();

    bool isEmpty();

    void addTone(int note, int duration);

    void update(unsigned long delta);

private:
    enum class State
    {
        WAITING_FOR_NOTE,
        START_NOTE,
        PLAYING_NOTE,
        STOP_NOTE,
        PAUSE_NOTE
    };

    State m_state = State::WAITING_FOR_NOTE;

    // checken ob das jetzt geht oder nicht
    Note m_notes[NOTE_CAPACITY];

    int m_notes_current = 0;
    int m_notes_size = 0;

    int m_sound_pin;
    unsigned long m_micros = 0;
};
