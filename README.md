# Temper-ESP8266
Temper is a compact, low power temperature sensor based on ESP8266 and SHT30 with large 13x7 pixel led display.  

- Low power, only consumes ~30-40uA while asleep.
- Super simple Web config ([see screenshot here](https://github.com/mcer12/Temper-ESP8266/wiki/Config-portal-&-OTA))
- Home Assistant compatible including auto-discovery, but can be used with any other platform supporting MQTT.
- 13x7 pixel led display
- TP4054 charging IC onboard for easy charging via USB

![alt text](https://github.com/mcer12/Temper-ESP8266/raw/master/Images/pcb.jpg)

## What you need to make it yourself
- Hot air station / reflow oven (SHT30 has pins at the bottom, can't be hand-soldered)
- Components listed in "PCB and BOM" directory
- Patience
- You can order it with most of the components pre-assembled from JLCPCB (you can find Pick&Place + BOM + Gerber  in the above directory)

## Wiki
- [Flashing](https://github.com/mcer12/Temper-ESP8266/wiki/Flashing)
- [Config portal](https://github.com/mcer12/Temper-ESP8266/wiki/Config-portal-&-OTA)
- [Home Assistant](https://github.com/mcer12/Temper-ESP8266/wiki/Home-Assistant)
- [Notes and considerations](https://github.com/mcer12/Temper-ESP8266/wiki/Some-notes-and-considerations)
