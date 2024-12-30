## Howto

### ESPhome configuration

See [cc1101.yaml](cc1101.yaml) for configuration details, specifically for adding the appropriate buttons.

#### Top part

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

#### Fan part

```yaml
fan:
  - platform: "cc1101"
    speed_count: 3
    name: "Mechanical Fan"
    data_pin: D1
    map_off_to_zero: True
```

#### Button part

See exampel configuration file

## Known limitations

Doesn't appear to 'read' (listen) to (other) remotes sending commands (even though the code is there)

Just low/medium/high ... set speed_count to 4 if you need full
If you map off to zero, 'low' and 'off' are the same (i.e., mine only has 3 speeds - there is no off)
