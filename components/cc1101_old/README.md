# Controlling ITHO Mechanical Ventilation

## Requirements

- An ESP8266 (or compatible)
- A C1101 board (including coil)
- Some tinkering [see Mechanics](#mechanics)

## How-to using Home Assistant

Ensure you have the hardware ready ([Mechanics](#mechanics)) and the `esphome` add-on installed. Next follow the ESPHome YAML configuration part.

When operational from a HA point of view:

- [ ] Power-off your mechanical fan unit
- [ ] Ensure you are able to send the join command from HA (i.e., have your mobile phone or tablet with you)
- [ ] Power-up the mechanical fan unit
- [ ] *Within* 30 seconds of powering the unit, send the join command
- [ ] The fan should respond by varying it's speed to indicate you joined successfully
- [ ] Test if you can send other commands.

## Current status:

 - Sending High/Medium/Low and the Timers work
 - Reading state and indicative time (not counting down (yet) - should HA or ESPhome do that?)

## ESPHome ITHO control
Trying to get ESPHome to mimic what is comprised in

 - https://github.com/compatech/esphome_c1101
 - https://github.com/jodur/ESPEASY_Plugin_ITHO/blob/master/_P145_Itho.ino
 - https://github.com/adri/IthoEcoFanRFT / https://github.com/supersjimmie/IthoEcoFanRFT


# Mechanics

## Wiring schema used

```
Connections between the CC1101 and the ESP8266 or Arduino:
CC11xx pins    ESP pins Arduino pins  Description
*  1 - VCC        VCC      VCC           3v3
*  2 - GND        GND      GND           Ground
*  3 - MOSI       13=D7    Pin 11        Data input to CC11xx
*  4 - SCK        14=D5    Pin 13        Clock pin
*  5 - MISO/GDO1  12=D6    Pin 12        Data output from CC11xx / serial clock from CC11xx
*  6 - GDO2       04=D2    Pin  2        Programmable output
*  7 - GDO0       ?        Pin  ?        Programmable output
*  8 - CSN        15=D8    Pin 10        Chip select / (SPI_SS)
```

# Dependencies

 - Home Assistant
 - ESPHome
 - https://github.com/CoMPaTech/esphome_itho

# ESPhome YAML configuration

See [cc1101.yaml](cc1101.yaml) for configuration details, specifically for adding the appropriate buttons.

## Top part, configure accordingly, the libaries **must** be there!


```yaml
esphome:
  name: fancontrol
  platform: ESP8266
  board: d1_mini_lite
  libraries:
    - SPI
    - Ticker

external_components:
  - source:
      type: git
      url: https://github.com/CoMPaTech/esphome_ct
    # refresh: 3600s # only set this briefly otherwise it just refreshes daily, see https://esphome.io/components/external_components#external-components-refresh for setting refresh too low
```

## Fan part

```yaml
fan:
  - platform: "cc1101"
    speed_count: 3
    name: "Mechanical Fan"
    data_pin: D1
    map_off_to_zero: True
    id: mech_fan
```

#### Button part

See example configuration file for details [cc1101.yaml](cc1101.yaml) on the buttons as instructed

```yaml
button:
  - platform: template
    name: "Send join command"
    on_press:
      then:
        - lambda: |-
            ((cc1101fan::CC1101Fan*)(&id(mech_fan)))->send_other_command(0);
  - platform: template
    name: "Run Timer for 10 Minutes"
    on_press:
      then:
        - lambda: |-
            ((cc1101fan::CC1101Fan*)(&id(mech_fan)))->send_other_command(1);
  - platform: template
    name: "Run Timer for 20 Minutes"
    on_press:
      then:
        - lambda: |-
            ((cc1101fan::CC1101Fan*)(&id(mech_fan)))->send_other_command(2);
  - platform: template
    name: "Run Timer for 30 Minutes"
    on_press:
      then:
        - lambda: |-
            ((cc1101fan::CC1101Fan*)(&id(mech_fan)))->send_other_command(3);
```

# Known limitations

Just low/medium/high ... set speed_count to 4 if you need full
If you map off to zero, 'low' and 'off' are the same (i.e., mine only has 3 speeds - there is no off)

## References (from archived repo)

ESPHome usable library for C1101/ITHO: https://github.com/CoMPaTech/esphome_itho

This is mostly from https://github.com/jodur/ESPEASY_Plugin_ITHO/

Although backtracked to (amongst other forks) : https://github.com/adri/IthoEcoFanRFT / https://github.com/supersjimmie/IthoEcoFanRFT



