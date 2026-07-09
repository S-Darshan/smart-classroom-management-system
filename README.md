# Smart Classroom Management System using Face Recognition, LoRa and IoT

## Overview

The **Smart Classroom Management System** is an IoT-enabled solution that combines **face recognition**, **automated attendance**, **classroom automation**, **energy monitoring**, and **LoRa-based wireless communication**. The system is designed to improve classroom management by automating attendance, reducing energy wastage, and enabling remote monitoring through the Blynk IoT platform.

The system uses an **ESP32-CAM** to stream live video to a Python application for face recognition. Attendance is recorded only after validating the student's classroom presence for a minimum duration. Classroom occupancy is used to automatically control classroom lighting. Simultaneously, current and voltage sensors monitor power consumption, and the measured energy data is transmitted wirelessly using LoRa to a central monitoring node.

---

## Features

- Real-time face recognition using ESP32-CAM
- Automatic student registration and face encoding
- Automated attendance logging with entry and exit time
- Minimum classroom duration validation before marking attendance
- Classroom occupancy detection
- Automatic classroom light control using relay module
- Real-time current, voltage, power and energy monitoring
- Long-range wireless communication using LoRa SX1278
- Remote monitoring through Blynk IoT dashboard
- Attendance records stored in CSV format

---

## System Architecture

```
                    Student
                       │
                       ▼
              ESP32-CAM Live Stream
                       │
                       ▼
          Python Face Recognition System
                       │
      ┌────────────────┴────────────────┐
      │                                 │
Attendance Logging              Occupancy Detection
      │                                 │
      ▼                                 ▼
Attendance.csv                  Blynk Cloud (V5)
                                        │
                                        ▼
                              ESP32 Relay Controller
                                        │
                                        ▼
                               Classroom Light

====================================================

 ACS712 + ZMPT101B Sensors
            │
            ▼
      Arduino Nano
            │
     Power Calculation
            │
            ▼
      LoRa SX1278 TX
            │
            ▼
      ESP32 LoRa RX
            │
            ▼
     Blynk Dashboard
```

---

## Hardware Components

- AI Thinker ESP32-CAM
- ESP32 Development Board
- Arduino Nano
- SX1278 LoRa Module
- ACS712 Current Sensor
- ZMPT101B Voltage Sensor
- Relay Module
- AC Bulb
- Wi-Fi Router

---

## Software Requirements

- Python 3.x
- Arduino IDE
- ESP32 Board Package
- OpenCV
- face_recognition
- NumPy
- Pandas
- Requests
- Blynk IoT Platform

---

## Technologies Used

- Python
- OpenCV
- face_recognition
- ESP32-CAM
- ESP32
- Arduino Nano
- LoRa SX1278
- Blynk IoT
- Computer Vision
- IoT
- Embedded Systems

---

## Working Principle

### Face Recognition Module

1. ESP32-CAM streams live video.
2. Python receives the video stream.
3. Student faces are detected and encoded.
4. Registered students are recognized.
5. Entry time is recorded.
6. Attendance is marked only after validating the minimum classroom duration.
7. Attendance is stored in a CSV file.

---

### Classroom Automation

- The system continuously monitors classroom occupancy.
- If students are present, the classroom light remains ON.
- If no students are detected, the relay automatically switches OFF the classroom light.

---

### Energy Monitoring

The Arduino Nano continuously measures:

- Current
- Voltage
- Power
- Energy Consumption
- Electricity Cost

The measured data is transmitted using LoRa.

---

### LoRa Communication

Energy monitoring data is transmitted wirelessly from the sensing node to the monitoring node using LoRa SX1278 operating at 433 MHz.

---

### Blynk Dashboard

The Blynk application displays:

- Current
- Voltage
- Power
- Energy Consumption
- Electricity Cost
- Classroom Occupancy Status

---

## Project Structure

```
Smart-Classroom-Management-System
│
├── FaceRecognition
│   ├── attendance_system.py
│   ├── Attendance.csv
│   └── student_images
│
├── EnergyMonitoring
│   └── energy_monitor_transmitter.ino
│
├── RelayController
│   └── relay_controller.ino
│
├── LoRaReceiver
│   └── lora_receiver.ino
│
├── requirements.txt
└── README.md
```

---

## Future Enhancements

- Face anti-spoofing
- Database integration (MySQL/Firebase)
- Email or SMS attendance notification
- Cloud-based attendance management
- Multiple classroom support
- Student and faculty web dashboard
- Mobile application integration

---

## Author

**Darshan S**

Electronics and Communication Engineering Student

---

## Acknowledgement

This project was developed as an academic IoT and Computer Vision project to demonstrate smart classroom automation using embedded systems, computer vision, wireless communication, and cloud-based monitoring.
