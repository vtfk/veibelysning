# 💡 Prosjekt Veilys 

## ℹ️ Om prosjekt veilys
Veillys er et utviklingprosjekt med mål om å lage et komplett styringssystem for veibelysning. Prosjektet er et samarbeid mellom seksjon for samferdsel, miljø og mobilitet (SMM) og utviklingsavdelingen i Vestfold og Telemark fylkeskommune.

All kode og informasjon om prosjektet og de tekniske løsningene åpen og fritt tilgjengelig og lisensiert med Creative Commons 4.0 BY - SA Det btyr at du kan gjenbruke og modifisere koden som du selv ønsker så lenger du deler den videre og krediterer VTFK som opprinnelse.

Happy hacking!

## ⚙️ Teknisk utstyr

Prosjektet baserer seg på utsyr fra [Industrial Shields](http://industrialshields.com) som produserer blant annet [ESP32 PLC 19R](https://www.industrialshields.com/shop/product/034001000100-esp32-plc-19r-2905#attr=387,1558,2240,2316,3727,2317,3804) som er den viktigste komponenten i veilysstyringen. I tillegg brukes det en [strømforsyning](https://www.industrialshields.com/shop/product/is-ac24vdc7-5adin-din-rail-power-supply-ac-dc-180w-1-output-7-5a-at-24vdc-690?search=power+supply#attr=3651) fra samme produsent.

## 🛠️ Slik kommer du i gang 

Dette repoet inneholder flere mapper. Hver mappe er et "trinn" mot det fullstendige systemet. Hver mappe innholder en egen README-fil med relevant informasjon om hva koden gjør og hvordan den skal brukes og settes opp.

I testingen har det vist seg at siste versjon (2.1.x) av industrialshields-esp32 board-biblioteket har gitt en del kompileringsfeil. Bruk derfor v.2.0.7 av industrialshields-esp32.

Det er ikke alle USB-kabler som fungerer. Sørg for at du bruker en som er beregnet for dataoverføring.

|||
|----|----|
| [01 - Intro-IO_test](./01-Intro-IO_test/01-Intro-IO_test.ino) | Boilerplate og enkel funksjonalitetstest av innganger og utganger |
| [02 - Automatisk styring med klokke](./02-Styring_med_klokke/) | Styring av utganger med klokke |
| 03 - WiFi og NTP (kommer) | Tilkobling med WiFi og lesing av tid/dato fra NTP-server |
| 04 - Kobling til Azure IoT-central med WiFi (kommer) | Enkel tilkobling til Azure IoT-central |
| 05 - flere eksempler kode kommer...||




