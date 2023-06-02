
// Dene funksjonen trengs ikke - Kun for formattering av utskrift i Serial monitor
char* hoursToString(double h, char* str) {
  int m = int(round(h * 60));
  int hr = (m / 60) % 24;
  int mn = m % 60;

  str[0] = (hr / 10) % 10 + '0';
  str[1] = (hr % 10) + '0';
  str[2] = ':';
  str[3] = (mn / 10) % 10 + '0';
  str[4] = (mn % 10) + '0';
  str[5] = '\0';
  return str;
}

// Funksjon som sjekker om det er "mørkt" ute
// opp = tidspunkt for soloppgang, ned = tidspunkt for solnedgang, timer = time, minutter = minutter i hele timer
bool sjekkIsDark(double opp, double ned, int timer, int minutter) {
  // Logger til Serial monitor for å sjekke at det fungerer
  Serial.print("\nSola opp: ");
  char str[6];
  Serial.print(hoursToString(opp, str));
  Serial.print("\nSola ned: ");
  Serial.print(hoursToString(ned, str));
  Serial.print("\nKlokka akkurat nå er: ");
  Serial.print(String(timer) + ":" + String(minutter));
  Serial.print("\n");

  // Skjøter sammen timer og minutter til en float
  float hm = timer + float(minutter) / 60;

  if (hm < opp || hm > ned) {
    return true;
  } else {
    return false;
  }
}

// Må kobles til bryter for å fungere skikelig
bool sjekkManuell_lux() {
  if (analogRead(I0_5) > 300)
    {
      return true;
    } else {
      return false;
    }
}

// Må kobles til bryter for å fungere skikelig
bool sjekkManuell_styring() {
  if (analogRead(I0_2) > 500)
    {
      return true;
    } else {
      return false;
    }
}
