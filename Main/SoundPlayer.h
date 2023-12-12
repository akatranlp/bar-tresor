#include "./pitches.h"

#define IMPLEMENTATION LIFO
#define NOTE_CAPACITY 128

#define STATE_WAITING_FOR_NOTE 0
#define STATE_START_NOTE 1
#define STATE_PLAYING_NOTE 2
#define STATE_STOP_NOTE 3
#define STATE_PAUSE_NOTE 4

struct Note
{
    long note;
    long duraction;
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
        m_state = STATE_START_NOTE;
        noTone(m_sound_pin);
        for (int i = 0; i < arr_len; ++i)
        {
            int noteDuration = 800 / noteDurations[i];
            Note note = Note{melody[i], noteDuration};
            m_notes_size++;
            m_notes[m_notes_size - 1] = note;
        }
    }

    void addTone(int note, int duration)
    {
        Note _note = Note{note, duration};
        m_notes[m_notes_size - 1] = _note;
        m_notes_size++;
    }

    void update(unsigned long delta)
    {
        switch (m_state)
        {
        case STATE_WAITING_FOR_NOTE:
            break;
        case STATE_START_NOTE:
            if (m_notes_current < m_notes_size)
            {
                Note *note = &m_notes[m_notes_current];
                tone(m_sound_pin, note->note, note->duraction);
                m_state = STATE_PLAYING_NOTE;
                m_micros = 0;
            }
            else
            {
                m_state = STATE_WAITING_FOR_NOTE;
            }
            break;
        case STATE_PLAYING_NOTE:
            if (m_notes_current < m_notes_size)
            {
                Note *note = &m_notes[m_notes_current];
                if (m_micros >= note->duraction * 1000)
                {
                    m_state = STATE_PAUSE_NOTE;
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
                m_state = STATE_WAITING_FOR_NOTE;
            }
            break;
        case STATE_PAUSE_NOTE:
            if (m_notes_current < m_notes_size)
            {
                Note *note = &m_notes[m_notes_current];
                if (m_micros >= note->duraction * 1000 * 1.3)
                {
                    m_state = STATE_STOP_NOTE;
                }
                else
                {
                    m_micros += delta;
                }
            }
            else
            {
                m_state = STATE_WAITING_FOR_NOTE;
            }
            break;
        case STATE_STOP_NOTE:
            m_notes_current++;
            m_state = STATE_START_NOTE;
            break;
        }
    }

private:
    int m_state = STATE_WAITING_FOR_NOTE;
    int m_notes_current = 0;
    int m_notes_size = 0;

    int m_sound_pin;
    unsigned long m_micros = 0;
};
