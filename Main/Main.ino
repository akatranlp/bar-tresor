#include <SPI.h>
#include <Wire.h>
#include <Stepper.h>
#include <Keypad.h>

// #include <MFRC522.h>
/* #include <MFRC522v2.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522Debug.h> */

#include <rfid1.h>

#include <DistancePlayer.h>
#include <DisplayPlayer.h>
#include <RotatePlayer.h>
#include <pitches.h>
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

// ------------------------------------
// ----------RFID----------------------
#define RFID_IRQ_PIN 27
#define RFID_SCK_PIN 22
#define RFID_MOSI_PIN 25
#define RFID_MISO_PIN 24
#define RFID_SDA_PIN 23
#define RFID_RST_PIN 5
RFID1 rfid;

uchar managementUID[4] = {0x8C, 0x35, 0xD7, 0xCD};
uchar card1UID[4] = {0x63, 0xB0, 0x43, 0xF6};
uchar card2UID[4] = {0x03, 0x05, 0x17, 0xF6};
uchar card3UID[4] = {0x23, 0x84, 0x1E, 0xF6};

uchar serNum[5];

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

const int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
const int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
const int melody_size = sizeof(melody) / sizeof(melody[0]);

// ------------------------------------
// ----------DISPLAY-------------------
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9
#define TFT_MOSI 28
#define TFT_SCLK 29
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
#define ROTATE_STEP 128
RotatePlayer rotatePlayer(ROTATE_PIN_LEFT, ROTATE_PIN_RIGHT);

// ------------------------------------
// ------------Stepper-----------------
#define SPR 512 // stepsPerRevolution: change this to fit the number of steps per revolution
#define RPM 15  // rolesPerMinute: Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 34 through 40:
Stepper stepper(SPR, 34, 38, 36, 40);

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
// --------------MPU6050---------------
#define MPU_ADDR 0x68
TiltPlayer tiltPlayer(MPU_ADDR);

// ------------------------------------
// --------------GAME------------------
Game *game;
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
State state = State::WAIT_FOR_RFID;
unsigned long m_micros = 0;

void setup()
{
  Serial.begin(115200);

  randomSeed(analogRead(A2) * analogRead(A3) * analogRead(A4) * analogRead(A5));

  touchPlayer.begin();

  Wire.begin();
  tiltPlayer.begin();

  rfid.begin(RFID_IRQ_PIN, RFID_SCK_PIN, RFID_MOSI_PIN, RFID_MISO_PIN, RFID_SDA_PIN, RFID_RST_PIN);
  delay(100);
  rfid.init();

  // TODO: already in constructor of touchplayer ! Check if begin function is needed
  // pinMode(TOUCH_PIN_RIGHT, INPUT);
  // pinMode(TOUCH_PIN_LEFT, INPUT);
  // pinMode(TOUCH_PIN_TOP, INPUT);
  // pinMode(TOUCH_PIN_BACK, INPUT);

  // pinMode(RFID_SS_PIN, OUTPUT);
  // pinMode(TFT_CS, OUTPUT);

  displayPlayer.begin();

  // SPI.begin();

  // rfid.PCD_Init(); // Init MFRC522 board.

  Serial.println("Starting...");
  displayPlayer.drawImmediate(drawHelloWorld);

  stepper.setSpeed(RPM);
  closeVault();

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
  uchar status;
  uchar str[MAX_LEN];
  status = rfid.request(PICC_REQIDL, str);

  if (status != MI_OK)
  {
    // Serial.println("No card detected");
    return -1;
  }

  // rfid.showCardType(str);
  status = rfid.anticoll(str);

  if (status != MI_OK)
  {
    // Serial.println("Something went wrong");
    return -1;
  }

  memcpy(serNum, str, 5);
  // rfid.showCardID(serNum);

  uchar *id = str;

  if (id[0] == managementUID[0] && id[1] == managementUID[1] && id[2] == managementUID[2] && id[3] == managementUID[3])
  {
    return 0;
  }
  else if (id[0] == card1UID[0] && id[1] == card1UID[1] && id[2] == card1UID[2] && id[3] == card1UID[3])
  {
    return 1;
  }
  else if (id[0] == card2UID[0] && id[1] == card2UID[1] && id[2] == card2UID[2] && id[3] == card2UID[3])
  {
    return 2;
  }
  else if (id[0] == card3UID[0] && id[1] == card3UID[1] && id[2] == card3UID[2] && id[3] == card3UID[3])
  {
    return 3;
  }
  else
  {
    return -1;
  }
}

void drawGame(Adafruit_ST7735 *tft)
{
  tft->fillScreen(ST7735_BLACK);
  tft->drawLine(0, 0, 100, 100, ST7735_WHITE);
}

void drawManagementFinish(Adafruit_ST7735 *tft)
{
  tft->fillScreen(ST7735_BLACK);
  tft->drawLine(0, 0, 100, 100, ST7735_CYAN);
}

void drawWon(Adafruit_ST7735 *tft)
{
  tft->fillScreen(ST7735_BLACK);
  tft->drawLine(0, 0, 100, 100, ST7735_ORANGE);
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
          game = new GameEasy(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }
        else if (cardId == 2)
        {
          game = new GameNormal(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }
        else if (cardId == 3)
        {
          game = new GameHard(&soundPlayer, &distancePlayer, &displayPlayer, &touchPlayer, &rotatePlayer, &keyPlayer, &tiltPlayer);
        }
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
  }
  break;
  case State::GAME:
  {
    if (m_micros > 1000 * 1000)
    {
      int cardId = getCardId();
      if (cardId == 0)
      {
        state = State::MANAGEMENT_OPEN;
        return;
      }
      m_micros = 0;
    }
    else
    {
      m_micros += delta;
    }

    if (game->update(delta, distance))
    {
      state = State::GAME_WON;
    }
  }
  break;
  case State::GAME_WON:
  {
    openVault();
    soundPlayer.playSound(melody, noteDurations, melody_size);
    state = State::FINISH;
  }
  break;
  case State::MANAGEMENT_OPEN:
  {
    Serial.println("MANAGEMENT_OPEN");
    openVault();
    // soundPlayer.playSound(melody, noteDurations, melody_size);
    displayPlayer.draw_text(&management_text);
    state = State::FINISH;
  }
  break;
  case State::FINISH:
  {
    Serial.println("FINISH");
  }
  break;
  }
}
