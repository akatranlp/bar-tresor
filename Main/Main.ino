#include <Arduino.h>
#include "./pitches.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <arduino-timer.h>
#include <Keypad.h>
#include <Stepper.h>

//--------------------------------------
//-----------Motor----------------------
#define SPR 512 // stepsPerRevolution: change this to fit the number of steps per revolution
#define RPM 15  // rolesPerMinute: Adjustable range of 28BYJ-48 stepper is 0~17 rpm
// initialize the stepper library on pins 34 through 40:
Stepper stepper(SPR, 34, 38, 36, 40);

//--------------------------------------
//-----------Timer----------------------
auto timer = timer_create_default();

//--------------------------------------
//-----------Touch----------------------

#define TOUCH_PIN_NORTH 24
#define TOUCH_PIN_EAST 26
#define TOUCH_PIN_SOUTH 25
#define TOUCH_PIN_WEST 27

//--------------------------------------
//-----------Rotate---------------------

#define ROTATE_PIN A0
#define ROTATE_STEP 128

int rotateSegment;
int lastSegment;

//--------------------------------------
//--------Distance-Measure--------------
#define DISTANCE_ECHO_PIN 6
#define DISTANCE_TRIGGER_PIN 7

int timeCounter;
int volatile lastHeight;

int targetHeight;
int targetTime;

//--------------------------------------
//-----------TFT-Display----------------
#define TFT_CS 10
#define TFT_RST 8
#define TFT_DC 9

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

auto tft_timer = timer_create_default();

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

byte rowPins[ROWS] = {35, 37, 39, 41}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {43, 45, 47};     // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//--------------------------------------
//----------------Piezo-----------------
#define PIEZO_PIN 22

const int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4};
const int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};
const int melody_size = sizeof(melody) / sizeof(melody[0]);
struct PiezoTone
{
  int pin;
  int note;
  int duration;
  int index;
};

const int falseSound[] = {NOTE_C3, NOTE_C2};
const int falseSoundDuration[] = {4, 4};
const int falseSoundSize = sizeof(falseSound) / sizeof(falseSound[0]);

auto note_timer = Timer<0x20, millis, PiezoTone>();

//--------------------------------------
//-----------Game State-----------------
int state = 0;
int counter = 0;

char *previousCenterText = "";

//--------------------------------------
//----------Rotate Game-----------------
const char rotateKeys[] = {'1', '3', '3', '7'};
const int rotateMelody[] = {NOTE_C4, NOTE_E4, NOTE_E4, NOTE_B4, 0};
const int rotateNoteDurations[] = {4, 4, 4, 4, 1};
const int rotateMelodySize = sizeof(rotateMelody) / sizeof(rotateMelody[0]);

char volatile currentKeys[] = {0, 0, 0, 0};
int volatile keyIndex = 0;

bool volatile win = false;

bool volatile isRotateGame = false;

//--------------------------------------
//-----------Functions------------------
//--------------------------------------

//--------------------------------------
//-----------Setup----------------------
void setup()
{
  Serial.begin(115200);
  pinMode(TOUCH_PIN_NORTH, INPUT);
  pinMode(TOUCH_PIN_EAST, INPUT);
  pinMode(TOUCH_PIN_SOUTH, INPUT);
  pinMode(TOUCH_PIN_WEST, INPUT);

  pinMode(PIEZO_PIN, OUTPUT);

  pinMode(DISTANCE_TRIGGER_PIN, OUTPUT);
  pinMode(DISTANCE_ECHO_PIN, INPUT);

  randomSeed(analogRead(1));

  targetHeight = random(3, 20);
  targetTime = random(3, 10);

  tft.initR(INITR_BLACKTAB); // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);
  drawInitBeforeBeep();

  int beep_sound[] = {NOTE_C7};
  int beep_duration[] = {8};

  stepper.setSpeed(RPM);
  closeVault();

  playSound(beep_sound, beep_duration, 1);

  drawInitAfterBeep();

  timer.every(50, keyListener);
  timer.every(200, mainLoop);
  // timer.every(200, rotateLoop);
}

void drawInitBeforeBeep()
{
  int initBeforeBeepTime = 1000;
  int rings = 8;
  // Draws a circular loading screen going in circles
  tft.fillScreen(ST7735_BLACK);
  tft.drawCircle(80, 64, 60, ST7735_WHITE);
  tft.drawCircle(80, 64, 59, ST7735_WHITE);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 50, ST7735_MAGENTA);
  tft.drawCircle(80, 64, 49, ST7735_MAGENTA);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 40, ST7735_BLUE);
  tft.drawCircle(80, 64, 39, ST7735_BLUE);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 30, ST7735_CYAN);
  tft.drawCircle(80, 64, 29, ST7735_CYAN);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 20, ST7735_GREEN);
  tft.drawCircle(80, 64, 19, ST7735_GREEN);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 10, ST7735_YELLOW);
  tft.drawCircle(80, 64, 9, ST7735_YELLOW);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 5, ST7735_ORANGE);
  tft.drawCircle(80, 64, 4, ST7735_ORANGE);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 3, ST7735_RED);
  tft.drawCircle(80, 64, 2, ST7735_RED);
  delay(initBeforeBeepTime / rings);
}

