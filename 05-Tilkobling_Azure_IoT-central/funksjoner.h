


void test_funksjon(const char* msg) {
  Serial.print("Dette er en custom funksjon");
  Serial.print(msg);
}


void lys_on(const char* msg) {
  digitalWrite(3, HIGH);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
}

void lys_off(const char* msg) {
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
}

void testkommando(const char* msg) {
  digitalWrite(5, HIGH);
}