# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repo is

Reverse-engineering and re-implementation of the "Freezanz Evolution Connect" mosquito-repellent
device: a custom ESP32 firmware replacement (ESPHome) plus original KiCad hardware add-on boards.
There is no build/test/lint tooling in the traditional sense — this is firmware YAML + KiCad project
files + documentation. Treat correctness as "matches the reverse-engineered hardware behavior",
not "passes a test suite".

## Repo layout

- `esphome-freezanz/` — the ESPHome YAML firmware that replaces the stock controller.
- `freezanz-level-io/` — KiCad project for a companion I/O expansion board (8-channel level-sensor
  interface, MCP23017-based) that plugs into the Freezanz mainboard's J3 expansion header.
- Root `README.md` — the reverse-engineering notes for the **stock Zhalt/Freezanz Evolution Connect
  mainboard** (the "upper level"), rev 0.6, mostly in Italian. This is a different subject from
  `freezanz-level-io/README.md`, which documents the add-on board — don't conflate them. It is the
  source of truth for: the ESP32-WROOM-32E pin map with Freezanz functions (GPIO numbers used
  throughout `freezanz.yaml`), the external connector tables (J1 DC_IN, PUMP, BATT, P1/P2 sensor
  inputs, J8 LED_EXT), the **J3 2×4 expansion header pinout that the level-io board plugs into**,
  IC29 = MCP7940M RTC at 0x6F with Y2 32.768 kHz crystal, the buzzer circuit (GPIO26 → R14/R13
  divider → Q6 BST82), LED/button mapping, and the H6 jumper.
  **It carries an explicit accuracy warning at the top**: written with AI assistance, contains
  errors, and *all voltages are unverified*. Treat its electrical claims as hypotheses to confirm
  against the hardware, not as fact — especially before acting on anything power-related.
- Root `PXL_*.jpg`, `11377_1078x1078.webp` — reference photos of the physical device/PCB used during
  reverse engineering.

## esphome-freezanz (firmware)

Single-file ESPHome config: `esphome-freezanz/freezanz.yaml`. Secrets come from `secrets.yaml`
(gitignored; copy `secrets.yaml.example` to create it — WiFi SSID/password only, referenced via
`!secret`).

**Common commands** (requires `esphome` installed, e.g. `pip install esphome` or `pipx install esphome`):
```bash
cd esphome-freezanz
cp secrets.yaml.example secrets.yaml   # first time only, then fill in real WiFi creds
esphome config freezanz.yaml           # validate YAML / config without compiling
esphome compile freezanz.yaml          # compile only
esphome run freezanz.yaml              # compile + flash (prompts for USB or OTA target)
esphome logs freezanz.yaml             # attach to logs (USB or API, depending on connectivity)
```
There's no unit test suite; validation is `esphome config` (catches YAML/schema errors) and
`esphome compile` (catches C++ lambda errors), plus flashing to real hardware for behavioral checks.

### Firmware architecture

Target: ESP32 (esp-idf framework, min chip rev 3.0). Everything lives in one YAML file, organized as:

- **I2C bus** (`bus_i2c`, GPIO21/22, low frequency — 10kHz, throttled down from 100kHz, likely to
  work around bus reliability issues noted in comments) shared by two devices:
  - **MCP23017 I/O expander** (`mcp23017_hub`) — provides extra GPIO for level sensors and switches
    beyond the ESP32's own pins. The `freezanz-level-io` board plugs its 8 level sensors into this
    chip's port A (`number: 0`–`7`).
  - **MCP7940M RTC** (address `0x6F`, global `rtc_address`) — no ESPHome component used; the RTC is
    driven by hand-rolled I2C read/write lambdas (`rtc_write_time`, `rtc_read_time`) using BCD
    conversion helpers, because ESPHome has no built-in MCP7940M support. Time syncs from SNTP on
    connect, then is written to the RTC (`rtc_write_time`) and read back on boot (`rtc_read_time`)
    to survive power loss without WiFi.
- **State machine for the nebulizer pump** lives in `script:` blocks (`pump_start`, `pump_stop`,
  `pump_force_stop`), gated by `globals: action_running` / `pre_cycle_running`. Pump start refuses to
  run if all repellent-level sensors read empty, runs a buzzer sequence first (`buzzer_start_sequence`,
  awaited via `wait_until`), then powers the pump relay for `pump_duration_number` seconds, with a
  `water_pressure_watchdog` script guarding against a dry/failed run.
- **Physical buttons** are plain `gpio` binary sensors with `on_click`/`on_press` handlers doing the
  actual logic in lambdas (not separate automations) — e.g. GPIO33 (ON/OFF) and GPIO32 (Start/Stop,
  short-press toggles the pump, the on/off button distinguishes short-press vs. >3s long-press via
  `on_click` min/max length to trigger a restart).
- **LEDs/buzzer are outputs driven from state**, not the other way around: `binary_sensor: platform:
  template` entities read back `switch`/`output` state via lambdas so the UI reflects true output
  state rather than duplicating logic.
- Known gotcha (documented in `freezanz-level-io/README.md`'s firmware note): `inverted:` on a
  MCP23017-backed `binary_sensor`/`switch` writes the chip's **IPOL register**, not just software
  inversion — mismatching this against the level-io board's hardware polarity caused a real bug.
- Security is intentionally deferred: `api:` and `ota:` password/encryption are commented out with
  `# TODO increase the security`.

## freezanz-level-io (hardware)

KiCad 10 project for an 8-channel capacitive-level-sensor interface board (XKC-Y25-NPN sensors) built
around a swappable CJMCU MCP23017 breakout module, connected to the main Freezanz board via a single
2×4 IDC ribbon from its `J3` expansion header. Full design rationale, pinouts, BOM, and routing notes
are in `freezanz-level-io/README.md` — read it before touching the schematic/PCB, it explains *why*
each part value/protection resistor is there (e.g. the 470Ω series resistors on sensor signals exist
specifically to protect the MCP23017 pins if 12V ever appears on the yellow sensor wire).

Files:
- `freezanz-level-io.kicad_pro` / `.kicad_sch` / `.kicad_pcb` — the KiCad project (format v10). The
  board is **fully placed and routed**: 51 footprints, 321 tracks, 2 vias, 0 airwires, Edge.Cuts is a
  single 93×53 mm rect. Single net class `Default`, 0.2 mm track / 0.2 mm clearance, all-through-hole
  so layer changes happen at THT pads. No copper pours. Note the README's "Getting started" section
  still describes the pre-placement state ("the board file carries only the outline") — that is stale.
- `schema-rev-a.svg` — rendered schematic for reviewing without opening KiCad.

**Common commands** (requires `kicad-cli`, part of a KiCad 8+ install):
```bash
cd freezanz-level-io
kicad-cli pcb export gerbers --output gerbers/ freezanz-level-io.kicad_pcb
kicad-cli pcb export drill   --output gerbers/ freezanz-level-io.kicad_pcb
```
Symbols/footprints reference standard KiCad libraries (not embedded), so opening the project requires
a local KiCad install with its standard libraries — first step in Eeschema is
`Tools → Update Symbols from Library` (select all) to resolve cached symbol graphics.

Editor lock files (`~*.lck`), backups (`*-backups/`), and OS cruft are gitignored — don't check those
in if KiCad regenerates them.
