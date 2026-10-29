# Wiring

## Bill of materials

| Part | Notes |
|---|---|
| ESP32 dev board | Also runs unchanged on ESP32-S3 dev boards |
| JSN-SR04T waterproof ultrasonic sensor | Mounted at the top of the tank, facing straight down at the water |
| 1-channel relay module (5V, opto-isolated) | Drives the pump's contactor coil -- see safety note below |
| SSD1306 0.96" OLED, I2C, 128x64 | Local status readout |
| Push button | Manual auto/manual/off mode toggle |
| Buzzer + LED | Fault/safety-trip alert |
| Resistor divider (1kΩ + 2kΩ) | Steps the sensor's 5V echo signal down to 3.3V for the ESP32 input |

## Pin map

See [`include/pins.h`](../include/pins.h) for the source of truth. Summary:

| Signal | ESP32 GPIO |
|---|---|
| Ultrasonic TRIG | 5 |
| Ultrasonic ECHO (via divider) | 18 |
| Relay control | 26 |
| Manual button (to GND, `INPUT_PULLUP`) | 27 |
| Fault LED | 25 |
| Buzzer | 33 |
| OLED SDA | 21 |
| OLED SCL | 22 |

## Safety note on the relay

The relay in this project switches a low-voltage contactor coil, **not**
the pump's mains supply directly. The pump itself is wired through the
contactor, which is rated for the motor's actual load and switched from
its own supply. If you're adapting this to switch mains directly through
the relay, that's a different (and much less forgiving) wiring job --
get someone qualified to check it before powering it up.

## Sensor mounting

`SENSOR_DEADBAND_CM` in `config.h` is the distance from the sensor face
to where you're calling the tank "100% full" -- it accounts for the
sensor's own mounting bracket and the minimum distance it can reliably
read at. Measure it once when you install the sensor and set it there;
guessing it produces a fill percentage that's off by a fixed offset
across the whole range.
