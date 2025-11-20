# Arduino Mega HC-SR04 + TM1637 4-digit Display

This project reads distance from an HC-SR04 ultrasonic sensor using an Arduino Mega (ATmega2560) and displays the measured distance on a TM1637 4‑digit display. It also prints debugging information to UART (9600 baud).

**Included files**
- `main.c` — firmware: HC‑SR04 measurement (Timer1 input capture), TM1637 driver and UART output.
- `diag_7seg.c` — diagnostic program for 7‑segment wiring (if you have a direct 4‑digit module or want to test wiring).
- `build.ps1` — PowerShell build script to compile `main.c` to `main.hex` using `avr-gcc`/`avr-objcopy`.
- `build_diag.ps1` — PowerShell build script to compile `diag_7seg.c` to `diag_7seg.hex`.
- `wokwi.toml` and `diagram.json` — Wokwi project files (diagram shows connections used in the repo).
- `wiring.svg` — simple wiring diagram showing connections used in Wokwi.

**What this firmware does**
- Triggers HC‑SR04 and measures echo pulse using Timer1 Input Capture for accurate timing.
- Uses median filtering (5 samples) and temperature compensation for better accuracy.
- Displays measured distance on a TM1637 4‑digit module and prints raw pulse width + distance to UART.

**Wiring (matches `diagram.json` / `wiring.svg`)**
- HC‑SR04 VCC -> `5V`
- HC‑SR04 GND -> `GND`
- HC‑SR04 TRIG -> `D2` (PORTE bit PE4)
- HC‑SR04 ECHO -> `D3` (PORTE bit PE5)
- TM1637 DIO -> `D6` (PORTH bit PH3)
- TM1637 CLK -> `D7` (PORTH bit PH4)
- TM1637 VCC -> `5V`
- TM1637 GND -> `GND`

If your hardware uses a bare 4‑digit common‑cathode display (not TM1637), see `diag_7seg.c` to identify wiring and then modify `main.c` or ask me to adapt the display driver.

**Schematic / Simulator diagram**

The repository includes `wiring.svg`. To embed a PNG schematic in this README, you can convert the SVG to PNG locally and the image will be shown below (if `wiring.png` exists).

![Simulator wiring](wiring.png)

To generate `wiring.png` from `wiring.svg`, run the `export_png.ps1` script in PowerShell (script will try ImageMagick `magick` or `inkscape`):

```powershell
cd /d "D:\Codes\git\NackademiN\Embedded_System\ArduinoMega_HC-HR04"
.\export_png.ps1
```

If you don't have ImageMagick or Inkscape installed, the script prints instructions for installing them.

**Build (Windows PowerShell)**
1. Install AVR toolchain (avr‑gcc, avr‑objcopy) and ensure tools are on your PATH.
2. From project folder run:
```powershell
cd /d "D:\Codes\git\NackademiN\Embedded_System\ArduinoMega_HC-HR04"
.\build.ps1
```
This produces `main.hex`.

To build the diagnostic program:
```powershell
.\build_diag.ps1
```
This produces `diag_7seg.hex`.

**Flash**
Use your preferred uploader. Example with `avrdude` (adjust COM port and programmer settings):
```powershell
avrdude -p m2560 -c wiring -P COM3 -b 115200 -U main.hex
```

**Run & Test**
- Open serial monitor at 9600 baud. You should see lines like:
  - `Raw us (median): 1234`
  - `Distance: 21 cm`
- The TM1637 display should show the measured distance.

**Diagnostics & Troubleshooting**
- If display is blank: verify DIO->D6 and CLK->D7 wiring and VCC/GND. TM1637 modules require pull-ups; driver uses internal pull-ups but external 10k pull-ups may improve reliability.
- If distance is noisy or off by a consistent amount: set `AMBIENT_TEMP_C` in `main.c` to your air temperature (°C) and/or set `CALIB_OFFSET_CM` to correct a constant offset.
- Run `diag_7seg.c` (build and flash `diag_7seg.hex`) to help map segments for a direct 4‑digit display.

**Wokwi**
- The repository contains `diagram.json` and `wokwi.toml`. In Wokwi the `firmware` points to `main.hex` so you can simulate the setup directly in Wokwi with the provided diagram.

**Next steps / Customization**
- Non-blocking display: if you want the display refresh to be interrupt-driven (so measurement and display don't block each other), I can implement a Timer ISR multiplex driver.
- Automatic temperature compensation: if you add a temperature sensor I can read it and adjust speed-of-sound automatically.

---
If you want me to modify pin mappings, add an avrdude upload script, or implement ISR-based display multiplexing, tell me which option and I will update the code.

---
Author: repository maintainer
