# Styring med PLC og NodeRED og MQTT

Info her

## Utstyr og oppsett
* 1 stk industrial shields PLC 19R
* 1 stk USB-kabel
* Arduino IDE v. 2.x
* v.2.0.7 av industrialshields-esp32 board-biblioteket.
* Om ønskelig: Potensiometer for å simulere en sensor
* Om ønskelig: En lysdiode for å indikere at utgang slår seg av og på.

Se mer informasjon i kodekommentarene
  
## Om programmet
Info kommer

```c++
#define SSID "<navn/SSID på det trådløse nette>";
```

## Oversikt over porter
Q0_0 - Utgang: Lysdiode som indikerer at utgang er på for testformål (0V - 5V)
R0_8 - Utgang: Rele som styrer lysene (0V - 250V)
I0_5 - Inngang: Bryter - Manuell lys av/på
I0_2 - Inngang: Bryter - Manuell styring av/på

## Tilpassing og justering til eget bruk

### Telemetri og dataoverføring
Info kommer

