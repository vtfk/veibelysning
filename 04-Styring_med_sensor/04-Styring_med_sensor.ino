/*
  Enkel test på bruk avtemperatursensor på en Industrial Shields PLC ESP32 19R+
  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/

  I dette programmet brukes bibliotekene:

  Programmet leser av verdi fra temperatursensor skriver ut resultatet i konsollen
*/

/* How to use the DHT-22 sensor with Arduino uno
   Temperature and humidity sensor
   More info: http://www.ardumotive.com/how-to-use-dht-22-sensor-en.html
   Dev: Michalis Vasilakis // Date: 1/7/2015 // www.ardumotive.com */

#include <OneWire.h>
#include <DallasTemperature.h>

#define ADDR_LEN 8
#define PIN I0_4

OneWire oneWire(PIN);
DallasTemperature sensors(&oneWire);

////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600L);
  Serial.println("ds18xx-list-devices started");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  Serial.println("----------------------------------------------------");

sensors.begin();

int deviceCount = sensors.getDeviceCount();
  Serial.print("Devices: ");
  Serial.println(deviceCount);

int ds18Count = sensors.getDS18Count();
  Serial.print("DS18xx devices: ");
  Serial.println(ds18Count);

uint8_t address[ADDR_LEN];
  for (int i = 0; i < deviceCount; ++i) {
    if (sensors.getAddress(address, i)) {
      Serial.print("Address: ");
      printAddress(address);
    }
  }
  delay(5000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void printAddress(const uint8_t *address) {
  for (int i = 0; i < ADDR_LEN; ++i) {
    if (i > 0) {
      Serial.print('-');
    }
    if (address[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(address[i], HEX);
  }
  if (!sensors.validFamily(address)) {
    Serial.print(" (not supported)");
  }
  Serial.println();
}