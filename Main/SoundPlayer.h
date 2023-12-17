#include "./pitches.h"

#pragma once

#define IMPLEMENTATION LIFO
#define NOTE_CAPACITY 128

const int failSound[] = {NOTE_C3, NOTE_A2, 0};
const int failDurations[] = {8, 4, 2};
const int failSize = sizeof(failSound) / sizeof(failSound[0]);

const int successSound[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
const int successDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
const int successSize = sizeof(successSound) / sizeof(successSound[0]);

const int correctSound = NOTE_A4;
const int correctDurations = 4;

struct Note
{
    long note;
    long duration;
};

Note m_notes[NOTE_CAPACITY];

class SoundPlayer
{
public:
    SoundPlayer(int sound_pin)
        : m_sound_pin(sound_pin)
    {
        pinMode(sound_pin, OUTPUT);
    }

    void playSound(const int melody[], const int noteDurations[], int arr_len)
    {
        m_notes_size = 0;
        m_notes_current = 0;
        m_state = State::START_NOTE;
        noTone(m_sound_pin);
        for (int i = 0; i < arr_len; ++i)
        {
            int noteDuration = 800 / noteDurations[i];
            Note note = Note{melody[i], noteDuration};
            m_notes_size++;
            m_notes[m_notes_size - 1] = note;
        }
    }

    void playFailSound()
    {
        playSound(failSound, failDurations, failSize);
    }

    void playSuccessSound()
    {
        playSound(successSound, successDurations, successSize);
    }

    void playCorrectSound()
    {
        addTone(correctSound, correctDurations);
    }

    bool isEmpty()
    {
        bool isEmpty = m_notes_current == m_notes_size;
        return isEmpty;
    }

    void addTone(int note, int duration)
    {
        int noteDuration = 800 / duration;
        Note _note = Note{note, noteDuration};
        m_notes_size++;
        m_notes[m_notes_size - 1] = _note;
        if (m_state == State::WAITING_FOR_NOTE)
        {
            m_state = State::START_NOTE;
        }
    }

    void update(unsigned long delta)
    {
        switch (m_state)
        {
        case State::WAITING_FOR_NOTE:
            break;
        case State::START_NOTE:
            if (!isEmpty())
            {
                Note *note = &m_notes[m_notes_current];
                tone(m_sound_pin, note->note, note->duration);
                m_state = State::PLAYING_NOTE;
                m_micros = 0;
            }
            else
            {
                m_state = State::WAITING_FOR_NOTE;
            }
            break;
        case State::PLAYING_NOTE:
            if (!isEmpty())
            {
                Note *note = &m_notes[m_notes_current];
                if (m_micros >= note->duration * 800)
                {
                    m_state = State::PAUSE_NOTE;
                    noTone(m_sound_pin);
                    m_micros = 0;
                }
                else
                {
                    m_micros += delta;
                }
            }
            else
            {
                m_state = State::WAITING_FOR_NOTE;
            }
            break;
        case State::PAUSE_NOTE:
            if (!isEmpty())
            {
                Note *note = &m_notes[m_notes_current];
                if (m_micros >= note->duration * 800 * 1.3)
                {
                    m_state = State::STOP_NOTE;
                }
                else
                {
                    m_micros += delta;
                }
            }
            else
            {
                m_state = State::WAITING_FOR_NOTE;
            }
            break;
        case State::STOP_NOTE:
            m_notes_current++;
            m_state = State::START_NOTE;
            break;
        }
    }

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
    int m_notes_current = 0;
    int m_notes_size = 0;

    int m_sound_pin;
    unsigned long m_micros = 0;
};
