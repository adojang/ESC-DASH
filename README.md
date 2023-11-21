# ESP-DASH: Dashboard for Esc Rooms

Welcome to the ESP-DASH repository! This codebase is a dashboard designed for a Esc Rooms that allows them to override any electronic puzzle in their rooms and track important metrics.

## Features

- Puzzle Override: Allows the override of any puzzle by clicking a button.
- Metric tracking: The dashboard allows users to track important metrics for each room, such as success rates, average completion times, and the number of times each room has been played.

## Getting started

To get started with ESP-DASH, you will need an ESP32 microcontroller board, as well as Platformio and Visiual Studio Code.
Once you have these components, you can clone the repository and upload the code to your board.
###
Installation Steps:
- Install Visual Studio Code
- Install PlatformIO as an Extension in Visual Studio Code
- Add your local Wifi network under configexample.h and rename the file ```config.h```
- Inside ```platformio.ini``` you will need to select which file you want to upload to the ESP32.

## Usage

Once you have uploaded the code to your board and connected, you should see the dashboard on `escape.local` in your local browser.


## License

ESP-DASH is released under the Apache 2 license. See `LICENSE` for more information.
