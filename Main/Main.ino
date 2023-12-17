#include <GY521_registers.h>
#include <GY521.h>

#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Stepper.h>
#include <Keypad.h>

#include "./pitches.h"
#include "./SoundPlayer.h"
#include "./DistancePlayer.h"
#include "./DisplayPlayer.h"
#include "./TouchPlayer.h"
#include "./RotatePlayer.h"
#include "./KeyPlayer.h"
#include "./TiltPlayer.h"

#include "./Game.h"
#include "./Game1.h"
#include "./Game2.h"
#include "./Game3.h"

// ------------------------------------
// ----------DELTA TIME----------------
unsigned long last_time = 0;

// ------------------------------------
// ----------RFID----------------------
#define RFID_SS_PIN 53
#define RFID_RST_PIN 5
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

byte managementUID[10] = {0x8C, 0x35, 0xD7, 0xCD, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte card1UID[10] = {0x63, 0xB0, 0x43, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte card2UID[10] = {0x03, 0x05, 0x17, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte card3UID[10] = {0x23, 0x84, 0x1E, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// ------------------------------------
// ----------DISTANCE------------------
#define DISTANCE_ECHO_PIN 3
#define DISTANCE_TRIGGER_PIN 6
DistancePlayer distancePlayer(DISTANCE_ECHO_PIN, DISTANCE_TRIGGER_PIN);

// ------------------------------------
// ----------SOUND---------------------
#define PIEZO_PIN 11
SoundPlayer soundPlayer(PIEZO_PIN);

// ------------------------------------
// ----------DISPLAY-------------------
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9
DisplayPlayer displayPlayer(TFT_CS, TFT_DC, TFT_RST);

// ------------------------------------
// ------------TOUCH-------------------
#define TOUCH_PIN_TOP 31
#define TOUCH_PIN_LEFT 4
#define TOUCH_PIN_RIGHT 7
#define TOUCH_PIN_BACK 30
TouchPlayer touchPlayer(TOUCH_PIN_TOP, TOUCH_PIN_BACK, TOUCH_PIN_LEFT, TOUCH_PIN_RIGHT);

// ------------------------------------
// -------------Rotate-----------------
#define ROTATE_PIN_LEFT A1
#define ROTATE_PIN_RIGHT A0
#define ROTATE_STEP 128
RotatePlayer rotatePlayer(ROTATE_PIN_LEFT, ROTATE_PIN_RIGHT);

// ------------------------------------
// ------------Stepper-----------------
#define SPR 512 // stepsPerRevolution: change this to fit the number of steps per revolution
#define RPM 15  // rolesPerMinute: Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 34 through 40:
Stepper stepper(SPR, 34, 38, 36, 40);

// ------------------------------------
// --------------Sound-----------------

const int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
const int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
const int melody_size = sizeof(melody) / sizeof(melody[0]);

//--------------------------------------
//-----------Keypad---------------------

#define ROWS 4 // four rows
#define COLS 3 // three columns
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
// --------------Gyro------------------

#define GYRO_SDA 20
GY521 gyro(GYRO_SDA);

// ------------------------------------
// --------------MPU6050---------------
#define MPU_ADDR 0x68
TiltPlayer tiltPlayer(MPU_ADDR);

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

void setup()
{
  randomSeed(analogRead(A2));
  Wire.begin();
  tiltPlayer.begin();

  Serial.begin(115200);

  pinMode(TOUCH_PIN_RIGHT, INPUT);
  pinMode(TOUCH_PIN_LEFT, INPUT);
  pinMode(TOUCH_PIN_TOP, INPUT);
  pinMode(TOUCH_PIN_BACK, INPUT);

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Starting...");

  displayPlayer.begin();
  displayPlayer.drawImmediate(drawHelloWorld);

  // stepper.setSpeed(RPM);
  // closeVault();

  int beep_sound[] = {NOTE_C7};
  int beep_duration[] = {8};
  soundPlayer.playSound(beep_sound, beep_duration, 1);
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

void drawHelloWorld(Adafruit_ST7735 *tft)
{
  int initBeforeBeepTime = 1000;
  int rings = 8;

  tft->fillScreen(ST7735_BLACK);
  tft->drawCircle(80, 64, 60, ST7735_WHITE);
  tft->drawCircle(80, 64, 59, ST7735_WHITE);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 50, ST7735_MAGENTA);
  tft->drawCircle(80, 64, 49, ST7735_MAGENTA);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 40, ST7735_BLUE);
  tft->drawCircle(80, 64, 39, ST7735_BLUE);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 30, ST7735_CYAN);
  tft->drawCircle(80, 64, 29, ST7735_CYAN);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 20, ST7735_GREEN);
  tft->drawCircle(80, 64, 19, ST7735_GREEN);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 10, ST7735_YELLOW);
  tft->drawCircle(80, 64, 9, ST7735_YELLOW);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 5, ST7735_ORANGE);
  tft->drawCircle(80, 64, 4, ST7735_ORANGE);
  delay(initBeforeBeepTime / rings);
  tft->drawCircle(80, 64, 3, ST7735_RED);
  tft->drawCircle(80, 64, 2, ST7735_RED);
  delay(initBeforeBeepTime / rings);
}

