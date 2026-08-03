# Freezanz level I/O board — rev A

Interface board for eight XKC-Y25-NPN capacitive level sensors, with the CJMCU
MCP23017 module on a socket so it stays swappable. Everything is through-hole:
the only thing needed from the fab house is the bare PCB.

- Board cut: **90 × 50 mm** (enclosure is 95 × 55, so there is margin all round)
- 2 layers, 1.6 mm
- 46 parts to fit, 5 optional footprints left unpopulated

## Files

| File | Contents |
|---|---|
| `freezanz-level-io.kicad_pro` | project — net classes Default 0.3 mm / Power 0.8 mm |
| `freezanz-level-io.kicad_sch` | full schematic, footprints already assigned |
| `freezanz-level-io.kicad_pcb` | board outline only, 90 × 50 on Edge.Cuts |
| `schema-rev-a.svg` | readable schematic, for review without KiCad |
| `gen_kicad.py`, `make_pcb.py` | generators — edit and re-run to rebuild |

## Getting started

Nothing in this project contains invented geometry. Symbols and footprints are
referenced by their standard KiCad library names and resolve against your own
installation:

1. **Eeschema** → `Tools → Update Symbols from Library`, select all. The cached
   symbol graphics are replaced by the real ones. Pin coordinates in the file
   already match the KiCad 10 library exactly, so no connection is lost.
2. **PCB editor** → `Tools → Update PCB from Schematic` (F8). All 51 footprints
   are imported from your libraries, stacked at the origin, ready to place.
3. Place and route. The board file carries only the outline.

No ground pour is defined. GND is the largest net (35 connections), so routing
it by hand is real work — a pour on B.Cu would absorb most of it, but that is
your call.

## MCP23017 module socket (CJMCU)

Two 2.54 mm rows on the right, one on the left. Measured: 9 pad positions
inclusive from the left row to the outer right row, so **20.32 mm** left-to-outer
and 17.78 mm left-to-inner.

| | inner right column | outer right column |
|---|---|---|
| 1 | VCC | GND |
| 2 | INTB | INTA |
| 3 | GPB0 | **GPA0 → IO1** |
| 4 | GPB1 | **GPA1 → IO2** |
| 5 | GPB2 | **GPA2 → IO3** |
| 6 | GPB3 | **GPA3 → IO4** |
| 7 | GPB4 | **GPA4 → IO5** |
| 8 | GPB5 | **GPA5 → IO6** |
| 9 | GPB6 | **GPA6 → IO7** |
| 10 | GPB7 | **GPA7 → IO8** |

Port A, the outer column, carries the level sensors. `J4` is a 2×10 with
odd/even numbering, so **odd pins are the outer column**: when placing it, keep
pin 1 on the side away from `J3`, and rotate it so pin 1 ends up at the top —
that makes the board read the same way up as the module silkscreen.

Left row `J3`: `A2 A1 A0 RESET SO CS SDA SCL GND VCC`. Pins 5 and 6 are SPI-only
and stay unconnected.

**A2/A1/A0 on the left row are the I²C hardware address inputs, not GPIO.** They
share names with the port A pins on the right row, which is confusing, but they
are unrelated pins. All three are tied straight to GND, setting the address to
0x20.

## Input — one IDC connector

A single 2×4 IDC header, extended 1:1 by ribbon cable from the `J3` expansion
header on the Freezanz board. Pinout per the reverse-engineering README rev 0.6;
IDC zig-zag numbering, odd pins on one row, even on the other.

| Pin | Net | Path on the Freezanz board |
|---|---|---|
| 1 | `+12V_RAW` | straight off rail P+ |
| 2 | `+12V_FILT` | through a 0.2 Ω sense/jumper resistor |
| 3 | `+3V3` | straight off the ESP32 3.3 V pin |
| 4 | `GND` | — |
| 5 | `SDA` | GPIO21 through R65, 120 Ω |
| 6 | `GPIO5` | through R36, 470 Ω — shared with the P1 sensor input |
| 7 | `SCL` | GPIO22 through R66, 120 Ω |
| 8 | `GPIO15` | through R89, 470 Ω — boot-strapping pin |

SDA and SCL already carry 120 Ω in series and 3.3 kΩ pull-ups (R452/R455,
enabled by powering pin 3) on the Freezanz side, which is why R1/R2 here stay
unpopulated. GPIO5 and GPIO15 are ESP32 strapping pins that must sit HIGH at
reset, so anything connected to `J8`/`J9` must not hold them low during boot.

`JP2` selects which 12 V feeds the board, centre pin as the output. The 0.2 Ω is
almost certainly a sense shunt rather than a filter, so do not parallel the two
rails; the jumper keeps both options open until that is confirmed.

