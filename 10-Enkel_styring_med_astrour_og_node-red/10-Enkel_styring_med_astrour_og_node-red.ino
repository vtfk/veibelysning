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

  I tillegg ligger det miljøvariabler (SSID og PASSORD) i en egen fil som heter config.h. Sjekk README.md for info

  Programmet tenner og slukker utgang Q0_0 basert på soloppgang og solnedgang og kommuniserer med en MQTT-broker og NODE-RED
*/

#include <WiFi.h>
#include <ESP32Time.h>
#include <SolarCalculator.h>
#include "./config.h"
// #include "./bibliotek.h"
#include "./funksjoner_loop1.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Henter og setter nødvendige parameter
const char *ssid = SSID;
const char *password = PASSWORD;

long gmtOffset_sec = 3600;
int daylightOffset_sec = 3600;
const char *ntpServer = "no.pool.ntp.org";

// Bredde- og lengdegrad for Helgeroa ;-) (Brukes for å beregne solopgang og solnedgang)
double latitude = PLC_POS_LAT;
double longitude = PLC_POS_LONG;
int utc_offset = 2;
int year, month, day;
double transit, sunrise, sunset, c_dawn, c_dusk;

//ESP32Time rtc;
ESP32Time rtc(0);

// Globale "tilstander"
bool isDark = false;
bool manuell_styring = true;
bool manuell_lux = false;
bool manuell_toppsystem = false;
bool door_open = false;

// Your MQTT broker ID
const char *mqttBroker = "test.mosquitto.org";
const int mqttPort = 1883;

// MQTT topics
const char *publishTopic = "VTFK/status";
const char *subscribeTopic = "VTFK/man_styring";

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (5)
char msg[MSG_BUFFER_SIZE];

WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Callback function whenever an MQTT message is received
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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // Ikke skru av nettet før man er sikker på at denne kommanoden har kjørt

  // setup the mqtt server and callback
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
}

void loop() {

  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // publish message after certain time.
  unsigned long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    // Sjekker input og publiserer basert på denne
    if (analogRead(I0_5) < 300) {
      Serial.print("Publish message: M1");
      client.publish(publishTopic, "M1");
      Serial.print("\n");
    } else if (analogRead(I0_5) > 700) {
      Serial.print("Publish message: M2");
      client.publish(publishTopic, "M2");
      Serial.print("\n");
    }
  }
  delay(1000);
}


void loop1() {
  Serial.println(rtc.getTime(" x--> %A, %B %d %Y %H:%M:%S\n"));

  year = rtc.getYear();
  month = rtc.getMonth() + 1;
  day = rtc.getDay();
  Serial.print("\nPling plong klokka er: ");
  Serial.print(rtc.getTimeDate(true) + "\n");

  calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);


  // Sjekker tilstanden til lysstyringen.
  isDark = sjekkIsDark(sunrise + utc_offset, sunset + utc_offset, rtc.getHour(true), rtc.getMinute());
  // manuell_styring = false; //sjekkManuell_lux(); // sjekkManuell_styring(); // Erstatt med true/false for å teste
  manuell_lux = false;  // sjekkManuell_lux(); // Erstatt med true/false for å teste
  manuell_toppsystem = false;

  delay(1000);  // Vent 1 sekund

  // Logging til Serial for debugging
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

  if (manuell_styring) {
    Serial.print("Manuell styring aktiv\n");
  }

  if (manuell_toppsystem) {
    Serial.print("Toppsystem styring aktivt!\n");
  }
  int verdi = analogRead(I0_5);
  int status_lys = analogRead(I0_3);


  StaticJsonDocument<100> veilysData;

  veilysData["sensorID"] = PLC_ID;
  // veilysData["PLC_time"] = rtc.getTime();
  //veilysData["posisjon_lat"] = PLC_POS_LAT;
  //veilysData["posisjon_lon"] = PLC_POS_LONG;
  veilysData["lux"] = verdi;
  veilysData["status_lys"] = status_lys;
  veilysData["manuell_styring"] = manuell_styring;
  veilysData["dør_lukket"] = true;

  char buffer[100];
  serializeJson(veilysData, meldingsobjekt);

  Serial.println(meldingsobjekt);

  client.publish(publishTopic, meldingsobjekt);
  delay(3000);  // Vent 3 sekunder før neste sjekk
}
