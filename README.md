# Bar-Tresor

## Installation

Folgende Arduino Pakete müssen zuvor heruntergeladen werden:

- Adafruit_GFX.h [(Adafruit GFX Library by Adafruit Version 1.11.9)](https://github.com/adafruit/Adafruit-GFX-Library)
- Adafruit_ST7735.h [(Adafruit ST7735 and ST7789 Library by Adafruit Version 1.10.3)](https://github.com/adafruit/Adafruit-ST7735-Library)
- Keypad.h [(Keypad by Mark Stanley, Alexander Brevig Version 3.1.1)](https://www.arduino.cc/reference/en/libraries/keypad/)
- Stepper.h [(Stepper by Arduino Version 1.1.3)](https://www.arduino.cc/reference/en/libraries/stepper/)

Je nachdem welcher Editor verwendet wird sind andere Schritte notwendig.

## VSCode

Wenn VSCode als Code-Editor verwendet wird müssen folgende Dinge durchgeführt werden:

In der arduino.json muss folgende Zeile, je nach Betriebssystem angepasst werden:

- windows:

```json
{
  ...
  "prebuild": "powershell ./prebuild.ps1",
  ...
}
```

- linux:

```json
{
  ...
  "prebuild": "sh ./prebuild.sh",
  ...
}
```

Im prebuild step werden sowohl die externen Libraries als auch die interen Libraries in den Standard Arduino Libraries-Ordner kopiert und stehen somit für die gesamte Applikation bereit.

## Arduino IDE

Mit Arduino IDE wurde das ganze nicht getestet. Es sollten aber höchstens folgende Schritte notwendig sein:

- Kopieren der externLibraries in den Arduino Libraries-Ordner
- Kopieren der libraries in den Arduino Libraries-Ordner

Anschließend sollten auch hier alle Libraries zu Verfügung stehen. Hierbei ist zu beachten, das bei jeglicher Änderung der internen oder externen Libraries das jeweilige Package erneut in den Arduino-Ordner kopiert werden muss.

## Installation auf den Arduino

Sobald die vorherigen Schritte durchgeführt wurden, kann das Main.ino Skript einfach wie gewohnt auf den Arduino hochgeladen werden. Es sind keine weiteren Schritte notwendig.
