/*
===========================================================
 Smart Classroom Management System
 LoRa Receiver Node (ESP32)

 Hardware:
 - ESP32 Dev Board
 - SX1278 LoRa Module
 - WiFi
 - Blynk IoT

 Function:
 Receives energy monitoring data from the LoRa transmitter,
 parses the received values, and uploads them to the
 Blynk IoT Dashboard.
===========================================================
*/

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SPI.h>
#include <LoRa.h>

// ==========================================================
// WiFi Credentials
// ==========================================================

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ==========================================================
// Blynk Virtual Pins
// ==========================================================

#define VPIN_CURRENT   V0
#define VPIN_VOLTAGE   V1
#define VPIN_POWER     V2
#define VPIN_ENERGY    V3
#define VPIN_COST      V4

// ==========================================================
// LoRa Pin Configuration
// ==========================================================

#define LORA_SCK    18
#define LORA_MISO   19
#define LORA_MOSI   23
#define LORA_SS      5
#define LORA_RST    14
#define LORA_DIO0   26

#define BUZZER_PIN  27

// ==========================================================

void setup()
{
    Serial.begin(9600);

    while (!Serial);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("--------------------------------");
    Serial.println("Smart Classroom LoRa Receiver");
    Serial.println("--------------------------------");

    Serial.println("Connecting to WiFi...");

    Blynk.begin(
        BLYNK_AUTH_TOKEN,
        ssid,
        pass
    );

    Serial.println("Initializing LoRa...");

    SPI.begin(
        LORA_SCK,
        LORA_MISO,
        LORA_MOSI,
        LORA_SS
    );

    LoRa.setPins(
        LORA_SS,
        LORA_RST,
        LORA_DIO0
    );

    if (!LoRa.begin(433E6))
    {
        Serial.println("LoRa Initialization Failed!");

        digitalWrite(BUZZER_PIN, HIGH);

        while (true);
    }

    Serial.println("LoRa Ready");
}

// ==========================================================

void loop()
{
    Blynk.run();

    int packetSize = LoRa.parsePacket();

    if (packetSize <= 0)
        return;

    String receivedData = "";

    while (LoRa.available())
    {
        receivedData += (char)LoRa.read();
    }

    Serial.println("--------------------------------");
    Serial.println("LoRa Packet Received");
    Serial.println(receivedData);

    float current;
    float voltage;
    float power;
    float energy;
    float cost;

    int parsed =
        sscanf(
            receivedData.c_str(),
            "%f,%f,%f,%f,%f",
            &current,
            &voltage,
            &power,
            &energy,
            &cost
        );

    if (parsed == 5)
    {
        Serial.println("Parsed Successfully");

        Serial.print("Current : ");
        Serial.print(current);
        Serial.println(" mA");

        Serial.print("Voltage : ");
        Serial.print(voltage);
        Serial.println(" V");

        Serial.print("Power   : ");
        Serial.print(power);
        Serial.println(" W");

        Serial.print("Energy  : ");
        Serial.print(energy);
        Serial.println(" Wh");

        Serial.print("Cost    : ₹");
        Serial.println(cost, 4);

        // Upload to Blynk Dashboard

        Blynk.virtualWrite(VPIN_CURRENT, current);
        Blynk.virtualWrite(VPIN_VOLTAGE, voltage);
        Blynk.virtualWrite(VPIN_POWER, power);
        Blynk.virtualWrite(VPIN_ENERGY, energy);
        Blynk.virtualWrite(VPIN_COST, cost);
    }
    else
    {
        Serial.println("Invalid Packet Format");
    }

    delay(100);