## Sensor channel (×8)

XKC-Y25-NPN, 4-pin flat connector:

| Pin | Wire | Net |
|---|---|---|
| 1 | black | `SENS_M` — mode select |
| 2 | blue | GND |
| 3 | yellow | signal out |
| 4 | brown | +12V |

```
          +3V3
            |
          [10k]  R21..R28
            |
XH 4p       |          MCP23017
 1 M -------|--------- SENS_M --- JP1 --- GND
 2 GND      |
 3 SIG -[470R]----------.-------- GPA0..GPA7
        R11..R18
 4 +12V
```

- **470R in series** is the protection that was missing. If the yellow wire ever
  sees 12 V, current into the MCP pin stays under 20 mA instead of destroying it.
  With the sensor's NPN conducting the pin sits around 0.35 V, comfortably inside
  the MCP23017 V_IL of 0.66 V.
- **10k to 3V3** is the pull-up, on the MCP side of the series resistor. The
  sensor output is open-collector, so without it the line floats when dry.
- **`SENS_M`** ties all eight mode pins together. `JP1` open (default) leaves the
  sensors in normal output mode; fitting the jumper pulls all eight to GND and
  inverts every output at once, with no rewiring.

## Power

```
J1.1 +12V_RAW  --+
                 |-- JP2 --> F1 --> +12V --+-- D1 P6KE15A --> GND
J1.2 +12V_FILT --+                         +-- C1 10uF (+ C2 pad, not fitted)
                                           +-- 8x sensor connectors
                                           +-- J8, J9
```

**F1 is a 500 mA fast 5×20 cartridge.** Real load is about 40 mA (8 × 5 mA), so
roughly 12× margin, and far below what the JST-XH connectors (3 A) or the traces
can carry. It is the only entry point for 12 V on this board, so it blows before
anything else is damaged.

**D1** is a unidirectional TVS: open circuit below 15 V, conducting above,
clamping transients to ground. It earns its place because the 12 V rail is shared
with the pump, and the motor inductance kicks back when the relay opens.

## Capacitors

A wire is not a short circuit — it has resistance and, more importantly,
inductance. When a load draws current suddenly the supply cannot deliver it
instantly through 20 cm of cable, so the local rail dips for a few microseconds.
A capacitor near the load is a local reservoir: it supplies the fast transient
and recharges slowly afterwards.

Two different values in parallel rather than one big one, because a real
capacitor is not ideal. An electrolytic has high capacitance but also ESR and
parasitic inductance, so above roughly 100 kHz it stops behaving like a
capacitor. A small ceramic has little capacitance but works into the tens of MHz.

| Ref | Value | Why |
|---|---|---|
| C1 | 10 µF / 25 V | reservoir on the 12 V rail, which feeds eight sensors and arrives over a cable |
| C2 | 100 nF | **footprint only, not fitted** — spare position; the level sensors switch rarely, so there is nothing fast to decouple |
| C3 | 10 µF / 16 V | reservoir on 3V3, which arrives from the ESP32 board over a cable |
| C4 | 100 nF | MCP23017 decoupling — the important one. Every I²C clock edge and internal transition draws a current spike in nanoseconds; without a local reservoir those spikes become supply noise, exactly the class of fault behind flaky reads. Place it as close to the socket VCC/GND pins as the layout allows: distance matters more than value. |

## Resistors other than the sensor chain

| Ref | Value | Why |
|---|---|---|
| R1, R2 | 4k7 | I²C pull-ups. I²C is open-drain: devices only pull low, so something has to return the line to 1. **Not fitted** — the RTC and the existing 2.58 k already do this, and more would drop the equivalent resistance too far. |
| R3 | 10k | RESET pull-up. RESET is active low; floating, it can pick up noise and reset the chip at random. The CJMCU module already has one, so this sits in parallel at about 5 k equivalent — harmless, and it covers a future module that lacks one. |
| R4, R5 | 10k | strapping pull-ups for GPIO5 / GPIO15. **Not fitted** — only needed if something is connected to `J8`/`J9` that could hold them low at boot. |

## Connectors

