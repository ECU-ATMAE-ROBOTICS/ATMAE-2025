# ATMAE-2025 Carrrgo Hold

## Overview
This project involves building a robot designed to compete in a 1v1v1v1 object collection challenge at the ATMAE 2025 Student Robotics Compeition. The robot uses a Raspberry Pi 5 and an Arduino Mega to control various components, including sensors and motors. Each robot competes in 4 individual rounds where the robots must pick up and sort colored whiffle balls, go to bins, dispense the colored ball to its bin, maneuver an obstacle, and shoot balls to a bullseye!

# Rules and Rubrics
- Rules: https://atmae.org/wp-content/uploads/2025-ATMAE-Robotics-Rules.pdf

## Hardware
- Raspberry Pi 5: Acts as the main control unit and recieves in the input from the Xbox controller and sends it to the Ardunio over serial.
- Picam v2: Internal sort camera that connects to the Raspberry Pi 5.
- USB Cam: External Cam that that streams feed to external server for autonomous control
- Off-Board Server: Hosts YOLO model that receives USB camera feed and returns alignments instructions to Pi.
- Arduino Giga R1 WiFi: Handles low-level control of motors, sensors, and parses the serial from the Pi. It communicates with the Raspberry Pi via serial.
- Router: Raspberry Pi and client connects to the router to stream camera to a client (in this case it was someones phone)

## Key Components:
- Motors: REV-41-1291 Hex Motor
- Smart Sensors: PiCamera, USB Camera
- 2 YOLOv11m models used (Intended to run on a computer with dedicated NVIDIA GPU) 
- Power Supply: Dewalt Battery 
