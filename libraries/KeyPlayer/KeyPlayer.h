/* *******************************************************
 *  KeyPlayer.h
 *  This library is used to get the input of our matrix
 *  keypad and playing a sound when a key is pressed
 ********************************************************/
#include <pitches.h>
#include <SoundPlayer.h>
#include <Keypad.h>

#pragma once

int map_key_to_note(char key);

class KeyPlayer
{
public:
    KeyPlayer(SoundPlayer *soundPlayer, const char keys[4][3], byte row_pins[4], byte col_pins[3], int rows, int cols);

    // When a new key is pressed, this will return true
    bool isNewKey() { return m_is_new_key; }

    // Get the last key and reset the new key flag
    char getKey()
    {
        m_is_new_key = false;
        return m_last_key;
    }

    void update();

private:
    Keypad m_keypad;
    SoundPlayer *m_sound_player;

    char m_last_key;
    bool m_is_new_key = false;
};
