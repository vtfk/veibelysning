#include <Arduino.h>

boolean toppsystem_aktivert = false;

// Funksjon som leser "sensordata" og returnerer
float sensordata() {
  float sensordata = analogRead(I0_5);
  Serial.print("\nStatus på lys er: ");
  Serial.print(analogRead(I0_3));
  Serial.print("\n");
  return sensordata;
}

float lysstatus() {
  float sensordata = analogRead(I0_3);
  return sensordata;
}

// Funksjon som trigges av kommando fra Azure IoT-central
void veilyson() {
  Serial.write("Azure: lys PÅ!");
  toppsystem_aktivert = true;

  // Løkke for å lage litt action på utgangene :-)
  for (int i = 0; i < 10; i++) {
    digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
    delay(100);
    // digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
    delay(100);
  }
}

void veilysoff() {
  Serial.write("Azure: Lys AV!");
  toppsystem_aktivert = false;

  // Løkke for å lage litt action på utgangene :-)
  for (int i = 0; i < 10; i++) {
    // digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
    delay(100);
    digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
    delay(100);
  }
}





