#include <Arduino.h>

bool lys = false;  // Toggle status lys

// Funksjon som leser "sensordata" og returnerer
float sensordata() {
  float sensordata = analogRead(I0_5);
  // Kodesnutt som togggler lys av/på. Bare for mor skyld...
  if (lys) {
    digitalWrite(Q0_0, HIGH);
  } else {
    digitalWrite(Q0_0, LOW);
  }
  lys = !lys;
  return sensordata;
}

// Funksjon som trigges av kommando fra Azure IoT-central
void veilyson() {
  Serial.write("Nuuuu kjørrrr");

  // Løkke for å lage litt action på utgangene :-)
  for (int i = 0; i < 10; i++) {
    digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
    delay(100);
    digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
    delay(100);
  }
}

void veilysoff() {
  Serial.write("Nuuuu skrudde vi av!!!");

  // Løkke for å lage litt action på utgangene :-)
  for (int i = 0; i < 10; i++) {
    digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
    delay(100);
    digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
    delay(100);
  }
}




