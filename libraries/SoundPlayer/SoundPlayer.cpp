#include "SoundPlayer.h"

#define NOTE_LENGTH 800

// some static arrays for the sounds
static const int failSound[] = {NOTE_C3, NOTE_A2, 0};
static const int failDurations[] = {8, 4, 2};
static const int failSize = sizeof(failSound) / sizeof(failSound[0]);

static const int successSound[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
static const int successDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
static const int successSize = sizeof(successSound) / sizeof(successSound[0]);

static const int correctSound = NOTE_A4;
static const int correctDurations = 4;

// TODO: Try the internal array
// static Note notes[NOTE_CAPACITY];

SoundPlayer::SoundPlayer(int sound_pin)
    : m_sound_pin(sound_pin)
{
    pinMode(sound_pin, OUTPUT);
}

// Reset the note array and replace it with the new notes
void SoundPlayer::playSound(const int melody[], const int noteDurations[], int arr_len)
{
    m_notes_size = 0;
    m_notes_current = 0;
    m_state = State::START_NOTE;
    noTone(m_sound_pin);
    for (int i = 0; i < arr_len; ++i)
    {
        int noteDuration = NOTE_LENGTH / noteDurations[i];
        Note note = Note{melody[i], noteDuration};
        m_notes_size++;
        m_notes[m_notes_size - 1] = note;
    }
}

// Add a note to the note array
void SoundPlayer::addTone(int note, int duration)
{
    int noteDuration = NOTE_LENGTH / duration;
    Note _note = Note{note, noteDuration};
    m_notes_size++;
    m_notes[m_notes_size - 1] = _note;
    if (m_state == State::WAITING_FOR_NOTE)
    {
        m_state = State::START_NOTE;
    }
}

void SoundPlayer::playFailSound()
{
    playSound(failSound, failDurations, failSize);
}

void SoundPlayer::playSuccessSound()
{
    playSound(successSound, successDurations, successSize);
}

void SoundPlayer::playCorrectSound()
{
    addTone(correctSound, correctDurations);
}

bool SoundPlayer::isEmpty()
{
    bool isEmpty = m_notes_current == m_notes_size;
    return isEmpty;
}

// Update the state machine
void SoundPlayer::update(unsigned long delta)
{
    switch (m_state)
    {
    // ------------------------------------
    // ------STATE: WAITING FOR NOTE-------
    // ------------------------------------
    case State::WAITING_FOR_NOTE:
        // Infinite loop till one or more notes are added
        break;
    // ------------------------------------
    // ---------STATE: START NOTE----------
    // ------------------------------------
    case State::START_NOTE:
    {
        if (isEmpty())
        {
            m_state = State::WAITING_FOR_NOTE;
            return;
        }

        // Start playing the next note in the array
        Note *note = &m_notes[m_notes_current];
        tone(m_sound_pin, note->note, note->duration);
        m_state = State::PLAYING_NOTE;
        m_micros = 0;
    }
    break;
    // ------------------------------------
    // --------STATE: PLAYING NOTE---------
    // ------------------------------------
    case State::PLAYING_NOTE:
    {
        if (isEmpty())
        {
            m_state = State::WAITING_FOR_NOTE;
            return;
        }

        // Check if the note has been played for the correct duration
        Note *note = &m_notes[m_notes_current];
        if (m_micros >= note->duration * NOTE_LENGTH)
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
    break;
    // ------------------------------------
    // ---------STATE: PAUSE NOTE----------
    // ------------------------------------
    case State::PAUSE_NOTE:
    {
        if (isEmpty())
        {
            m_state = State::WAITING_FOR_NOTE;
            return;
        }

        // Check if the pause has been long enough to start the next note
        Note *note = &m_notes[m_notes_current];
        if (m_micros >= note->duration * NOTE_LENGTH * 1.3)
        {
            m_notes_current++;
            m_state = State::START_NOTE;
        }
        else
        {
            m_micros += delta;
        }
    }
    break;
    }
}