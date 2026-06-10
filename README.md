Teensy audio project sidechain ducker.

## Hardware

- Teensy 4.1
- PCM1808 (I2S ADC)
- PCM5102 (I2S DAC)

Compressor diagram:
https://raw.githubusercontent.com/p-hlp/CTAGDRC/refs/heads/master/Documentation/Base-Diagram.png

## Flashing

Open this folder as the sketch (`volumegate.ino` must live alongside `effect_compressor.cpp`).

### Arduino IDE

1. Open `volumegate.ino` in Arduino IDE.
2. Set **Board** to **Teensy 4.1**.
3. Click **Upload**. Press the Teensy **Program** button if prompted.

### Arduino CLI

Install once:

```bash
brew install arduino-cli
arduino-cli core install teensy:avr
```

Build and upload from the project directory:

```bash
cd /path/to/volumegate
arduino-cli compile --fqbn teensy:avr:teensy41 .
arduino-cli board list
arduino-cli upload --fqbn teensy:avr:teensy41 -p "/dev/cu.usbmodemXXXX" .
```

Use the exact port from `arduino-cli board list` (for example `/dev/cu.usbmodem1101`). Quote the path so zsh does not expand `*`. If upload fails, press the Teensy **Program** button and retry.

Serial debug output is at **115200** baud.

## Todo

- install arduino language server: https://github.com/neovim/nvim-lspconfig/blob/master/lua/lspconfig/configs/arduino_language_server.lua
