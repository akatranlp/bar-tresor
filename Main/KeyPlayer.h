#include "./pitches.h"
#include "./SoundPlayer.h"
#include <Keypad.h>

#pragma once

int keyToNoteMap(char key)
{
    switch (key)
    {
    case '1':
        return NOTE_C4;
    case '2':
        return NOTE_D4;
    case '3':
        return NOTE_E4;
    case '4':
        return NOTE_F4;
    case '5':
        return NOTE_G4;
    case '6':
        return NOTE_A4;
    case '7':
        return NOTE_B4;
    case '8':
        return NOTE_C5;
    case '9':
        return NOTE_D5;
    case '*':
        return NOTE_E5;
    case '0':
        return NOTE_F5;
    case '#':
        return NOTE_G5;
    default:
        return 0;
    }
}

class KeyPlayer
{
public:
    KeyPlayer(SoundPlayer *soundPlayer, const char keys[4][3], byte row_pins[4], byte col_pins[3], int rows, int cols)
    {
        m_keypad = new Keypad(makeKeymap(keys), row_pins, col_pins, rows, cols);
        m_sound_player = soundPlayer;
    }

    bool isNewKey()
    {
        return m_is_new_key;
    }

    char getKey()
    {
        m_is_new_key = false;
        return m_last_key;
    }

    void update()
    {
        char key = m_keypad->getKey();
        if (key != NO_KEY)
        {
            int keyNotes[] = {keyToNoteMap(key), 0};
            int keyNoteDurations[] = {4, 1};
            m_sound_player->playSound(keyNotes, keyNoteDurations, 2);
            m_last_key = key;
            m_is_new_key = true;
        }
    }

private:
    Keypad *m_keypad;
    SoundPlayer *m_sound_player;
    char m_last_key;
    bool m_is_new_key = false;
};