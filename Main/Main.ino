#include <SPI.h>
#include <Wire.h>

// extern libraries
#include <Stepper.h>
#include <rfid1.h>
#include <pitches.h>

// intern libraries
#include <DistancePlayer.h>
#include <DisplayPlayer.h>
#include <RotatePlayer.h>
#include <SoundPlayer.h>
#include <TiltPlayer.h>
#include <KeyPlayer.h>
#include <TouchPlayer.h>

#include <Game.h>
#include <GameEasy.h>
#include <GameNormal.h>
#include <GameHard.h>

// ------------------------------------
// ----------DELTA TIME----------------
unsigned long last_time = 0;
unsigned long m_micros = 0;

// ------------------------------------
// ----------RFID----------------------
#define RFID_IRQ_PIN 27
#define RFID_SCK_PIN 22
#define RFID_MOSI_PIN 25
#define RFID_MISO_PIN 24
#define RFID_SDA_PIN 23
#define RFID_RST_PIN 5
RFID1 rfid;

// UIDs of our RFID cards
uchar management_UID[4] = {0x8C, 0x35, 0xD7, 0xCD};
uchar card_easy_UID[4] = {0x63, 0xB0, 0x43, 0xF6};
uchar card_normal_UID[4] = {0x03, 0x05, 0x17, 0xF6};
uchar card_hard_UID[4] = {0x23, 0x84, 0x1E, 0xF6};

#define INVALID_CARD_ID -1
#define MANAGMENT_CARD_ID 0
#define CARD_EASY_ID 1
#define CARD_NORMAL_ID 2
#define CARD_HARD_ID 3

// buffer for reading id from an rfid card
uchar rfid_serial_number[5];

// Texts to display on certain states
const char *card_text_rows[] = {
    "Bitte",
    "stecke eine",
    "Karte ein!",
};

static DisplayPlayer::Text card_text = DisplayPlayer::Text{
  rows : card_text_rows,
  size : 3,
  text_size : 2,
};

const char *management_text_rows[] = {
    "Der Tresor",
    "wurde durch",
    "einen",
    "Mitarbeiter",
    "geoeffnet.",
};

static DisplayPlayer::Text management_text = DisplayPlayer::Text{
  rows : management_text_rows,
  size : 5,
  text_size : 2,
};

// ------------------------------------
// ----------DISTANCE------------------
#define DISTANCE_ECHO_PIN 3
#define DISTANCE_TRIGGER_PIN 6
DistancePlayer distancePlayer(DISTANCE_ECHO_PIN, DISTANCE_TRIGGER_PIN);

// ------------------------------------
// ----------SOUND---------------------
#define PIEZO_PIN 11
SoundPlayer soundPlayer(PIEZO_PIN);

// Winning melody
const int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
const int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
const int melody_size = sizeof(melody) / sizeof(melody[0]);

// ------------------------------------
// ----------DISPLAY-------------------
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9
DisplayPlayer displayPlayer(TFT_CS, TFT_DC, TFT_RST);

// ------------------------------------
// ------------TOUCH-------------------
#define TOUCH_PIN_TOP 32
#define TOUCH_PIN_LEFT 4
#define TOUCH_PIN_RIGHT 7
#define TOUCH_PIN_BACK 30
TouchPlayer touchPlayer(TOUCH_PIN_TOP, TOUCH_PIN_BACK, TOUCH_PIN_LEFT, TOUCH_PIN_RIGHT);

// ------------------------------------
// -------------Rotate-----------------
#define ROTATE_PIN_LEFT A1
#define ROTATE_PIN_RIGHT A0
RotatePlayer rotatePlayer(ROTATE_PIN_LEFT, ROTATE_PIN_RIGHT);

// ------------------------------------
// ------------Stepper-----------------

// The stepper uses 512 steps to make a forth of a full rotation
#define SPR 512 // stepsPerRevolution: change this to fit the number of steps per revolution
#define RPM 15  // rolesPerMinute: Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 34 through 40:
Stepper stepper(SPR, 34, 38, 36, 40);

//--------------------------------------
//-----------Keypad---------------------
#define ROWS 4
#define COLS 3
const char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'},
};
byte rowPins[ROWS] = {37, 39, 41, 43}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {45, 49, 47};     // connect to the column pinouts of the keypad

