# Freezanz **Zhalt Evolution Connect** — Reverse‑Engineering README

# Attenzione: questo README è stato creato con ChatGPT e contiene degli errori, sopratutto tutti i voltaggi sono da verificare. Non è sicuro affidarsi a questo progetto, sopratutto per la parte elettrica.

## Avvertenza legale

*Documento redatto esclusivamente a fini di studio, analisi tecnica e interoperabilità (art. 5 DLGS 518/92). L’autore declina ogni responsabilità per usi impropri o violazioni di licenze/marchi.*

---

## Scopo del documento

Raccogliere e mantenere una descrizione completa di **pin‑out**, **connettori** e **I/O** della scheda elettronica **Zhalt Evolution Connect** per lo sviluppo di firmware/software alternativi.

---

## Stato del lavoro

| Versione | Data (EU/Rome) | Autore               | Note brevi                                                                   |
| -------- | -------------- | -------------------- | ---------------------------------------------------------------------------- |
| 0.5      | 2025‑05‑13     | ChatGPT + <tuo nome> | Sostituita tabella pin‑out ESP32 con layout basato su LME + mapping Freezanz |

---

## Materiale di riferimento

* Foto PCB fronte/retro (repository)
* **Datasheet ESP32‑WROOM‑32E** (Espressif)
* Articolo di riferimento pin‑out: "ESP32‑WROOM‑32 Pinout" — LastMinuteEngineers 🎓
* Manuale d’uso Freezanz (in attesa)
* Foto prodotto: ![Zhalt Evolution Connect](https://www.emporiodiantonio.com/cdn/shop/products/zhaltevolutionconnect_1024x1024@2x.jpg)

---

## Riepilogo connettori esterni
| Rif.            | Tipo / passo           | Pin ↓                        | Segnale         | Tensione       | Descrizione |
|-----------------|------------------------|------------------------------|-----------------|----------------|-------|
| **J1 (DC_IN)**  | Jack barrel (diameter TBD) | Tip = **V+**, Sleeve = GND | TBD (12 V ?)    | Ingresso alimentazione continua; negativo a massa, positivo protetto da **D1** e instradato al bus **P+** e **B+**, oltre a P1 via **H6** |
| **PUMP**        | Fast-on 2 p            | **PUMP P (P+) / PUMP N (P-)**          | TBD (VCC)       | Pompa nebulizzatore (?) usa la stessa VCC del jack J1, GND mediato da IO27 (da confermare) |
| **BATT**        | Fast-on 2 p            | **BATT P (B+)  / BATT N (B-)**                  | TBD (12 V ?)    | Backup battery; **B+** è solidale alla rail **P+** |
| **P1**          | JST-XH 3 p             | 1 = **VCC via H6**, 2 = GND, 3 = GPIO5 | TBD           | I/O esterno; **P+** su pin 1 tramite jumper **H6** |
| **J3**          | Pin header 10 × 1 (NP) | 8 = **IO15** (via R89 470 Ω) | TBD             | Porta espansione - non collegata|
| **J8 (LED_EXT)**| JST-XH 3 p             | TBD                          | TBD             | Connettore LED esterni - non collegato |

### Silkscreen power rail labels (bottom edge)
| Label serigrafia | Rail / Signal                                                                       |
|------------------|-------------------------------------------------------------------------------------|
| **P+**           | “PUMP P” – Positive supply rail (shared with **B+** and **J1 V+**)                   |
| **P-**           | “PUMP N” – Return/ground for pump                                                   |
| **B+**           | “BATT P” – Battery positive (internally tied to **P+**)                             |
| **B-**           | “BATT N” – Battery negative                                                         |
## IC29 — tentative identification

**Package**: likely SOIC‑8 or MSOP‑8, marking “940MI SJ6”.
The routing of **GPIO21 / GPIO22** (I²C) strongly suggests that **IC29 is an I²C real‑time clock (RTC)** — most plausibly the **Microchip MCP7940M**.

### Associated timing element

| Ref‑des | Mark code  | Type                           | Connection                                                        |
| ------- | ---------- | ------------------------------ | ----------------------------------------------------------------- |
| **Y2**  | “32 C 040” | 32.768 kHz tuning‑fork crystal | Pins X1/X2 of IC29 (through short tracks) — provides RTC timebase |

*(On this PCB the crystal footprint uses the “Y” prefix; only the two outer pads are connected, centre pad is mechanical/N‑C).*

### Hypotheses still to confirm

1. 8‑bit GPIO expander (PCA9554A) — less likely because VBAT present.
2. Low‑power ADC — also possible but VBAT pin matches RTC.
3. Power monitor.

> **Next steps**: scan I²C (expect address 0x6F), read time registers, remove Y2 to see clock stop.

---

## Tests to perform

| Rif.            | Tipo / passo             | Pin ↓                          | Segnale     | Tensione                                                                                                                                              | Descrizione |
| --------------- | ------------------------ | ------------------------------ | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| **J1 (DC\_IN)** | Jack barrel Ø2.1 mm      | Tip = **V+**, Sleeve = GND     | TBD (12 V?) | Ingresso alimentazione continua; negativo a massa, positivo protetto da **D1** (diodo anti‑inversione, 1.22 kΩ) e instradato al bus **P+** via **H6** |             |
| **AC\_IN**      | Fast-on 2 p              | L / N                          | 230 VAC     | Alimentazione di rete                                                                                                                                 |             |
| **PUMP\_OUT**   | Fast-on 2 p              | **PUMP P / PUMP N**            | 230 VAC     | Pompa nebulizzatore (relè)                                                                                                                            |             |
| **BAT**         | Fast-on 2 p              | **B+ / B-**                    | 12 V DC     | Backup battery; **B+ rail is hard‑wired to bus P+**                                                                                                   |             |
| **P1**          | JST-XH 3 p               | 1 = GPIO5, 2 = GND, 3 = **P+** | 0–3 V3      | I/O esterno; pin 3 condiviso con **P+** tramite jumper **H6**                                                                                         |             |
| **J3**          | Pin header (unpopulated) | 8 = **IO15** (via R89 470 Ω)   | 0–3 V3      | Porta di espansione riservata (non cablata di serie)                                                                                                  |             |
| **LED\_EXT**    | JST-XH 3 p               | TBD                            | 0–3 V3      | Connettore LED esterni                                                                                                                                |             |

### Silkscreen power rail labels (bottom edge)

| Label serigrafia | Rail / Signal                                          |
| ---------------- | ------------------------------------------------------ |
| **P+**           | "PUMP P" – Positive supply rail for pump & peripherals |
| **P-**           | "PUMP N" – Return/ground for pump                      |
| **B+**           | "BATT P" – Battery positive (internally tied to P+)    |
| **B-**           | "BATT N" – Battery negative                            |

## Connettore **J4** — Header di programmazione ESP32

| Rif.            | Tipo / passo        | Pin ↓                          | Segnale     | Tensione                                                                                                                                              | Descrizione |
| --------------- | ------------------- | ------------------------------ | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| **J1 (DC\_IN)** | Jack barrel Ø2.1 mm | Tip = **V+**, Sleeve = GND     | TBD (12 V?) | Ingresso alimentazione continua; negativo a massa, positivo protetto da **D1** (diodo anti‑inversione, 1.22 kΩ) e instradato al bus **P+** via **H6** |             |
| **AC\_IN**      | Fast-on 2 p         | L / N                          | 230 VAC     | Alimentazione di rete                                                                                                                                 |             |
| **PUMP\_OUT**   | Fast-on 2 p         | **PUMP P / PUMP N**            | 230 VAC     | Pompa nebulizzatore (relè)                                                                                                                            |             |
| **BAT**         | Fast-on 2 p         | **B+ / B-**                    | 12 V DC     | Backup battery; **B+ rail is hard‑wired to bus P+**                                                                                                   |             |
| **P1**          | JST‑XH 3 p          | 1 = GPIO5, 2 = GND, 3 = **P+** | 0–3 V3      | I/O esterno; pin 3 condiviso con **P+** tramite jumper **H6**                                                                                         |             |
| **LED\_EXT**    | JST‑XH 3 p          | TBD                            | 0–3 V3      | Connettore LED esterni                                                                                                                                |             |

### Silkscreen power rail labels (bottom edge)

| Label serigrafia | Rail / Signal                                          |
| ---------------- | ------------------------------------------------------ |
| **P+**           | "PUMP P" – Positive supply rail for pump & peripherals |
| **P-**           | "PUMP N" – Return/ground for pump                      |
| **B+**           | "BATT P" – Battery positive (internally tied to P+)    |
| **B-**           | "BATT N" – Battery negative                            |

## Connettore **J4** — Header di programmazione ESP32 **J4** — Header di programmazione ESP32 **J1** — Header di programmazione ESP32

| Pin J1 | Segnale scheda   | Collegare FTDI        | Pad modulo | Descrizione                         |
| ------ | ---------------- | --------------------- | ---------- | ----------------------------------- |
| 1      | **BOOT / GPIO0** | — (strap)             | Pin 25     | LOW all’accensione → bootloader     |
| 2      | **TX0 / GPIO1**  | RX FTDI               | Pin 35     | UART console 115 200 8N1            |
| 3      | **RX0 / GPIO3**  | TX FTDI               | Pin 34     | UART console                        |
| 4      | **VCC 3 V 3**    | 3 V 3 FTDI (≥ 500 mA) | —          | Alimenta la logica durante il flash |
| 5      | **EN / RESET**   | —                     | Pin 3      | CHIP\_EN — LOW ⇒ reset              |
| 6      | **GND**          | GND FTDI              | —          | Riferimento comune                  |

> **Procedura di flash**
>  1 Disconnettere la scheda da potenza.
>  2 Collegare FTDI 3 V 3.
>  3 Tenere BOOT e EN LOW, quindi rilasciare EN → HIGH, poi BOOT → HIGH.
>  4 `esptool.py --chip esp32 --baud 921600 write_flash 0x0 firmware.bin`

---

## Pin‑out **ESP32‑WROOM‑32E** (basato su LME + mapping Freezanz)

| Pin # | Pin Label | GPIO |**Freezanz Function** |Tipo |Note |Reason |Safe to use? |
| - | ---------- | ---- |------------------------- |--------------- |------ |----------- |------------ |
| 3 | **EN** | — |**Reset (CHIP_EN)** |Input |J1‑5 · LOW ⇒ reset   |Chip enable / strapping |⚠︎ |
| 4 | SENSOR\_VP | 36 |— |ADC input | TBD — SW probe   |Input‑only |✗ |
| 5 | SENSOR\_VN | 39 |— |ADC input | TBD — SW probe   |Input‑only |✗ |
| 6 | IO34 | 34 |N/C |ADC / input |Footprint **D5** (non popolato)   |Input‑only |✗ |
| 7 | IO35 | 35 |N/C |ADC / input |Footprint **D45** (non popolato)   |Input‑only |✗ |
| 8 | IO32 | 32 |**SW3** (Start/Stop) |Input (PULL‑UP) |Pulsante LOW ⇒ pressed   |— |✔︎ |
| 9 | IO33 | 33 |**SW2** (On/Off) |Input (PULL‑UP) |Pulsante LOW ⇒ pressed |— |✔︎ |
| 10 | IO25 | 25 |**LED D7** (Red) |Output |High = ON |— |✔︎ |
| 11 | IO26 | 26 |**Buzzer** |PWM Out |Frequenza TBD |— |✔︎ |
| 12 | IO27 | 27 |— |— |R24->R23->Q7->X20->P- |— |✔︎ |
| 13 | IO14 | 14 |— |— |Non usato |— |✔︎ |
| 14 | IO12 | 12 |N/C |— |Collegato a **R35** (N/C) |Strapping · LOW al boot |⚠︎ |
| 16 | IO13 | 13 |SPI flash interno |— |— |Strapping |⚠︎ | Non disponibile |
| 18 | SWP/SD3 | 10 |— |— |Non disponibile |SPI flash interno |✗ |
| 19 | SCS/CMD | 11 |— |— |Non disponibile |SPI flash interno |✗ |
| 20 | SCK/CLK | 6 |— |— |Non disponibile |SPI flash interno |✗ |
| 21 | SDO/SD0 | 7 |— |— |Non disponibile |SPI flash interno |✗ |
| 22 | SDI/SD1 | 8 |— |— |Non disponibile |SPI flash interno |✗ |
| 23 | IO15 | 15 |**Expansion line (J3‑8)** |I/O |Via **R91** (not fitted) & **C30→GND**, then through **R89 470 Ω** to connector **J3‑pin 8** (header currently unpopulated) – reserved for future external signal |Boot‑strapping pin (must be **HIGH** at reset) |⚠︎ |
| 24 | IO2 | 2 |**Line via R95 → D9** |I/O |Series **R95 150 Ω** to node with **D9** and pulldown **R76 8.3 kΩ**; D9 centre via **R8 10 kΩ** to GND. Purpose TBD (possible indicator or external sense). |Must be LOW at boot (strapping) |⚠︎ |
| 25 | IO0 | 0 |**BOOT (J1‑1)** |Input |LOW al reset ⇒ flash |Boot / flash mode |⚠︎ |
| 26 | IO4 | 4 |— |— |Non usato |— |✔︎ |
| 27 | IO16 | 16 |— |— |Non usato |— |✔︎ |
| 28 | IO17 | 17 |— |— |Non usato |— |✔︎ |
| 29 | IO5 | 5 |**P1‑1** |TBD input |Connettore P1 pin sinistro |Must be HIGH at boot |⚠︎ |
| 30 | IO18 | 18 |**LED D6** (Green) |Output |High = ON |— |✔︎ |
| 31 | IO19 | 19 |**LED D3** (Blue) |Output |High = ON |— |✔︎ |
| 33 | IO21 | 21 |**I²C to IC29** |I/O |Through **R455** (value? ≈290 Ω) then via network (series 290 Ω → node → **R69 220 Ω**) into IC29 pin ? (3rd from top‑left) |Default I²C **SDA** |✔︎ |
| 34 | RXD0 | 3 |**UART RX0 (J1‑3)** |Input |115 200 8N1 console |UART / flashing |⚠︎ |
| 35 | TXD0 | 1 |**UART TX0 (J1‑2)** |Output |115 200 8N1 console |UART / flashing |⚠︎ |
| 36 | IO22 | 22 |**I²C to IC29** |I/O |Through **R452** (≈290 Ω) → shared node with IO21 → R69 220 Ω → IC29 |Default I²C **SCL** |✔︎ |
| 37 | IO23 | 23 |**LED D7** (Green) |Output |High = ON |— |✔︎ |

---

## Indicatori & pulsanti

### LED

| LED                | GPIO                    | Stato                           | Significato             |
| ------------------ | ----------------------- | ------------------------------- | ----------------------- |
| **D6** (Verde)     | 18                      | Solid                           | Nebulizzazione in corso |
|                    |                         | Blink                           | Pulse cycle             |
| **D3** (Blu)       | 19                      | Solid                           | Wi‑Fi attivo            |
|                    |                         | Blink                           | Wi‑Fi connesso          |
| **D7** (Bi‑colore) | 25 (Rosso) / 23 (Verde) | Rosso = allarme, Verde = pronto |                         |

### Pulsanti

| Pulsante | GPIO | Funzione   | Note            |
| -------- | ---- | ---------- | --------------- |
| **SW2**  | 33   | ON/OFF     | Pull‑up interno |
| **SW3**  | 32   | START/STOP | Pull‑up interno |

### Buzzer

| GPIO | Funzione     | Segnale           |
| ---- | ------------ | ----------------- |
| 26   | Piezo buzzer | PWM 2‑4 kHz (TBD) |

---

## Ponticelli / Jumper

| Jumper | Default           | Collega                  | Funzione                                                            |
|--------|-------------------|--------------------------|---------------------------------------------------------------------|
| **H6** | CHIUSO (fabbrica) | **P1-3 (P+)** ↔ **VCC**  | Collega il pin 3 del connettore P1 alla linea VCC (DC_IN/BATT P); aprirlo isola P1 dalla VCC esterna |

---

## Tests to perform

### IO2 — unknown D9 network

*(see above)*

---

### IO21 / IO22 — IC29 I²C peripheral

**Goal**
Confirm that **GPIO21 (SDA)** and **GPIO22 (SCL)** communicate with IC29 (marked "940MI SJ6") via the series resistor network R455/R452 and R69.

**Measurements**

* Verify pull-up levels on the shared node (expect ≈3.3 V through external or internal pull‑ups of IC29).
* Scan I²C bus (`Wire.scan()`) and note address(es). Compare with known parts (0x20‑0x27 GPIO expander, 0x48‑0x4F ADC, etc.).
* If bus is idle high and toggling during operation, capture transactions with logic analyser to decode function.
* Check IC29 supply pins with oscilloscope for switching (if DC‑DC controller) or static (if logic IC).

**Hypotheses**

| Possibility                       | Expected I²C address | Quick test                             |
| --------------------------------- | -------------------- | -------------------------------------- |
| GPIO expander (PCA9554A, TCA9555) | 0x20‑0x27            | Toggle LEDs via expander regs          |
| ADC (MCP3021)                     | 0x48‑0x4F            | Read 12‑bit value changing with sensor |
| Power monitor (INA219)            | 0x40‑0x45            | Inject load, look for current register |

Add result to README once confirmed.

---

## TODO

* [ ] Investigate **IO15 / J3‑8** expansion line: measure the default voltage, test with external pull‑down/up through R89, and determine intended peripheral use.
* [ ] SENSOR\_VP/VN: log ADC values in real operation.
* [ ] Confirm LED D7 green on GPIO23.
* [ ] Characterize PWM buzzer on GPIO26.
* [ ] Phase‑2 test: board re‑installed in unit.
* [ ] Draw partial schematic in KiCad.
* [ ] Document P1 signal P+ after trace.

