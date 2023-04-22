#define daTemp 999.8

#include <Arduino.h>

bool lys = false;

float tallet() {
  float pi = analogRead(I0_5);
  if (lys) {
    digitalWrite(Q0_0, HIGH);
  } else {
    digitalWrite(Q0_0, LOW);
  }
  lys = !lys;
  return pi;
}

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

