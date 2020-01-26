# Temper-ESP8266
Temper is a compact, low power temperature sensor based on ESP8266 and SHT30 with large 13x7 pixel led display.  

- Low power, only consumes ~30-40uA while asleep.
- SHT30 provides very accurate, factory calibrated measurement capability.
- Super simple Web config ([see screenshot here](https://github.com/mcer12/Temper-ESP8266/wiki/Config-portal-&-OTA))
- Home Assistant compatible including auto-discovery, but can be used with any other platform supporting MQTT.
- 13x7 pixel led display
- TP4054 charging IC onboard for easy charging via USB
- Can be attached to a wall using magnets (see thingiverse for details).

## How does this thing work?
- After it's set up in the config portal, it periodically wakes up and publishes Temperature, Humidity and battery percentage through MQTT
- Pushing the reset button will force send current data and show Temperature on the display

![alt text](https://github.com/mcer12/Temper-ESP8266/raw/master/Images/with_cover_green.jpg)

![alt text](https://github.com/mcer12/Temper-ESP8266/raw/master/Images/pcb.jpg)

## What you need to make it yourself
- Hot air station / reflow oven (SHT30 has pins at the bottom, can't be hand-soldered)
- Components listed in "PCB and BOM" directory (lcsc and aliexpress links provided)
- Plastic case ([download on thingiverse](https://www.thingiverse.com/thing:4126709))
- Hot glue to fixate pcb in the enclosure
- Patience
- You can order it with most of the components pre-assembled from JLCPCB (you can find Pick&Place + BOM + Gerber in the above mentioned directory). Partially assambled pcbs will cost you little over $60 for 10pcs, see components information in "PCB and BOM" directory.

## Wiki
- [Flashing](https://github.com/mcer12/Temper-ESP8266/wiki/Flashing)
- [Config portal](https://github.com/mcer12/Temper-ESP8266/wiki/Config-portal-&-OTA)
- [Home Assistant](https://github.com/mcer12/Temper-ESP8266/wiki/Home-Assistant)
- [Notes and considerations](https://github.com/mcer12/Temper-ESP8266/wiki/Some-notes-and-considerations)
