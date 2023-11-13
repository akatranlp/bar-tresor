#include <Arduino.h>
#include "./pitches.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <arduino-timer.h>
#include <Keypad.h>
#include <Stepper.h>

//--------------------------------------
//-----------Motor----------------------
const int stepsPerRevolution = 512;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm

// initialize the stepper library on pins 34 through 40:
Stepper stepper(stepsPerRevolution, 34,  38,36, 40);

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

const int falseSound[] = {};
const int falseSoundDuration[] = {4};
const int falseSoundSize = sizeof(falseSound) / sizeof(falseSound[0]);

auto note_timer = Timer<0x20, millis, PiezoTone>();

//--------------------------------------
//-----------Game State-----------------
int state = 0;
int counter = 0;

//--------------------------------------
//----------Rotate Game-----------------
const char rotateKeys[] = {'1', '3', '3', '7'};
const int rotateMelody[] = {NOTE_C4, NOTE_E4, NOTE_E4, NOTE_B4, 0};
const int rotateNoteDurations[] = {4, 4, 4, 4, 1};
const int rotateMelodySize = sizeof(rotateMelody) / sizeof(rotateMelody[0]);

char volatile currentKeys[] = {0, 0, 0, 0};
int volatile keyIndex = 0;

bool volatile win = false;

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

  int value = analogRead(ROTATE_PIN);
  int currentSegment = value / ROTATE_STEP;

  do
  {
    rotateSegment = random(0, 8);
  } while (rotateSegment == currentSegment);

  tft.initR(INITR_BLACKTAB); // initialize a ST7735S chip, black tab
  void drawInit();

  delay(200);
  int beep_sound[] = {NOTE_C7};
  int beep_duration[] = {8};

  playSound(beep_sound, beep_duration, 1);

  tft.setRotation(1);

  stepper.setSpeed(rolePerMinute);
  closeVault();

  delay(200);

  timer.every(50, keyListener);
  // timer.every(200, mainLoop);
  timer.every(200, rotateLoop);

}

void drawInit() {
  tft.fillScreen(ST7735_BLACK);
  drawTextCentered("Hello World", ST7735_WHITE,12,3);
  drawTextCentered("You win!", ST7735_WHITE, 9,2);
  //drawText("Hello World", ST7735_WHITE);
}

//--------------------------------------
//-----------Loop-----------------------

bool mainLoop(void *)
{
  // getDistance();
  // getRotate();
  // getTouch();

  Serial.println("mainLoop");
  counter++;
  if (counter == 10)
  {
    timer.every(200, distanceLoop);
    return false;
  }

  // Serial.println(note_timer.size());

  return true;
}

bool distanceLoop(void *)
{
  Serial.println("distanceLoop");
  getDistance();
  return true;
}

bool touchLoop(void *)
{
  getTouch();
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

  if (key != NO_KEY)
  {
    Serial.println(key);
    note_timer.cancel();
    noTone(PIEZO_PIN);
    // playSoundTimer(melody2, noteDurations2, melody2_size);

    int notes[] = {keyToNoteMap(key), 0};
    int noteDurations[] = {4, 1};

    playSoundTimer(notes, noteDurations, 2);

    currentKeys[keyIndex++] = key;
    if (keyIndex == 4)
    {
      keyIndex = 0;
      if (checkKeys())
      {
        win = true;
        tft.fillScreen(ST7735_BLACK);
        drawTextCentered("You win!", ST7735_WHITE, 9,2);
        openVault();
        return false;
      }
    }
  }

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

void getTouch()
{

  if (digitalRead(TOUCH_PIN_NORTH) == HIGH)
  {
    Serial.println("NORTH");
  }
  if (digitalRead(TOUCH_PIN_EAST) == HIGH)
  {
    Serial.println("EAST");
    if (note_timer.empty())
    {
      playSoundTimer(melody, noteDurations, melody_size);
    }
  }
  if (digitalRead(TOUCH_PIN_SOUTH) == HIGH)
  {
    Serial.println("SOUTH");
  }
  if (digitalRead(TOUCH_PIN_WEST) == HIGH)
  {
    Serial.println("WEST");
    if (note_timer.empty())
    {
      playSoundTimer(melody, noteDurations, melody_size);
    }
  }
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
    // Serial.println("Kein Messwert");
  }
  else //  Ansonsten…
  {
    Serial.print(distance);
    Serial.println(" cm");
  }
}

void playSound(const int melody[], const int noteDurations[], const int arr_len)
{
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


/*
void drawText(char *text, uint16_t color)
{
  tft.setCursor(0, 0);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.setTextWrap(true);
  tft.print(text);
}
*/

void drawTextCentered(char *text, uint16_t color, int length, int textSize) {
  int x = (tft.width() - (length * textSize * 6)) / 2;
  int y = (tft.height() - (textSize * 8)) / 2;
  tft.setCursor(x, y);
  Serial.print(x);
  Serial.print(y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
  
}




void closeVault() {
  stepper.step(-stepsPerRevolution);
  delay(2000);
}


void openVault() {
  stepper.step(stepsPerRevolution);
  delay(2000);
}
