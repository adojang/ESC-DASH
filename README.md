# ESP-DASH: Dashboard for Esc Rooms

Welcome to the ESP-DASH repository! This codebase is a dashboard designed for a Esc Rooms that allows them to override any electronic puzzle in their rooms and track important metrics.

## Features

- Puzzle Override: Allows the override of any puzzle by clicking a button.
- Metric tracking: The dashboard allows users to track important metrics for each room, such as success rates, average completion times, and the number of times each room has been played.

## Getting started

To get started with ESP-DASH, you will need an ESP32 microcontroller board, as well as Platformio and Visiual Studio Code.
Once you have these components, you can clone the repository and upload the code to your board.

You will also need to file for lib/_EscCore/config.h
Use the provided template and save it as config.h (or just edit the library)

## Usage

Once you have uploaded the code to your board and connected, you should see the dashboard on `escape.local` in your local browser.


## License

ESP-DASH is released under the Apache 2 license. See `LICENSE` for more information.
