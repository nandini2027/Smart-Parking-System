# IoT Smart Parking System using ESP32

An IoT-based Smart Parking System developed using the **ESP32** microcontroller for intelligent parking management. The system automatically allocates parking slots, detects vehicle occupancy in real time using ultrasonic sensors, controls an automated barrier gate, provides LED and LCD-based guidance, and hosts a live web dashboard for parking status monitoring.

---
## Table of Contents

- Overview
- Features
- Hardware
- Software
- Working Principle
- Circuit Diagram
- Images
- Technologies Used
- Future Improvements
#  Project Overview

Finding an available parking space in crowded parking lots often leads to unnecessary fuel consumption, increased traffic congestion, and wasted time.

This project provides an automated parking solution that guides drivers to the nearest available parking slot using real-time occupancy detection. An ESP32 microcontroller processes sensor data, controls the entry barrier, updates parking status, and serves an IoT web dashboard accessible over Wi-Fi.

---

# Features

- Automatic parking slot allocation
- Real-time parking slot occupancy detection
- Automated barrier gate using SG90 Servo Motor
- LED-based parking guidance (Green, Yellow & Red)
- LCD display showing allocated parking slot
- Live IoT Web Dashboard
- Real-time Check-In and Check-Out information
- Automatic parking duration calculation
- Automatic slot availability update after vehicle exit
- Wi-Fi enabled monitoring using ESP32

---

# Hardware Components

| Component | Quantity |
|-----------|:--------:|
| ESP32 DevKit V1 | 1 |
| HC-SR04 Ultrasonic Sensor | 6 |
| IR Obstacle Sensor Module | 1 |
| SG90 Micro Servo Motor | 1 |
| 16×2 LCD Display with I2C Module | 1 |
| 74HC595 Shift Register | 2 |
| Bi-color LEDs (Red/Green) | 6 |
| 220 Ω Resistors | 12 |
| Voltage Divider Resistors (1 kΩ & 2 kΩ) | 6 Pairs |
| Breadboard | 1 |
| Jumper Wires | Multiple |
| Micro USB Cable | 1 |

For the complete list of hardware components, descriptions, specifications, and quantities used in this project, refer to 
[Components List](Components_List.pdf)

---

# Software & Libraries

- Arduino IDE
- Embedded C++
- ESP32 Board Package
- WiFi Library
- LiquidCrystal_I2C Library
- ESP32Servo Library
- time.h Library

---

# Working Principle

1. Vehicle arrival is detected using the IR sensor.
2. ESP32 allocates the next available parking slot.
3. The allocated slot LED starts blinking to guide the driver.
4. LCD displays the allocated parking slot along with color guidance.
5. Servo motor opens the entry barrier.
6. The parking slot is confirmed only if the vehicle remains in front of the ultrasonic sensor for **2 seconds**.
7. The LED changes to **red**, indicating the slot is occupied.
8. Check-In time is recorded automatically.
9. Parking duration is calculated in real time.
10. When the vehicle leaves and the slot remains empty for **2 seconds**, the slot becomes available again and Check-Out time is recorded.

---

# IoT Dashboard

The ESP32 hosts a web dashboard displaying:

- Total Available Parking Slots
- Live Parking Slot Status
- Check-In Time
- Check-Out Time
- Parking Duration
- Automatic Status Refresh

---

# Project Images


---

# Technologies Used

- Embedded Systems
- Internet of Things (IoT)
- ESP32 Programming
- Wi-Fi Communication
- Embedded C++
- PWM Servo Control
- I2C Communication
- Shift Register Interfacing
- Ultrasonic Distance Measurement
- HTML Web Dashboard
- Network Time Protocol (NTP)

---

# Future Improvements

- Mobile application integration
- Cloud database connectivity
- QR Code based vehicle authentication
- Automatic billing system
- Number Plate Recognition (ANPR)
- Online parking reservation
- Multi-floor parking support
- Mobile notifications

---

#  Author

**Nandini Singh**

B.Tech Electronics(IoT)

---



This project has been developed for educational and academic purposes.
