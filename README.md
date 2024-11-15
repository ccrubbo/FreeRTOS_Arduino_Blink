# FreeRTOS_Arduino_Blink

## Requirements
This project was designed for and requires an `Arduino Uno R4 WiFi` board. 

## Application Overview
The Arduino sketch will build FreeRTOS firmware that will:
1. Initialize serial for debug prints
2. Create the `loop` thread (unused)
3. Create the `blink` thread (used to blink the second LED) 
4. Start a 16-bit timer peripheral called the "AGT" timer which is used to both:
   - Toggle the first LED when the 16-bit timer count reaches 65,000
   - Notify the `blink` thread and start the second LED blink sequence every three seconds

## Test Setup
### Board Configuration
Attach logic analyzer lines and/or LEDs/resistors to the following pins:
- `DIGITAL0`: optional pin associated with "debug" line for logic analyzer
- `DIGITAL1`: pin associated with the "first" LED
- `DIGITAL2`: pin associated with the "second" LED

### Building and Flashing
Use the steps below to setup the project environment and flash your board
1. Download and install the Arduino IDE https://www.arduino.cc/en/software
2. Open the Arduino IDE library manager and install the `FreeRTOS` library
3. Use `File > Open` and select the `FreeRTOS_Arduino_Blink.ino` sketch file
4. Plug in the USB to your board and select it using the dropdown list at the top of the Arduino IDE
5. Click the upload arrow button to build the project and flash it to your board


## Test Results
Using a logic analyzer, I captured 10 minutes of data to verify timings. See the screenshot below, where:
- `DBG`: toggled every time the first LED starts its blink sequence
- `LED1`: toggled when the 16-bit timer count reaches 65,000
- `LED2`: starts the second LED blink sequence every three seconds

![logic-analyzer-capture](https://github.com/ccrubbo/FreeRTOS_Arduino_Blink/blob/main/DSView_capture.png)

To inspect the capture file uploaded to this repository, download DSView https://www.dreamsourcelab.com/download/ 
