# AquaSync

Water tank level monitoring and automated motor control on an ESP32, with
status published over MQTT so the tank behaves like any other device in a
home automation setup (built and run here against [OpenHAB](https://www.openhab.org/)).

Built for a rooftop/underground water tank setup where the pump was
previously switched by hand -- either forgotten on (overflow, wasted
power) or forgotten off (tank runs dry, pump cavitates). AquaSync reads
the tank level continuously, decides when the motor should run, and pushes
that decision through a hysteresis band and a hard runtime cutoff so it
fails safe instead of failing dry.

## What it actually does

- Reads tank fill level from a waterproof ultrasonic sensor (JSN-SR04T),
  median-filtered across a burst of samples so a splashy surface doesn't
  cause relay chatter.
- Drives the pump contactor through a two-threshold hysteresis band
  (`LOW_THRESHOLD_PCT` starts it, `HIGH_THRESHOLD_PCT` stops it) instead
  of a single trigger point, so it isn't cycling the motor on and off
  every time the level crosses one line.
- Tracks how long the motor has been running. If it runs past
  `MAX_RUNTIME_MINUTES` without reaching the high mark, that's treated as
  a dry well / stuck valve / bad sensor rather than "keep pumping" --
  it cuts power and waits for a manual reset.
- Detects a faulted sensor (repeated out-of-range or no-echo reads) and
  refuses to run the motor blind rather than guessing.
- Publishes level, motor state, mode, and fault status over MQTT
  (retained), and takes mode/reset commands back the same way -- this is
  what lets it show up as normal items in OpenHAB rather than needing a
  bespoke integration.
- Local 0.96" OLED shows the same state on the device itself, and a
  physical button cycles auto/manual-on/manual-off so the tank is still
  operable by hand if WiFi or the broker is down.

## Hardware

ESP32 dev board, JSN-SR04T ultrasonic sensor, a relay module driving a
pump contactor (not the pump directly), an SSD1306 OLED, a push button,
and a buzzer/LED for faults. Full parts list and pin map in
[`docs/wiring.md`](docs/wiring.md).

## Architecture

```
WaterLevelSensor  --  median-filtered %, fault detection
MotorController   --  hysteresis + runtime safety cutoff, owns the relay
DisplayUI         --  renders current state to the OLED
MqttHandler       --  publishes state / receives commands, OpenHAB-facing
main.cpp          --  wires the above together, non-blocking main loop
```

Each piece only knows its own job -- the sensor doesn't know about
thresholds, the motor controller doesn't know about MQTT. `main.cpp` is
the only file that has an opinion about how they fit together, which is
what made it possible to bench-test the sensor and the motor logic
independently before wiring the whole thing to a real tank.

## Building it

This is a [PlatformIO](https://platformio.org/) project.

```bash
cp include/config.example.h include/config.h
# edit include/config.h with your WiFi + MQTT broker details and tank geometry
pio run --target upload
```

`include/config.h` is gitignored on purpose -- it's the only file that
holds anything specific to one install (WiFi credentials, broker
address, tank dimensions).

## OpenHAB integration

With the MQTT binding pointed at the same broker, `aquasync/level` maps
to a `Number` item, `aquasync/motor` and `aquasync/fault` to `String` or
`Switch` items, and `aquasync/cmd/mode` lets a rule or sitemap switch
override AUTO from OpenHAB the same way the physical button does locally.

## License

MIT, see [LICENSE](LICENSE).
