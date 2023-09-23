/*
  Autonom styring av lys med en Industrial Shields PLC ESP32 19R+

  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  
  Lisens: CC BY SA - VTFK 2023

  Koden er kjørt i Arduino IDE v2.0.4
  Husk å bruke v.2.0.7 av industrialshields-esp32 board-biblioteket.

  Les mer om PLC ESP32 19R+ på: https://www.industrialshields.com/

  I dette programmet brukes bibliotekene:

  1. Ethernet/EthernetUdp - https://github.com/arduino-libraries/Ethernet/tree/master
  2. ESP32Time - https://github.com/fbiego/ESP32Time/tree/main
  3. SolarCalculator - https://github.com/jpb10/SolarCalculator
  4. PubSubClient - https://pubsubclient.knolleary.net/
  5. ArduinoJson - https://arduinojson.org/

  I tillegg ligger det miljøvariabler i en egen fil som heter config.h. Sjekk README.md for mer info

  Programmet tenner og slukker utgang Q0_0 basert på soloppgang og solnedgang og kommuniserer med en MQTT-broker via Ethernet-gateway 
*/

// Importerer nødvendige biblioteker
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ESP32Time.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SolarCalculator.h>

// Lokale konfigfiler og miljøvariabler
#include "./config.h"

// Klokkkeinnstillinger
long gmtOffset_sec = 3600;      // Tidssone i Norge + 1 time
int daylightOffset_sec = 3600;  // Sommertid + 3600 sekunder - Vintertid + 0 sekunder

// NTP UDP - Må ryddes?
unsigned int localPort = 8888;                // local port to listen for UDP packets
const char timeServer[] = "no.pool.ntp.org";  // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48;               // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];           //buffer to hold incoming and outgoing packets

// Parametre som brukes av SolarCalcultaor (Astrouret)
double latitude = PLC_POS_LAT;
double longitude = PLC_POS_LONG;
int utc_offset = (gmtOffset_sec + daylightOffset_sec);
int utc_offset_hour = utc_offset / 3600;  // Endre navn på denne variabelen?
int year, month, day;
double transit, sunrise, sunset, c_dawn, c_dusk;

//ESP32Time-objekt: rtc;
ESP32Time rtc(utc_offset);

// Globale "tilstander"
bool isDark = false;
bool manuell_styring = false;
bool manuell_lux = false;
int sensor_lux = 0;
bool manuell_toppsystem = false;
bool door_open = false;
bool status_lys = false;

// MQTT-innstillinger
const char *mqttBroker = MQTT_BROKER;
const int mqttPort = MQTT_PORT;
const char *mqttUser = MQTT_USER;
const char *mqttPassword = MQTT_PASSWORD;

// MQTT topics og meldingsbuffer
const char *publishTopic = MQTT_PUBLISH_TOPIC;
const char *subscribeTopic = MQTT_SUBSCRIBE_TOPIC;
unsigned long lastMsg = 0;
unsigned long lastState = 0;

#define MSG_BUFFER_SIZE (5)
char msg[MSG_BUFFER_SIZE];

// Ethernet og udp-klienter;
EthernetClient ethClient;
PubSubClient client(ethClient);

// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Update these with values suitable for your network.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Denne skjønner jeg ikke helt. ....ennå. Men det virker. :-)