KeyPlayer keyPlayer(&soundPlayer, keys, rowPins, colPins, ROWS, COLS);

// ------------------------------------
// --------------MPU6050---------------
#define MPU_ADDR 0x68
TiltPlayer tiltPlayer(MPU_ADDR);

// ------------------------------------
// --------------GAME------------------
// All puzzles are implemented in the Game class
Game *game;

// Global state machine
enum class State
{
  WAIT_FOR_RFID,
  START_GAME,
  GAME,
  GAME_WON,
  MANAGEMENT_OPEN,
  FINISH,
};
State state = State::WAIT_FOR_RFID;

// ------------------------------------
// --------------SETUP-----------------
void setup()
{
  Serial.begin(115200);

  // Generate a random Seed for some of the puzzles
  // These pins are not connected to anything and therefore generate a pseudo random value
  randomSeed(analogRead(A2) * analogRead(A3) * analogRead(A4) * analogRead(A5));

  // initialize I2C
  Wire.begin();

  // initialize all our own libraries
  touchPlayer.begin();
  tiltPlayer.begin();
  displayPlayer.begin();

  // initialize the RFID reader
  rfid.begin(RFID_IRQ_PIN, RFID_SCK_PIN, RFID_MOSI_PIN, RFID_MISO_PIN, RFID_SDA_PIN, RFID_RST_PIN);
  delay(100);
  rfid.init();

  Serial.println("Starting...");

  // TODO: rename this
  displayPlayer.drawImmediate(draw_bootup_screen);

  // Close the vault
  stepper.setSpeed(RPM);
  closeVault();

  // Play our iconic startup beep sound
  int beep_sound[] = {NOTE_C7};
  int beep_duration[] = {8};
  soundPlayer.playSound(beep_sound, beep_duration, 1);
  displayPlayer.draw_text(&card_text);

  last_time = micros();
}

void closeVault()
{
  stepper.step(-SPR);
}

void openVault()
{
  stepper.step(SPR);
}

// This functions draws 8 rings in different colors on the display
void draw_bootup_screen(Adafruit_ST7735 *tft)
{
  int second = 1000;
  int rings = 8;
  int delayValue = second / rings;

  tft->fillScreen(ST7735_BLACK);
  tft->drawCircle(80, 64, 60, ST7735_WHITE);
  tft->drawCircle(80, 64, 59, ST7735_WHITE);
  delay(delayValue);
  tft->drawCircle(80, 64, 50, ST7735_MAGENTA);
  tft->drawCircle(80, 64, 49, ST7735_MAGENTA);
  delay(delayValue);
  tft->drawCircle(80, 64, 40, ST7735_BLUE);
  tft->drawCircle(80, 64, 39, ST7735_BLUE);
  delay(delayValue);
  tft->drawCircle(80, 64, 30, ST7735_CYAN);
  tft->drawCircle(80, 64, 29, ST7735_CYAN);
  delay(delayValue);
  tft->drawCircle(80, 64, 20, ST7735_GREEN);
  tft->drawCircle(80, 64, 19, ST7735_GREEN);
  delay(delayValue);
  tft->drawCircle(80, 64, 10, ST7735_YELLOW);
  tft->drawCircle(80, 64, 9, ST7735_YELLOW);
  delay(delayValue);
  tft->drawCircle(80, 64, 5, ST7735_ORANGE);
  tft->drawCircle(80, 64, 4, ST7735_ORANGE);
  delay(delayValue);
  tft->drawCircle(80, 64, 3, ST7735_RED);
  tft->drawCircle(80, 64, 2, ST7735_RED);
  delay(delayValue);
}