| Ref | Type | Pins |
|---|---|---|
| J1 | IDC 2×4 vertical | single input, 1:1 from the Freezanz J3 header |
| JP2 | header 1×3 | +12V_RAW / out / +12V_FILT — centre pin is the output |
| J11–J18 | XH 4p | M, GND, SIG, +12V — sensors S1…S8 |
| J21–J23 | XH 4p | +3V3, GND, SDA, SCL — I²C expansion, 3.3 V only |
| J8, J9 | XH 3p | +12V, GND, GPIO5 / GPIO15 — same pinout as P1/P2 |
| J3 / J4 | socket 1×10 / 2×10 | MCP23017 module |
| J6 | header 1×10 | GPB0…GPB7, GND, +3V3 — port B left free |
| J7 | header 1×6 | INTA, INTB, GND, +3V3, SDA, SCL |
| JP1 | header 1×2 | `SENS_M` → GND, open by default |
| H1–H4 | M3 | mounting |

## BOM

| Qty | Value | Refs | Footprint |
|---|---|---|---|
| 8 | 470R 1/4W | R11–R18 | R_Axial_DIN0207 P7.62 horizontal |
| 8 | 10k 1/4W | R21–R28 | R_Axial_DIN0207 P7.62 horizontal |
| 1 | 10k 1/4W | R3 | R_Axial_DIN0207 P2.54 vertical |
| 2 | 4k7 1/4W *(not fitted)* | R1, R2 | R_Axial_DIN0207 P2.54 vertical |
| 2 | 10k 1/4W *(not fitted)* | R4, R5 | R_Axial_DIN0207 P2.54 vertical |
| 1 | 10 µF 25 V electrolytic | C1 | CP_Radial_D6.3mm_P2.50mm |
| 1 | 10 µF 16 V electrolytic | C3 | CP_Radial_D6.3mm_P2.50mm |
| 1 | 100 nF ceramic | C4 | C_Disc_D5.0mm_W2.5mm_P5.00mm |
| 1 | 100 nF ceramic *(not fitted)* | C2 | C_Disc_D5.0mm_W2.5mm_P5.00mm |
| 1 | P6KE15A TVS | D1 | D_DO-15_P12.70mm_Horizontal |
| 1 | 5×20 holder + F500 mA | F1 | Fuseholder_Cylinder-5x20mm Stelvio-Kontek PTF78 |
| 1 | IDC header 2×4 vertical | J1 | IDC-Header_2x04_P2.54mm_Vertical |
| 11 | JST-XH 4p vertical | J11–J18, J21–J23 | JST_XH_B4B-XH-A |
| 2 | JST-XH 3p vertical | J8, J9 | JST_XH_B3B-XH-A |
| 1 | Female socket 1×10 | J3 | PinSocket_1x10_P2.54mm_Vertical |
| 1 | Female socket 2×10 | J4 | PinSocket_2x10_P2.54mm_Vertical |
| 1 | Header 1×10 | J6 | PinHeader_1x10_P2.54mm_Vertical |
| 1 | Header 1×6 | J7 | PinHeader_1x06_P2.54mm_Vertical |
| 1 | Header 1×3 + jumper | JP2 | PinHeader_1x03_P2.54mm_Vertical |
| 1 | Header 1×2 + jumper | JP1 | PinHeader_1x02_P2.54mm_Vertical |
| 4 | M3 screw | H1–H4 | MountingHole_3.2mm_M3 |

## Placement notes

Not placed — that is yours. Two things worth knowing before you start:

- A JST-XH 4-pin body is about 13 mm wide, so eight in one row need more than
  the board is wide. Two staggered columns of four fit in about 27 mm.
- KiCad THT footprints put their **origin on pad 1, not at the centre of the
  body**. `R_Axial_P7.62_Horizontal` runs from −1.05 to +7.62 in x, and
  `PinHeader_1x10` from −1.77 to +24.24 in y. Worth remembering when typing
  coordinates by hand.

## Routing notes

- SDA and SCL run from `J1` pins 5 and 7 to `J3` pins 7 and 8. Keep them short
  and away from the 12 V run — given the history of I²C timeouts on this bus,
  that is not fussiness.
- 12 V and 3V3 on the Power net class, 0.8 mm.
- `SIG1..SIG8` are slow and can go anywhere.
- GND has 35 connections and is by far the biggest net.

## From project to gerbers

```bash
kicad-cli pcb export gerbers --output gerbers/ freezanz-level-io.kicad_pcb
kicad-cli pcb export drill   --output gerbers/ freezanz-level-io.kicad_pcb
```

## Firmware note

With eight sensors on port A, the MCP pins in `freezanz.yaml` become `number: 0`
through `7`:

```yaml
  - platform: gpio
    name: "Repellent Level S1 (min)"
    id: repellent_level_s1
    pin:
      mcp23xxx: mcp23017_hub
      number: 0
      mode:
        input: true
        pullup: false     # 10k pull-up is on the board now
```

`pullup: false` because the 10k now lives on the PCB. And remember that
`inverted:` writes the IPOL register on the chip — the bug that already cost an
evening.
