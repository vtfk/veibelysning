/*
  Autonom styring av lys med en Industrial Shields PLC ESP32 19R+

  All kode må brukes på eget ansvar -  Se mer på https://github.com/vtfk
  CC BY SA - VTFK 2023

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
// #include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ESP32Time.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SolarCalculator.h>

// Lokale konfigfiler og miljøvariabler
#include "./config.h"

// Klokkkeinnstillinger
long gmtOffset_sec = 3600; // Tidssone + 1 time
int daylightOffset_sec = 3600; // Sommertid +1 time - Vintertid +0 timer
// const char *ntpServer = "no.pool.ntp.org"; // Klokkeserver

// NTP UDP - Må ryddes
unsigned int localPort = 8888;       // local port to listen for UDP packets
const char timeServer[] = "no.pool.ntp.org"; // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// Parametre som brukes av SolarCalcultaor (Astrouret)
double latitude = PLC_POS_LAT;
double longitude = PLC_POS_LONG;
int utc_offset = (gmtOffset_sec + daylightOffset_sec);
int utc_offset_timer = utc_offset / 3600;  // Endre navn på denne variabelen?
int year, month, day;
double transit, sunrise, sunset, c_dawn, c_dusk;

//ESP32Time-objekt: rtc;
ESP32Time rtc(utc_offset);

// Globale "tilstander"
bool isDark = false;
bool manuell_styring = false;
bool manuell_lux = false;
bool manuell_toppsystem = false;
bool door_open = false;

// MQTT-innstillinger
const char *mqttBroker = MQTT_BROKER;
const int mqttPort = MQTT_PORT;
const char *mqttUser = MQTT_USER;
const char *mqttPassword = MQTT_PASSWORD;

// MQTT topics og meldingsbuffer
const char *publishTopic = MQTT_PUBLISH_TOPIC;
const char *subscribeTopic = MQTT_SUBSCRIBE_TOPIC;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (5)
char msg[MSG_BUFFER_SIZE];

// Ethernet og udp-klienter;
EthernetClient ethClient;
PubSubClient client(ethClient);
// A UDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Update these with values suitable for your network.
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Denne skjønner jeg ikke helt. ....ennå.

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

// send an NTP request to the time server at the given address
void sendNTPpacket(const char * address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); // NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

// **************************************************************************
// **************************************************************************

void setup() {
  Serial.begin(115200);

    // start Ethernet and UDP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
      Serial.println("Noe gikk galt!!");
    }
  }
  
  Udp.begin(localPort);
  delay(1000);
  // Nå kommer klokkestæsj - Må ryddes

  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:
    //uint32_t myepoch = ((uint32_t)packetBuffer[40] << 24) | ((uint32_t)packetBuffer[41] << 16);
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    //Serial.print("Seconds since Jan 1 1900 = ");
    //Serial.println(secsSince1900);
    // now convert NTP time into everyday time:
    Serial.print("Epochtime = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long myepoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(myepoch);
    rtc.setTime(myepoch); // Setter rtc til ntp-tid ved boot

  }
  // wait ten seconds before asking for the time again
  delay(1000);
  Ethernet.maintain();

  // Slutt på klokkestæsj - Husk å rydde
  
  delay(3000);

  // Stiller klokka ved reboot - Må kjøres på nytt når man skal endre sommer-/vintertid
  // configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  //rtc.setTime(myepoch);

  // Initiering av MQTT (PubSub-klienten)
  client.setServer(mqttBroker, mqttPort);
  client.setCallback(callback);

  delay(2000); // Venter i 2 sekunder slik at klokka blir satt for å unngå av/på av utganger ved boot
}

// Hovedløkke - kommunikasjon
void loop() {
  Serial.println(Ethernet.localIP()); // print M-Duino ip
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
  delay(5000);
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
  isDark = sjekkIsDark(sunrise + utc_offset_timer, sunset + utc_offset_timer, rtc.getHour(true), rtc.getMinute());
  // manuell_styring = false; //sjekkManuell_lux(); // sjekkManuell_styring(); // Erstatt med true/false for å teste
  // manuell_lux = false;  // sjekkManuell_lux(); // Erstatt med true/false for å teste
  // manuell_toppsystem = false;

  delay(1000);  // Vent 1 sekund

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
  delay(5000);  // Vent 3 sekunder før neste sjekk
}