/*
===========================================================
 Smart Classroom Management System
 Energy Monitoring Node (LoRa Transmitter)

 Hardware:
 - Arduino Nano
 - ACS712 Current Sensor
 - ZMPT101B Voltage Sensor
 - SX1278 LoRa Module

 Function:
 Measures current, voltage, power and energy consumption,
 then transmits the readings to the LoRa receiver.
===========================================================
*/

#include <SPI.h>
#include <LoRa.h>
#include "ACS712.h"
#include <ZMPT101B.h>

// ==========================================================
// Sensor Pins
// ==========================================================

#define CURRENT_SENSOR_PIN A1
#define VOLTAGE_SENSOR_PIN A0

// ==========================================================
// LoRa SX1278 Pins
// ==========================================================

#define LORA_SS    10
#define LORA_RST    9
#define LORA_DIO0   2

// ==========================================================
// Calibration Parameters
// ==========================================================

float CALIBRATION_CURRENT = 1.02;
float CALIBRATION_VOLTAGE = 0.98;
float CURRENT_OFFSET = 90.0;

ACS712 ACS(CURRENT_SENSOR_PIN, 5.0, 1023, 80);
ZMPT101B voltageSensor(VOLTAGE_SENSOR_PIN, 50.0);

// ==========================================================
// Energy Variables
// ==========================================================

unsigned long previousMillis = 0;

float energyWh = 0.0;
float electricityRate = 5.0;

// ==========================================================

void setup()
{
    Serial.begin(9600);

    while (!Serial);

    Serial.println("Initializing Energy Monitoring Node...");

    ACS.autoMidPoint();
    voltageSensor.setSensitivity(500.0f);

    previousMillis = millis();

    Serial.println("Initializing LoRa...");

    LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

    if (!LoRa.begin(433E6))
    {
        Serial.println("LoRa initialization failed!");
        while (true);
    }

    LoRa.setTxPower(10);

    Serial.println("LoRa Ready");
}

// ==========================================================

void loop()
{
    // -----------------------------
    // Measure Current
    // -----------------------------

    float totalCurrent = 0;

    for (int i = 0; i < 100; i++)
    {
        totalCurrent += ACS.mA_AC();
    }

    float current_mA =
        (totalCurrent / 100.0) * CALIBRATION_CURRENT;

    current_mA -= CURRENT_OFFSET;

    if (current_mA < 10)
        current_mA = 0;

    // -----------------------------
    // Measure Voltage
    // -----------------------------

    float voltage =
        voltageSensor.getRmsVoltage() *
        CALIBRATION_VOLTAGE;

    if (voltage < 10)
        current_mA = 0.01;

    // -----------------------------
    // Calculate Power
    // -----------------------------

    float power =
        voltage * (current_mA / 1000.0);

    // -----------------------------
    // Calculate Energy
    // -----------------------------

    unsigned long currentMillis = millis();

    float elapsedTime =
        (currentMillis - previousMillis) / 1000.0;

    previousMillis = currentMillis;

    if (power > 1.0)
    {
        energyWh +=
            (power * elapsedTime) / 3600.0;
    }

    float energykWh = energyWh / 1000.0;

    float cost =
        energykWh * electricityRate;

    // -----------------------------
    // Prepare LoRa Packet
    // -----------------------------

    String message =
        String(current_mA, 2) + "," +
        String(voltage, 2) + "," +
        String(power, 2) + "," +
        String(energyWh, 4) + "," +
        String(cost, 4);

    // -----------------------------
    // Display Readings
    // -----------------------------

    Serial.println("--------------------------------");

    Serial.print("Current : ");
    Serial.print(current_mA);
    Serial.println(" mA");

    Serial.print("Voltage : ");
    Serial.print(voltage);
    Serial.println(" V");

    Serial.print("Power   : ");
    Serial.print(power);
    Serial.println(" W");

    Serial.print("Energy  : ");
    Serial.print(energyWh);
    Serial.println(" Wh");

    Serial.print("Cost    : ₹");
    Serial.println(cost, 4);

    // -----------------------------
    // Send via LoRa
    // -----------------------------

    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket(true);

    delay(1000);
}