// MQTT-Callbak som kjører kommandoer fra toppsystem
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print(message);

  if (message == "Toggle_manuell") {
    manuell_styring = !manuell_styring;
    publiserTilstand();
  }

  // Restarter PLC
  if (message == "RESTART_PLC") {
    Serial.print("RESTARTER SYSTEMET!!!");
    ESP.restart();
  }

  // Still klokka
  if (message == "STILL_KLOKKA") {
    Serial.print("Stiller klokka");
    stillKlokka();
  }

  if (message == "Manuell_ON" && manuell_styring) {
    Serial.println("Manuell - PÅ \n");
    digitalWrite(Q0_0, HIGH);
    digitalWrite(R0_8, HIGH);
    status_lys = true;
    publiserTilstand();
  } else if (message == "Manuell_OFF" && manuell_styring) {
    Serial.println("Manuell - AV \n");
    digitalWrite(Q0_0, LOW);
    digitalWrite(R0_8, LOW);
    status_lys = false;
    publiserTilstand();
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
    if (client.connect(clientId.c_str(), mqttUser, mqttPassword)) {
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

// Dene funksjonen brukes kun til debugging - Kun for formattering av utskrift i Serial monitor
char *hoursToString(double h, char *str) {
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

  Serial.print("\nDST: " + String(daylightOffset_sec) + "\n");

  // Skjøter sammen timer og minutter til en float
  float hm = timer + float(minutter) / 60;

  if (hm < opp || hm > ned) {
    return true;
  } else {
    return false;
  }
}

int sjekkDST() {
    int dow = rtc.getDayofWeek();
    int d = rtc.getDay();
    int h = rtc.getHour(true);
    int mon = rtc.getMonth();

    if (dow == 0 && mon == 2 && d >= 25 && h == 1 && daylightOffset_sec == 0) { 
      daylightOffset_sec = 3600;
    } else if (dow == 0 && mon == 9 && d >= 25 && h == 2 && daylightOffset_sec == 3600) {
      daylightOffset_sec = 0;
    }
    return daylightOffset_sec;
}

// Må kobles til bryter for å fungere skikelig
bool sjekkManuell_lux() {
  if (analogRead(I0_5) > 300) {
    return true;
  } else {
    return false;
  }
}

// Må kobles til bryter for å fungere skikelig
bool sjekkManuell_styring() {
  if (analogRead(I0_2) > 500) {
    return true;
  } else {
    return false;
  }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(const char *address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;  // LI, Version, Mode
  packetBuffer[1] = 0;           // Stratum, or type of clock
  packetBuffer[2] = 6;           // Polling Interval
  packetBuffer[3] = 0xEC;        // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123);  // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void stillKlokka() {
  // En funksjon som stiller klokka f.eks. ved strømutfall og/eller boot
  // Henter epoch-tid fra NTP-server med UDP
  Udp.begin(localPort);
  delay(1000);
  sendNTPpacket(timeServer);  // Egen funksjon som sender en "custom"-pakke til NTP-server
  // Venter på respons
  delay(1000);
  // Henter ut epoch-tid som ligger godt gjemt langt inne i responsen fra NTP-serveren
  if (Udp.parsePacket()) {
    Udp.read(packetBuffer, NTP_PACKET_SIZE);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long myepoch = secsSince1900 - seventyYears;
    rtc.setTime(myepoch);  // Setter rtc til ntp-tid ved boot
    Serial.println("Klokka er nå korrekt og viser: ");
    Serial.println(myepoch);
  }
}

void jegLever() {
  publiserTilstand();
}

bool sjekkTilstandLys() {
  if (analogRead(I0_3) > 300) {
    return true;
  } else {
    return false;
  }
}

void publiserTilstand() {
  // Lager JSON-objekt som skal sendes til MQTT
  StaticJsonDocument<200> veilysData;
  veilysData["epoch"] = rtc.getEpoch();
  veilysData["skapID"] = PLC_ID;
  veilysData["lux"] = sensor_lux;
  veilysData["status_lys"] = status_lys;
  veilysData["manuell_styring"] = manuell_styring;
  veilysData["dor_lukket"] = door_open;
  char meldingsobjekt[200];
  serializeJson(veilysData, meldingsobjekt);

  Serial.println(meldingsobjekt);
  client.publish(publishTopic, meldingsobjekt);
}

// **************************************************************************
// ************ Her kommer setup og loopene *********************************
// **************************************************************************

void setup() {
  Serial.begin(115200);
  // Starter Ethernet and UDP/NTP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Feilet i å konfigurere ethernet med DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Finner ikke ethernet-shield.  Kan ikke fortsette uten hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernetkabel er ikke tilkoblet");
    } else {
      Serial.println("Noe gikk veldig galt!! Kjører reset om 10 sekunder");
      delay(10000);
      ESP.restart();
    }
  }

  delay(3000);
  stillKlokka();
  delay(1000);
  Ethernet.maintain();

  // Initiering av MQTT (PubSub-klienten)
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  delay(2000);  // Venter i 2 sekunder slik at klokka rekker å bli satt for å unngå av/på av utganger ved boot

  // Sjekker om klokka er stilt riktig. Hvis ikke venter den 1 minutt og prøver på nytt
  if (rtc.getYear() < 2023) {
    Serial.print("Venter 1 minutt på å stille klokka");
    delay(60000);
    stillKlokka();
  }
}

// Hovedløkke - kommunikasjon
void loop() {
  // Listen for mqtt message and reconnect if disconnected
  if (!client.connected()) {
    reconnect();
  }
  // Kall til PubSubClient som prosesserer innkomne og utgående meldinger
  client.loop();

  // Publiserer "Jeg lever melding" en gang i minuttet
  unsigned long now = millis();
  if (now - lastMsg > 900000) {
    lastMsg = now;
    publiserTilstand();
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
  sjekkDST(); // Sjekker og setter riktig sommer-/vintertid

  // Kalkulerer soloppgang og solnedgang for gitt dato
  calcSunriseSunset(year, month, day, latitude, longitude, transit, sunrise, sunset);

  // Sjekker og setter tilstanden til lysstyringen.
  isDark = sjekkIsDark(sunrise + utc_offset_hour, sunset + utc_offset_hour, rtc.getHour(true), rtc.getMinute());
  delay(1000);  // Vent 1 sekund for å sikre at isDark returnerer verdi


  // Her kan man lese inn sensorverdier og andre inputs
  sensor_lux = analogRead(I0_2);
  // int status_lys = analogRead(I0_3); // Leser av status på utgang for lysstyring

  // Sjekker _isDark_ om det er natt eller dag og tenner/slukker utgang
  if (!manuell_styring && !manuell_toppsystem) {
    if ( (!status_lys && isDark) || (status_lys && !isDark) ) {
      status_lys != status_lys;
      publiserTilstand(); // Publiserer tilstand på tenntidspunkt
    }
    if (isDark) {
      Serial.print("Automatisk styring aktiv - Lampestatus: PÅ!\n");
      digitalWrite(Q0_0, HIGH);
      digitalWrite(R0_8, HIGH);
      status_lys = true;
    } else {
      Serial.print("Automatisk styring aktiv - Lampestatus AV!\n");
      digitalWrite(Q0_0, LOW);
      digitalWrite(R0_8, LOW);
      status_lys = false;
    }
  }

  // Logging av status
  if (manuell_styring) {
    Serial.print("Manuell styring aktiv\n");
  }

  if (manuell_toppsystem) {
    Serial.print("Toppsystem styring aktivt!\n");
  }
  delay(5000);  // Vent 5 sekunder før neste sjekk
}