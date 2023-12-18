/* *******************************************************
 *  SoundPlayer.h
 *  This library is used to play sounds on a piezo speaker
 *  It uses an fixed size array to store the notes
 *
 *  The notes are played in a non-blocking way
 ********************************************************/
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
    void addTone(int note, int duration);

    void playFailSound();
    void playCorrectSound();
    void playSuccessSound();

    bool isEmpty();

    void update(unsigned long delta);

private:
    int m_sound_pin;

    // States of the state machine
    enum class State
    {
        WAITING_FOR_NOTE,
        START_NOTE,
        PLAYING_NOTE,
        PAUSE_NOTE
    };

    State m_state = State::WAITING_FOR_NOTE;
    unsigned long m_micros = 0;

    Note m_notes[NOTE_CAPACITY];
    int m_notes_size = 0;
    int m_notes_current = 0;
};
