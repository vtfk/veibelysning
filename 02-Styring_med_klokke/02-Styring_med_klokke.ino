/*
  Enkel intro og test av hvordan inngganger og utganger kan styres av klokkeen på Industrial Shields PLC ESP32 19R+
  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/

  I dette programmet brukes bibliotekene:

  1. Beregning soloppgang/-nedgang SolarCalculator Library for Arduino example sketch: SunriseSunset.ino - https://github.com/jpb10/SolarCalculator/
  2. Klokke: Grove RTC-biblioteket - https://github.com/Seeed-Studio/RTC_DS1307

  Programmet tenner og slukker innganger på PLC annenhvert minutt.
*/

#include <SolarCalculator.h>
#include <DS1307.h>

DS1307 clock;//Lager et klokkeobjekt fra klassen DS1307 som finnes i RTC-biblioteket

/* Hvis ønskelig så kan dato/tid settes manuelt - Bør hentes fra en NTP-server
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

const byte day = 20;
const byte month = 4;
const byte year = 23;
*/

// Bredde- og lengdegrad for Helgeroa ;-)
double latitude = 58.9850417;
double longitude = 9.8623531;
int utc_offset = 2;

double transit, sunrise, sunset;

void setup()
{
  Serial.begin(9600);
  clock.begin();
  /*
  // Her kan klokken stilles om ønskelig
  clock.fillByYMD(2023, 4, 20); //Jan 19,2013
  clock.fillByHMS(10, 15, 30); //15:28 30"
  clock.fillDayOfWeek(THU);//Saturday
  clock.setTime();//write time to the RTC chip
  */

  // Kalkulerer tidspunkter for soloppgange/-nedgang
  calcSunriseSunset(clock.year, clock.month, clock.dayOfMonth, latitude, longitude, transit, sunrise, sunset);
}

void loop()
{
  // printTime();
  clock.getTime();

  // Skriver ut tidspunkt for soloppgang/nedgang i hele timer
  Serial.print("Soloppgang:" + String(sunrise) + "\n");
  Serial.print("Solnedgang:" + String(sunset) + "\n");
  Serial.print("\n");

  if (clock.minute % 2 == 0) {
    Serial.write("Utganger skrur seg PÅ\n");
    digitalWrite(Q0_0, HIGH); // Digital 0-5 VO
    digitalWrite(R0_8, HIGH); // Rele av/på
    analogWrite(A0_2, 4095);  // Analog 12bit 0V-10V (Oppløsning: 4094 verdier)
  } else {
    Serial.write("Utganger skrur seg AV\n");
    digitalWrite(Q0_0, LOW); // Digital 0-5 VO
    digitalWrite(R0_8, LOW); // Rele av/på
    analogWrite(A0_2, 0);  // Analog 12bit 0V-10V (Oppløsning: 4094 verdier)
  }
  delay(1000);
}
