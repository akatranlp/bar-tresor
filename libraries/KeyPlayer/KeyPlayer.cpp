#include "KeyPlayer.h"

KeyPlayer::KeyPlayer(SoundPlayer *soundPlayer, const char keys[4][3], byte row_pins[4], byte col_pins[3], int rows, int cols)
    : m_keypad(Keypad(makeKeymap(keys), row_pins, col_pins, rows, cols)), m_sound_player(soundPlayer)
{
}

// Read the input of the keypad, save it and play the corresponding sound
void KeyPlayer::update()
{
    char key = m_keypad.getKey();
    if (key == NO_KEY)
    {
        return;
    }

    int keyNotes[] = {map_key_to_note(key), 0};
    int keyNoteDurations[] = {4, 1};
    m_sound_player->playSound(keyNotes, keyNoteDurations, 2);

    m_last_key = key;
    m_is_new_key = true;
}

// Map the key to a note
int map_key_to_note(char key)
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