## Duck Buddy

## Overview
Duck Buddy is a study buddy that detects how long a person is studying and times a study session for 30 minutes.

## Project
This project was created for CPEG/ELEG 298 during the Spring 2026 semester.

## Hardware and Software 
Arduino UNO, Ultrasonic distance sensor, LEDs, and UART connection - 
Arduino IDE and C++

## How It Works
This IOT device takes in distance data from an HC-SR04 ultrasonic sensor. Every second, the device checks for a user's presence and updates their study or absence status. This data is then sent to Adafruit IO using an ESP8266 Wi-Fi module. Four thru-hole LEDs (Red, yellow, blue, green) are used to provide a visual status of the study session, transitioning between high and low states depending on that time. 

## Author
Jenna Fontana
