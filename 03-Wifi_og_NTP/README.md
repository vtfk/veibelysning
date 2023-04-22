# Tilkobling til WiFi og henting av tid og dato fra NTP-server

For å kjøre dette eksempelet trengs det kun PLC som er tilkoblet datamaskin med USB-kabel.

Programmet kobler seg opp på trådløst nett og henter ned riktig dato/tid fra en [NTP-server](http://www.ntp.org/ntpfaq/NTP-s-def/)

## Utstyr og oppsett
* 1 stk industrial shields PLC 19R
* 1 stk USB-kabel
* Arduino IDE v. 2.x
* v.2.0.7 av industrialshields-esp32 board-biblioteket.

Se mer informasjon i kodekommentarene
  
## Om programmet
  
For at programmet skal fungere må du opprette en fil som heter config.h. Denne må inneholde SSID og passord til det trådløse nettet som skal tilkobles.

I tillegg ligger det miljøvariabler (SSID og PASSORD) i en egen fil som heter config.h. Denne filen må ligge i samme mappe som programmet. 

Slik må config.h se ut:

```
#define SSID "<navn/SSID på det trådløse nette>";
#define PASSWORD "<Passord til det trådløse nettet>";
```