/*
  Enkel test på bruk av WiFi og innhenting av dato/tid fra NTP-server på en Industrial Shields PLC ESP32 19R+
  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/

  I dette programmet brukes bibliotekene:

  1. WiFi - https://github.com/arduino-libraries/WiFi
  2. ESP32Time - https://github.com/fbiego/ESP32Time/tree/main

  I tillegg ligger det miljøvariabler (SSID og PASSORD) i en egen fil som heter config.h. Sjekk README.md for info

  Programmet skriver ut tid og dato i consollen
*/

#include <WiFi.h>
#include <ESP32Time.h>
#include "./config.h"

// Henter og setter nødvendige parameter
const char* ssid = SSID;
const char* password = PASSWORD;

long gmtOffset_sec = 3600;
int daylightOffset_sec = 3600;
const char* ntpServer = "no.pool.ntp.org";

//ESP32Time rtc;
ESP32Time rtc(0);

void setup() {
  Serial.begin(115200);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // Ikke skru av nettet før man er sikker på at denne kommanoden har kjørt

}

void loop() {
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
  delay(1000);
}
