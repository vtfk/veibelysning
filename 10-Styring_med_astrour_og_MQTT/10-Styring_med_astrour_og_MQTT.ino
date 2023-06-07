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
  4. PubSubClient - https://pubsubclient.knolleary.net/
  5. ArduinoJson - https://arduinojson.org/

  I tillegg ligger det miljøvariabler i en egen fil som heter config.h. Sjekk README.md for mer info

  Programmet tenner og slukker utgang Q0_0 basert på soloppgang og solnedgang og kommuniserer med en MQTT-broker
*/

// Importerer nødvendige biblioteker
#include <WiFi.h>
#include <ESP32Time.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SolarCalculator.h>

// Lokale konfigfiler og miljøvariabler
#include "./config.h"

// Henter og setter nødvendige parameter fra config.h
const char *ssid = SSID;
const char *password = PASSWORD;

// Klokkkeinnstillinger
long gmtOffset_sec = 3600; // Tidssone + 1 time
int daylightOffset_sec = 3600; // Sommertid +1 time - Vintertid +0 timer
const char *ntpServer = "no.pool.ntp.org"; // Klokkeserver

// Parametre som brukes av SolarCalcultaor (Astrouret)
double latitude = PLC_POS_LAT;
double longitude = PLC_POS_LONG;
int utc_offset = (gmtOffset_sec + daylightOffset_sec) / 3600;
int year, month, day;
double transit, sunrise, sunset, c_dawn, c_dusk;

//ESP32Time-objekt: rtc;
ESP32Time rtc(0);

// Globale "tilstander"
bool isDark = false;
bool manuell_styring = false;
bool manuell_lux = false;
bool manuell_toppsystem = false;
bool door_open = false;

// Your MQTT-innstillinger
const char *mqttBroker = MQTT_BROKER;
const int mqttPort = MQTT_PORT;
const char *mqttUser = MQTT_USER;
const char *mqttPassword = MQTT_PASSWORD;

// MQTT topics
const char *publishTopic = MQTT_PUBLISH_TOPIC;
const char *subscribeTopic = MQTT_SUBSCRIBE_TOPIC;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (5)
char msg[MSG_BUFFER_SIZE];

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Callbak som kjører kommandoer fra toppsystem
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print(message);

  // Switch on the LED if 'ON' was received
  if (message == "Toggle_manuell") {
    manuell_styring = !manuell_styring;
  }

  if (message == "Manuell_ON" && manuell_styring) {
    Serial.println("Manuell - PÅ \n");
    digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
  } else if (message == "Manuell_OFF" && manuell_styring) {
    Serial.println("Manuell - AV \n");
    digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe to topic
      client.subscribe(subscribeTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// *********** Egne funksjoner som tidligere lå i funksjoner_loop1.h *****************
// ************ Rydde i denne delen ***************

// Dene funksjonen brukes kun til debugging - Kun for formattering av utskrift i Serial monitor
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

// **************************************************************************
// **************************************************************************

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

  // Stiller klokka ved reboot - Må kjøres på nytt når man skal endre sommer-/vintertid
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Initiering av MQTT (PubSub-klienten)
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  delay(10000); // Venter i 10 sekunder slik at klokka blir satt for å unngå av/på av utganger ved boot
}

// Hovedløkke - kommunikasjon
void loop() {

  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected()) {
    reconnect();
  }
  // Kall til PubSubClient som prosesserer innkomne og utgående meldinger
  client.loop();

  // Hvis ønskelig kan det publiseres meldinger til faste intervall
  unsigned long now = millis();
  if (now - lastMsg > 60000) {
    lastMsg = now;
    // client.publish(publishTopic, "Nå er det 1 minutt siden jeg har publisert");
  }
  delay(1000);
}

// Hovedløkke - Automatikk
void loop1() {
  Serial.println(rtc.getTime("RTC-tid er: %A, %B %d %Y %H:%M:%S\n"));

  // Setter parametre slik at astrouret kan beregne soloppgang og solnedgang
  year = rtc.getYear();
  month = rtc.getMonth() + 1;  // +1 siden månedene telles fra 0-11 (!)
  day = rtc.getDay();
  
  // Kalkulerer soloppgang og solnedgang for gitt dato
  calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);

  // Sjekker og setter tilstanden til lysstyringen.
  isDark = sjekkIsDark(sunrise + utc_offset, sunset + utc_offset, rtc.getHour(true), rtc.getMinute());
  // manuell_styring = false; //sjekkManuell_lux(); // sjekkManuell_styring(); // Erstatt med true/false for å teste
  // manuell_lux = false;  // sjekkManuell_lux(); // Erstatt med true/false for å teste
  // manuell_toppsystem = false;

  delay(1000);  // Vent 1 sekund

  // Logging til Serial for debugging - Fjernes i prod
  char str[6];
  Serial.print("\nTussmørke: ");
  Serial.print(hoursToString(c_dusk + utc_offset, str));
  Serial.print("\n");
  Serial.print("Grålysning: ");
  Serial.print(hoursToString(c_dawn + utc_offset, str));
  Serial.print("\n");

  // Sjekker isD (isDark()) om det er natt eller dag og tenner/slukker utgang
  if (!manuell_styring && !manuell_toppsystem) {
    if (isDark) {  // isDark er erstattet med true for å teste
      Serial.print("Automatisk styring aktiv - Lampestatus: PÅ!\n");
      digitalWrite(Q0_0, HIGH);
      digitalWrite(R0_8, HIGH);
    } else {
      Serial.print("Automatisk styring aktiv - Lampestatus AV!\n");
      digitalWrite(Q0_0, LOW);
      digitalWrite(R0_8, LOW);
    }
  }

  // Logging av status
  if (manuell_styring) {
    Serial.print("Manuell styring aktiv\n");
  }

  if (manuell_toppsystem) {
    Serial.print("Toppsystem styring aktivt!\n");
  }

  // Her kan man lese inn sensorverdier og andre inputs
  int verdi = analogRead(I0_5); // Simulerer lux-verdi
  int status_lys = analogRead(I0_3); // Leser av status på utgang for lysstyring

  // Lager JSON-objekt som skal sendes til MQTT
  StaticJsonDocument<200> veilysData;
  veilysData["epoch"] = rtc.getEpoch();
  veilysData["skapID"] = PLC_ID;
  veilysData["lux"] = verdi;
  veilysData["status_lys"] = status_lys;
  veilysData["manuell_styring"] = manuell_styring;
  veilysData["dor_lukket"] = true;
  char meldingsobjekt[200];
  serializeJson(veilysData, meldingsobjekt);

  Serial.println(meldingsobjekt);

  client.publish(publishTopic, meldingsobjekt);
  delay(3000);  // Vent 3 sekunder før neste sjekk
}