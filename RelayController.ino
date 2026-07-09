/*
===========================================================
 Smart Classroom Management System
 Relay Controller (ESP32)

 Hardware:
 - ESP32 Dev Board
 - Relay Module
 - AC Light

 Function:
 Receives classroom occupancy status from the
 Blynk Cloud and controls the classroom light.

 V5 = 1 → Relay ON
 V5 = 0 → Relay OFF
===========================================================
*/

#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// ==========================================================
// WiFi Credentials
// ==========================================================

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ==========================================================
// Hardware Configuration
// ==========================================================

#define RELAY_PIN 5

// ==========================================================
// Blynk Virtual Pin Callback
// ==========================================================

BLYNK_WRITE(V5)
{
    int relayState = param.asInt();

    digitalWrite(RELAY_PIN, relayState);

    if (relayState)
    {
        Serial.println("Classroom Occupied - Light ON");
    }
    else
    {
        Serial.println("Classroom Empty - Light OFF");
    }
}

// ==========================================================

void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);

    // Keep relay OFF during startup
    digitalWrite(RELAY_PIN, LOW);

    Serial.println("--------------------------------");
    Serial.println("Smart Classroom Relay Controller");
    Serial.println("--------------------------------");

    Serial.println("Connecting to WiFi...");

    Blynk.begin(
        BLYNK_AUTH_TOKEN,
        ssid,
        pass
    );

    Serial.println("Connected to Blynk.");
}

// ==========================================================

void loop()
{
    Blynk.run();
}
