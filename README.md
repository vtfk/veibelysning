# 💡 Veilys 💡

## ℹ️ Om veilys
I dette erpoet finner du veiledninger og prototyping av utstyr og kode med mål om å lage et komplett styringssystem for veibelysning i Vestfold og Telemark. Prosjektet er et samarbeid mellom seksjon for samferdsel, miljø og mobilitet (SMM) og utviklingsavdelingen (BDK/TEK) i Vestfold og Telemark fylkeskommune.

All kode og informasjon om prosjektet samt de tekniske løsningene åpne og fritt tilgjengelige. De er lisensiert med Creative Commons 4.0 BY - SA som betyr at du kan gjenbruke og modifisere koden slik du selv ønsker forutsatt at du deler den videre med samme lisens og krediterer VTFK som opphav.

Ta gjerne kontakt om du har spørsmål eller lurer på noe!

Happy hacking!

## ⚙️ Teknisk utstyr

Prosjektet baserer seg på utsyr fra [Industrial Shields](http://industrialshields.com) som produserer blant annet [ESP32 PLC 19R](https://www.industrialshields.com/shop/product/034001000100-esp32-plc-19r-2905#attr=387,1558,2240,2316,3727,2317,3804) som er den viktigste komponenten i veilysstyringen. I tillegg brukes det en [strømforsyning](https://www.industrialshields.com/shop/product/is-ac24vdc7-5adin-din-rail-power-supply-ac-dc-180w-1-output-7-5a-at-24vdc-690?search=power+supply#attr=3651) fra samme produsent.

## 🛠️ Slik kommer du i gang 

Dette repoet inneholder flere mapper. Hver mappe er et "trinn" mot det fullstendige systemet. Hver mappe innholder en egen README-fil med relevant informasjon om hva koden gjør og hvordan den skal brukes og settes opp.

I testingen har det vist seg at siste versjon (2.1.x) av industrialshields-esp32 board-biblioteket har gitt en del kompileringsfeil. Bruk derfor v.2.0.7 av industrialshields-esp32.

Det er ikke alle USB-kabler som fungerer. Sørg for at du bruker en som er beregnet for dataoverføring.

| **Mappe** | **Beskrivelse**|
|----|----|
| [01 - Intro-IO_test](./01-Intro-IO_test/01-Intro-IO_test.ino) | Boilerplate og enkel funksjonalitetstest av innganger og utganger |
| [02 - Automatisk styring med klokke](./02-Styring_med_klokke/) | Styring av utganger med enkel klokke |
| [03 - WiFi og NTP](./03-Wifi_og_NTP/) | Tilkobling med WiFi og lesing av tid/dato fra NTP-server |
| [04 - Styring baser på sensordata](./04-Styring_med_sensor/) | Styring av utganger basert på verdier fra temperatursensor |
| [05 - Kobling til Azure IoT-central med WiFi](./05-Tilkobling_Azure_IoT-central/) | Enkel tilkobling til Azure IoT-central |
| [06 - Autonom_styring](./06-Autonom_styring/) | Autonom styring med astrour |




