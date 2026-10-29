# ESP32 Modular Digital Clock — ESP-IDF / Wokwi

Implementation derived from the two user-provided HTML examples:
`wokwi_esp32_digital_clock_suite.html` and
`wokwi_esp32_digital_clock_dashboard.html`.

## Features

- DS1307 RTC on I2C `0x68`, BCD decoding and 12/24-hour handling
- SSD1306 OLED 128×64 on I2C `0x3C`, 5×7 font and framebuffer
- GPIO interrupt + FreeRTOS queue button input with debounce and long-press detection
- Alarm scheduler, buzzer tone, 10-second ringing timeout
- Stopwatch start/pause, 5 lap records, reset
- Wi-Fi `Wokwi-GUEST`, SNTP `pool.ntp.org`, writes Bangkok local time back to RTC
- RTC-based clock if Wi-Fi/NTP is unavailable
- Four display pages: CLOCK, ALARM, STOPWATCH, SET RTC TIME

## Pin mapping (uses Suite reference, not Dashboard mapping)

| Hardware | Pin | Notes |
|---|---|---|
| I2C SDA | GPIO21 | DS1307 and OLED |
| I2C SCL | GPIO22 | DS1307 and OLED |
| DS1307 power | 5V | Wokwi DS1307 pin is `5V` |
| OLED power | 3V3 | `board-ssd1306` |
| MODE | GPIO18 | Next screen |
| SET | GPIO19 | Alarm +5 min / RTC +1 min / lap in stopwatch |
| ALARM | GPIO4 | Toggle alarm or dismiss ringing |
| LAP | GPIO5 | Stopwatch start/pause; long press reset |
| Buzzer positive | GPIO25 | LEDC 2 kHz PWM |
| Buzzer negative | GND | Common ground |

Wokwi part IDs were corrected to official IDs:
`board-esp32-devkit-c-v4`, `wokwi-ds1307`, `board-ssd1306`.

## How to build with Docker (Fedora/Fish)

Extract the ZIP under your workspace. In a **Fedora host terminal**, change to the
`esp32-digital-clock` folder and run:


This mounts the current project into Docker at `/project` using `:Z` for SELinux
and builds with `espressif/idf:latest`. Unlike older examples, this multi-component
project must be built from its root, not from `main/`.

For a persistent interactive Bash session:

```bash
docker run --rm -it \
  -v "$PWD:/project:Z" -w /project \
  -u "$(id -u):$(id -g)" -e HOME=/tmp \
  espressif/idf:latest bash
# inside container:
idf.py build
```

## Wokwi simulation

For **Wokwi CLI or VS Code Wokwi extension**, the included `wokwi.toml` points to
`build/flasher_args.json`, which includes application + bootloader + partitions.

## Interaction guide

1. On the CLOCK screen, view RTC time/date and Wi-Fi/NTP status.
2. Press MODE to cycle CLOCK → ALARM → STOPWATCH → SET RTC TIME → CLOCK.
3. On ALARM, press SET to increase alarm time by five minutes, then ALARM to arm.
4. On STOPWATCH, press LAP to start/pause and SET to record a lap while running;
   long-press LAP for reset.
5. On SET RTC TIME, press SET to advance RTC by one minute. The next NTP sync
   may overwrite a manual adjustment.
6. If alarm rings, press ALARM to silence it. It also times out after 10 seconds.

## Known limitations

- No published Wokwi project URL or actual ESP-IDF firmware-build/runtime log yet.
- UI uses compact ASCII uppercase glyphs; no Thai text rendering.
- Button ISR queue is bounded; heavy bouncing may drop edges. Monitor logs or
  add a stress test for overflow before calling it production-ready.
- DS1307 uses a 5V rail per Wokwi pin layout; physical hardware can need
  proper 3.3V-safe I2C level shifting and external pull-ups.
- GPIO5 is an ESP32 strapping pin; do not hold the connected button during reset.
- Wi-Fi `Wokwi-GUEST` works in simulator only; real device requires SSID/password
  and likely different network setup. NTP may be unavailable on restricted networks.
- Includes per-feature host tests, not full FreeRTOS integration tests.

## References

- https://docs.wokwi.com/parts/wokwi-ds1307
- https://docs.wokwi.com/parts/board-ssd1306
- https://docs.wokwi.com/parts/wokwi-buzzer
- https://docs.wokwi.com/diagram-format
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_netif.html
