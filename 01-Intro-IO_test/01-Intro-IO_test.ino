/*
  Enkel intro og test av IO-porter på Industrial Shields PLC ESP32 19R+
  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/
*/

// Initiering og oppsett
void setup() {
  Serial.begin(9600);  // Klargjør til utskrift i Serial-monitor
}

// Hovedløkka
void loop() {
  // Utganger skrus på
  Serial.write("Utganger skrur seg PÅ\n");
  digitalWrite(Q0_0, HIGH); // Digital 0-5 VO
  digitalWrite(R0_8, HIGH); // Rele av/på
  analogWrite(A0_2, 4095);  // Analog 12bit 0V-10V (Oppløsning: 4094 verdier)
  delay(500);

  // Utganger skrus av
  Serial.write("Utganger skrur seg AV\n");
  digitalWrite(Q0_0, LOW);
  digitalWrite(R0_8, LOW);
  analogWrite(A0_2, 0);
  delay(500);
}