int getCardId()
{
  if (!rfid.PICC_IsNewCardPresent())
  {
    return -1;
  }

  if (!rfid.PICC_ReadCardSerial())
  {
    return -1;
  }

  int cardId = 0;
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    byte cardByte = rfid.uid.uidByte[i];
    byte otherCardByte = managementUID[i];
    if (cardByte != otherCardByte)
    {
      cardId = -1;
      break;
    }
  }
  if (cardId == 0)
  {
    return 0;
  }
  cardId = 1;
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    byte cardByte = rfid.uid.uidByte[i];
    byte otherCardByte = card1UID[i];
    if (cardByte != otherCardByte)
    {
      cardId = -1;
      break;
    }
  }
  if (cardId == 1)
  {
    return 1;
  }
  cardId = 2;
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    byte cardByte = rfid.uid.uidByte[i];
    byte otherCardByte = card2UID[i];
    if (cardByte != otherCardByte)
    {
      cardId = -1;
      break;
    }
  }
  if (cardId == 2)
  {
    return 2;
  }
  cardId = 3;
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    byte cardByte = rfid.uid.uidByte[i];
    byte otherCardByte = card3UID[i];
    if (cardByte != otherCardByte)
    {
      cardId = -1;
      break;
    }
  }
  return cardId;
}

enum class State
{
  STARTUP,
  WAIT_FOR_RFID,
  START_GAME,
  GAME,
  GAME_WON,
  MANAGEMENT_OPEN,
  FINISH,
};

volatile State state = State::WAIT_FOR_RFID;

int gameId = -1;
Game *game;

void drawGame(Adafruit_ST7735 *tft)
{
  tft->drawLine(0, 0, 100, 100, ST7735_WHITE);
}

void drawManagementFinish(Adafruit_ST7735 *tft)
{
  tft->drawLine(0, 0, 100, 100, ST7735_WHITE);
}

void drawWon(Adafruit_ST7735 *tft)
{
  tft->drawLine(0, 0, 100, 100, ST7735_WHITE);
}

void loop()
{
  unsigned long time = micros();
  unsigned long delta = time - last_time;
  last_time = time;
  soundPlayer.update(delta);
  displayPlayer.update(delta);
  int distance = distancePlayer.update(delta);
  keyPlayer.update();

  switch (state)
  {
  case State::STARTUP:
    break;
  case State::WAIT_FOR_RFID:
  {
    // Serial.println("WAIT_FOR_RFID");
    int cardId = getCardId();
    if (cardId != -1)
    {
      Serial.print("RFID FOUND: ");
      Serial.println(cardId);
      if (cardId == 0)
      {
        state = State::MANAGEMENT_OPEN;
        Serial.println("MANAGEMENT OPEN");
      }
      else
      {
        if (cardId == 1)
        {
          game = new Game1(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }
        else if (cardId == 2)
        {
          game = new Game2(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }
        else if (cardId == 3)
        {
          game = new Game3(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }

        Serial.println("Start Game");
        gameId = cardId;
        state = State::START_GAME;
      }
    }
  }
  break;
  case State::START_GAME:
  {
    Serial.println("Start Game");
    state = State::GAME;
    soundPlayer.playSound(melody, noteDurations, melody_size);
    displayPlayer.drawImmediate(drawGame);
  }
  break;
  case State::GAME:
  {
    int cardId = getCardId();
    if (cardId == 0)
    {
      state = State::MANAGEMENT_OPEN;
    }
    else if (game->update(delta, distance))
    {
      state = State::GAME_WON;
    }
  }
  break;
  case State::GAME_WON:
  {
    // openVault();
    soundPlayer.playSound(melody, noteDurations, melody_size);
    displayPlayer.drawImmediate(drawWon);
    state = State::FINISH;
  }
  break;
  case State::MANAGEMENT_OPEN:
  {
    Serial.println("MANAGEMENT_OPEN");
    // openVault();
    soundPlayer.playSound(melody, noteDurations, melody_size);
    displayPlayer.drawImmediate(drawManagementFinish);
    state = State::FINISH;
  }
  break;
  case State::FINISH:
  {
    Serial.println("FINISH");
  }
  break;
  }

  /* Serial.print(time);
  Serial.print(" - ");
  Serial.print(last_time);
  Serial.print(" = ");
  Serial.println(delta); */
}
