
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

// Funksjon som sjekker om det "mørkt"
bool isDark(double opp, double ned, int timer, int minutter) {
  Serial.print("\nSola opp: ");
  char str[6];
  Serial.print(hoursToString(opp, str));
  Serial.print("\nSola ned: ");
  Serial.print(hoursToString(ned, str));
  Serial.print("\nKlokka akkurat nå er: ");
  Serial.print(String(timer) + ":" + String(minutter));
  float hm = timer + float(minutter) / 60;
  Serial.print("\n");
  //Serial.print(hm);

  if (hm < opp || hm > ned) {
    return true;
  } else {
    return false;
  }
}
