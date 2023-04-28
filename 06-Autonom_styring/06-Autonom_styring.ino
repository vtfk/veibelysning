/*
  Autonom styring av lys med en Industrial Shields PLC ESP32 19R+

  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/

  I dette programmet brukes bibliotekene:

  1. Wifi - https://github.com/arduino-libraries/WiFi
  2. ESP32Time - https://github.com/fbiego/ESP32Time/tree/main
  3. SolarCalculator - https://github.com/jpb10/SolarCalculator

  I tillegg ligger det miljøvariabler (SSID og PASSORD) i en egen fil som heter config.h. Sjekk README.md for info

  Programmet tenner og slukker utgang Q0_0 basert på soloppgang og solnedgang
*/

#include <WiFi.h>
#include <ESP32Time.h>
#include <SolarCalculator.h>
#include "./config.h"
#include "./bibliotek.h"

// Henter og setter nødvendige parameter
const char* ssid = SSID;
const char* password = PASSWORD;

long gmtOffset_sec = 3600;
int daylightOffset_sec = 3600;
const char* ntpServer = "no.pool.ntp.org";

// Bredde- og lengdegrad for Helgeroa ;-) (Brukes for å beregne solopgang og solnedgang)
double latitude = 58.99019;
double longitude = 9.879223;
int utc_offset = 2;
int year, month, day;
double transit, sunrise, sunset;

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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // Ikke skru av nettet før man er sikker på at denne kommanoden har kjørt

  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
}

void loop() {
  Serial.println(rtc.getTime(" x--> %A, %B %d %Y %H:%M:%S\n"));
  
  year = rtc.getYear();
  month = rtc.getMonth() + 1;
  day = rtc.getDay();
  Serial.print("\nPling plong klokka er: ");
  Serial.print(rtc.getTimeDate(true) + "\n");

  calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);

  if (isDark(sunrise + utc_offset, sunset + utc_offset, rtc.getHour(true), rtc.getMinute())) {
    Serial.print("Nå errre natta!");
    digitalWrite(Q0_0, HIGH);
  } else {
    Serial.print("Nå errre lyse dagen!");
    digitalWrite(Q0_0, LOW);
  }
 
  delay(1000);
}
