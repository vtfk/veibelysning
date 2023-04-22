# Tilkobling til Azure IoT-centrak

Dette eksempelet demonstrerer en basistilkobling til Azure IoT-central og demonstrer kun enkel sending av kommando fra Azure til PLC og enkel telemteri av sensordata fra PLC til Azure.

Utgangspunktet for dette eksempelet er [ESP32-veiledningen til Azure SDK for C](https://github.com/Azure/azure-sdk-for-c-arduino/blob/main/examples/Azure_IoT_Central_ESP32/readme.md).

For at  kjøre eksempelet trengs det kun PLC som er tilkoblet datamaskin med USB-kabel.

Programmet bruker trådløst nett til å koble seg opp på Azure IoT-central

## Utstyr og oppsett
* 1 stk industrial shields PLC 19R
* 1 stk USB-kabel
* Arduino IDE v. 2.x
* v.2.0.7 av industrialshields-esp32 board-biblioteket.
* Potensiometer for å simulere en sensor
* Om ønskelig: En lysdiode for å indikere at utgang slår seg av og på.

Se mer informasjon i kodekommentarene
  
## Om programmet
Dette eksempelet er en modifisert utgave av [ESP32-veiledningen til Azure SDK for C](https://github.com/Azure/azure-sdk-for-c-arduino/blob/main/examples/Azure_IoT_Central_ESP32/readme.md), men er så tilpasset eget utstyr og hardware.

For at programmet skal fungere må det opprettes en fil som heter config.h. Denne må inneholde SSID og passord til det trådløse nettet som skal tilkobles.

I tillegg ligger det miljøvariabler (SSID, PASSORD og tilkoblingsinformasjon til Azure-central) i en egen fil som heter config.h. Denne filen må ligge i samme mappe som programmet og må inkluderes i hovedfilen ```05-Tilkobling_Azure_IoT-central.ino```.

Slik ser config.h ut:

```c++
#define SSID "<navn/SSID på det trådløse nette>";
#define PASSWORD "<Passord til det trådløse nettet>";

#define conf_SCOPE "<scope>"
#define conf_DEVICE_ID "<device-ID>"
#define conf_DEVICE_KEY "<device-key>"
```

## Tilpassing og justering til eget bruk
Info kommer

### Kjøre egne funksjoner
Info kommer

### Telemetri og dataoverføring
Info kommer