void drawInitAfterBeep()
{
  int initBeforeBeepTime = 1000;
  int rings = 8;
  tft.drawCircle(80, 64, 60, ST7735_BLACK);
  tft.drawCircle(80, 64, 59, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 50, ST7735_BLACK);
  tft.drawCircle(80, 64, 49, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 40, ST7735_BLACK);
  tft.drawCircle(80, 64, 39, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 30, ST7735_BLACK);
  tft.drawCircle(80, 64, 29, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 20, ST7735_BLACK);
  tft.drawCircle(80, 64, 19, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 10, ST7735_BLACK);
  tft.drawCircle(80, 64, 9, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 5, ST7735_BLACK);
  tft.drawCircle(80, 64, 4, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.drawCircle(80, 64, 3, ST7735_BLACK);
  tft.drawCircle(80, 64, 2, ST7735_BLACK);
  delay(initBeforeBeepTime / rings);
  tft.fillScreen(ST7735_BLACK);
  drawTextCentered("Vault locked", ST7735_WHITE, 2);
}

//--------------------------------------
//-----------Loop-----------------------

bool mainLoop(void *)
{

  Serial.println("mainLoop");
  if (counter == 0)
  {
  }
  counter++;

  if (counter == 10)
  {
    timer.every(200, distanceLoop);
    tft_timer.every(2000, tftDistanceLoop);
    return false;
  }

  // Serial.println(note_timer.size());

  return true;
}

bool distanceLoop(void *)
{
  int value = getDistance();

  Serial.println(value);

  if (value >= targetHeight - 1 && value <= targetHeight + 1)
  {
    timeCounter++;
    if (timeCounter == targetTime)
    {
      tft_timer.cancel();
      tft_timer.every(2000, tftTouchLoop);
      timer.every(200, touchLoop);
      playSoundTimer(melody, noteDurations, melody_size);
      return false;
    }
  }
  else
  {
    timeCounter = 0;
  }

  lastHeight = value;

  return true;
}

bool tftDistanceLoop(void *)
{

  char *text = "000 cm";
  if (lastHeight < 10)
  {
    text[0] = '0';
    text[1] = '0';
    text[2] = '0' + lastHeight;
  }
  else if (lastHeight < 100)
  {
    text[0] = '0';
    text[1] = '0' + lastHeight / 10;
    text[2] = '0' + lastHeight % 10;
  }
  else
  {
    text[0] = '0' + lastHeight / 100;
    text[1] = '0' + (lastHeight % 100) / 10;
    text[2] = '0' + lastHeight % 10;
  }
  tft.fillScreen(ST7735_BLACK);
  drawTextCentered(text, ST7735_WHITE, 2);

  char *text2 = "000 cm";
  if (targetHeight < 10)
  {
    text2[0] = '0';
    text2[1] = '0';
    text2[2] = '0' + targetHeight;
  }
  else if (targetHeight < 100)
  {
    text2[0] = '0';
    text2[1] = '0' + targetHeight / 10;
    text2[2] = '0' + targetHeight % 10;
  }
  else
  {
    text2[0] = '0' + targetHeight / 100;
    text2[1] = '0' + (targetHeight % 100) / 10;
    text2[2] = '0' + targetHeight % 10;
  }
  drawText(text2, ST7735_WHITE);
}

bool touchLoop(void *)
{
  bool allTouched = digitalRead(TOUCH_PIN_NORTH) == HIGH && digitalRead(TOUCH_PIN_EAST) == HIGH && digitalRead(TOUCH_PIN_SOUTH) == HIGH && digitalRead(TOUCH_PIN_WEST) == HIGH;
  if (allTouched)
  {
    tft_timer.cancel();
    tft_timer.every(2000, tftRotateLoop);

    int value = analogRead(ROTATE_PIN);
    int currentSegment = value / ROTATE_STEP;
    do
    {
      rotateSegment = random(0, 8);
    } while (rotateSegment == currentSegment);
    lastSegment = currentSegment;

    timer.every(200, rotateLoop);
    playSoundTimer(melody, noteDurations, melody_size);
    Serial.println("allTouched");
    isRotateGame = true;
    return false;
  }
  return true;
}

bool tftTouchLoop(void *)
{
  tft.fillScreen(ST7735_BLACK);
  drawTextCentered("Embrace me", ST7735_WHITE, 2);
  return true;
}

bool rotateLoop(void *)
{
  if (!win)
  {
    int value = analogRead(ROTATE_PIN);
    int currentSegment = value / ROTATE_STEP;

    if (currentSegment == rotateSegment)
    {
      if (note_timer.empty())
      {
        playSoundTimer(rotateMelody, rotateNoteDurations, rotateMelodySize);
      }
    }
    else if (currentSegment != lastSegment)
    {
      note_timer.cancel();
      noTone(PIEZO_PIN);
    }

    lastSegment = currentSegment;
    return true;
  }
  else
  {
    return false;
  }
}

bool keyListener(void *)
{
  char key = keypad.getKey();

  Serial.println(key);

  if (key != NO_KEY)
  {
    Serial.println(key);
    note_timer.cancel();
    noTone(PIEZO_PIN);
    // playSoundTimer(melody2, noteDurations2, melody2_size);

    int keyNotes[] = {keyToNoteMap(key), 0};
    int keyNoteDurations[] = {4, 1};

    playSoundTimer(keyNotes, keyNoteDurations, 2);

    if (isRotateGame)
    {
      currentKeys[keyIndex++] = key;
      if (keyIndex == 4)
      {
        keyIndex = 0;
        if (checkKeys())
        {
          playSound(keyNotes, keyNoteDurations, 1);
          keyIndex = 5;
          win = true;
          tft_timer.cancel();
          tft.fillScreen(ST7735_BLACK);
          timer.cancel();
          drawTextCentered("You win!", ST7735_WHITE, 2);

          openVault();
          delay(1000);
          playSound(melody, noteDurations, melody_size);
          return false;
        }
        else
        {
          playSoundTimer(falseSound, falseSoundDuration, falseSoundSize);
        }
      }
    }
  }

  return true;
}

bool tftRotateLoop(void *)
{
  tft.fillScreen(ST7735_BLACK);
  drawTextCentered("Rotate me", ST7735_WHITE, 2);
  return true;
}

void loop()
{
  timer.tick();
  note_timer.tick();
  tft_timer.tick();
}

bool checkKeys()
{
  for (int i = 0; i < 4; i++)
  {
    if (currentKeys[i] != rotateKeys[i])
    {
      return false;
    }
  }
  return true;
}

int getDistance()
{
  digitalWrite(DISTANCE_TRIGGER_PIN, LOW);
  delay(5);
  digitalWrite(DISTANCE_TRIGGER_PIN, HIGH);
  delay(10);
  digitalWrite(DISTANCE_TRIGGER_PIN, LOW);
  int duration = pulseIn(DISTANCE_ECHO_PIN, HIGH);
  int distance = (duration / 2) * 0.03432;
  if (distance >= 500 || distance <= 0)
  {
    return lastHeight;
  }
  else //  Ansonsten…
  {
    return distance;
  }
}

void playSound(const int melody[], const int noteDurations[], const int arr_len)
{
  // Blocks
  int delayTime = 0;
  for (int thisNote = 0; thisNote < arr_len; thisNote++)
  {
    int noteDuration = 1000 / noteDurations[thisNote];

    tone(PIEZO_PIN, melody[thisNote], noteDuration);

    // delay(noteDuration);

    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    noTone(PIEZO_PIN);

    delayTime += pauseBetweenNotes;
  }
}

void playSoundTimer(const int melody[], const int noteDurations[], int arr_len)
{
  // Doesn't block
  int delayTime = 0;
  for (int thisNote = 0; thisNote < arr_len; thisNote++)
  {
    int noteDuration = 1000 / noteDurations[thisNote];

    auto note = PiezoTone{PIEZO_PIN, melody[thisNote], noteDuration, thisNote};

    note_timer.in(
        delayTime,
        [](PiezoTone note)
        {
          tone(note.pin, note.note, note.duration);
          return true;
        },
        note);

    // delayTime += noteDuration;
    int pauseBetweenNotes = noteDuration * 1.30;
    delayTime += pauseBetweenNotes;

    note_timer.in(
        delayTime,
        [](PiezoTone note)
        {
          noTone(note.pin);
          return true;
        },
        note);
  }
}

void drawText(char *text, uint16_t color)
{
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.print(text);
}

void drawTextCentered(char *text, uint16_t color, int textSize)
{
  tft.setTextColor(ST7735_BLACK);
  tft.print(previousCenterText);
  previousCenterText = text;
  int x = (tft.width() - (strlen(text) * textSize * 6)) / 2;
  int y = (tft.height() - (textSize * 8)) / 2;
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
}

void closeVault()
{
  stepper.step(SPR);
  delay(500);
}

void openVault()
{
  stepper.step(-SPR);
}