// https://arduino.stackexchange.com/questions/18313/issue-sharing-miso-with-multiple-rc522-rfid-readers
// http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader#Module_Interface_SPI_Parameters
// Under this links we found the RFID1 library from the extern libraries section
// We now use Software SPI, because our display also uses SPI and we had a lot of problems in combination with the RFID reader
//
// This function reads the UID of an RFID card, compares it to the UIDs of our cards and returns the card id
int getCardId()
{
  // Searches an RFID card and returns basic information about the card and puts it into the buffer
  uchar status;
  uchar buffer[MAX_LEN];
  status = rfid.request(PICC_REQIDL, buffer);

  if (status != MI_OK)
  {
    return INVALID_CARD_ID;
  }

  // This returns the UID of the card and puts it into the buffer
  status = rfid.anticoll(buffer);

  if (status != MI_OK)
  {
    return INVALID_CARD_ID;
  }

  // Copy the UID from the buffer to our own buffer
  // TODO: check if this is really necessary
  memcpy(rfid_serial_number, buffer, 5);
  uchar *id = buffer;

  // Compare the UID of the card with our UIDs and return the card id
  if (id[0] == management_UID[0] && id[1] == management_UID[1] && id[2] == management_UID[2] && id[3] == management_UID[3])
  {
    return MANAGMENT_CARD_ID;
  }
  else if (id[0] == card_easy_UID[0] && id[1] == card_easy_UID[1] && id[2] == card_easy_UID[2] && id[3] == card_easy_UID[3])
  {
    return CARD_EASY_ID;
  }
  else if (id[0] == card_normal_UID[0] && id[1] == card_normal_UID[1] && id[2] == card_normal_UID[2] && id[3] == card_normal_UID[3])
  {
    return CARD_NORMAL_ID;
  }
  else if (id[0] == card_hard_UID[0] && id[1] == card_hard_UID[1] && id[2] == card_hard_UID[2] && id[3] == card_hard_UID[3])
  {
    return CARD_HARD_ID;
  }
  else
  {
    return INVALID_CARD_ID;
  }
}

// ------------------------------------
// --------------LOOP------------------
void loop()
{
  // Calculate the delta time
  unsigned long time = micros();
  unsigned long delta = time - last_time;
  last_time = time;

  // Update the state machines of all our libraries
  soundPlayer.update(delta);
  displayPlayer.update(delta);
  int distance = distancePlayer.update(delta);
  keyPlayer.update();

  // Update our global state machine
  switch (state)
  {
    // ------------------------------------
    // --------STATE: WAIT FOR RFID--------
    // ------------------------------------
  case State::WAIT_FOR_RFID:
  {
    int cardId = getCardId();
    if (cardId == INVALID_CARD_ID)
    {
      return;
    }

    // If the management card is inserted, open the vault and go to the management state
    if (cardId == MANAGMENT_CARD_ID)
    {
      state = State::MANAGEMENT_OPEN;
      return;
    }

    // If a game card is inserted, start the game
    if (cardId == CARD_EASY_ID)
    {
      game = new GameEasy(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
    }
    else if (cardId == CARD_NORMAL_ID)
    {
      game = new GameNormal(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
    }
    else if (cardId == CARD_HARD_ID)
    {
      game = new GameHard(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
    }
    state = State::START_GAME;
  }
  break;
    // ------------------------------------
    // ---------STATE: START GAME----------
    // ------------------------------------
  case State::START_GAME:
  {
    // Play the start sound and start the game
    soundPlayer.playSound(melody, noteDurations, melody_size);
    state = State::GAME;
  }
  break;
    // ------------------------------------
    // ------------STATE: GAME-------------
    // ------------------------------------
  case State::GAME:
  {
    // Check every second for the management card
    // If it is inserted, open the vault and go to the management state
    if (m_micros > 1000000)
    {
      m_micros = 0;
      if (getCardId() == MANAGMENT_CARD_ID)
      {
        state = State::MANAGEMENT_OPEN;
        return;
      }
    }
    // Increment passed microseconds
    else
    {
      m_micros += delta;
    }

    // Update the game state machine and check if the game is won
    if (game->update(delta, distance))
    {
      state = State::GAME_WON;
    }
  }
  break;
    // ------------------------------------
    // ----------STATE: GAME WON-----------
    // ------------------------------------
  case State::GAME_WON:
  {
    // Play the winning melody and open the vault
    openVault();
    soundPlayer.playSound(melody, noteDurations, melody_size);
    state = State::FINISH;
  }
  break;
    // ------------------------------------
    // -------STATE: MANAGEMENT OPEN-------
    // ------------------------------------
  case State::MANAGEMENT_OPEN:
  {
    // Open the vault and display the management text
    openVault();
    displayPlayer.draw_text(&management_text);
    state = State::FINISH;
  }
  break;
    // ------------------------------------
    // -----------STATE: FINISH------------
    // ------------------------------------
  case State::FINISH:
  {
    // Infinite loop so the owner needs to restart the device
    Serial.println("FINISH");
  }
  break;
  }
}
