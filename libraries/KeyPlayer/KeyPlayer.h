#include <pitches.h>
#include <SoundPlayer.h>
#include <Keypad.h>

#pragma once

class KeyPlayer
{
public:
    KeyPlayer(SoundPlayer *soundPlayer, const char keys[4][3], byte row_pins[4], byte col_pins[3], int rows, int cols);

    bool isNewKey();
    char getKey();
    void update();

private:
    Keypad m_keypad;
    SoundPlayer *m_sound_player;
    char m_last_key;
    bool m_is_new_key = false;
};
