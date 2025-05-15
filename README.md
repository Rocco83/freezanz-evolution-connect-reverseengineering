# Freezanz **Zhalt Evolution Connect** — Reverse‑Engineering README

## Avvertenza legale

*Documento redatto esclusivamente a fini di studio, analisi tecnica e interoperabilità (art. 5 DLGS 518/92). L’autore declina ogni responsabilità per usi impropri o violazioni di licenze/marchi.*

---

## Scopo del documento

Raccogliere e mantenere una descrizione completa di **pin‑out**, **connettori** e **I/O** della scheda elettronica **Zhalt Evolution Connect** per lo sviluppo di firmware/software alternativi.

---

## Stato del lavoro

| Versione | Data (EU/Rome) | Autore               | Note brevi                                                                   |
| -------- | -------------- | -------------------- | ---------------------------------------------------------------------------- |
| 0.5      | 2025‑05‑13     | ChatGPT + <tuo nome> | Sostituita tabella pin‑out ESP32 con layout basato su LastMinuteEngineer + mapping Freezanz |

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
| **PUMP**        | Fast-on 2 p            | **PUMP P (P+) / PUMP N (P-)**   | 12 VCC | Rele` pompa nebulizzatore usa la stessa VCC del jack J1, GND mediato da IO27 (da confermare) |
| **BATT**        | Fast-on 2 p            | **BATT P (B+)  / BATT N (B-)**  | 12 VCC | Backup battery (condensatore 16V 68000 uF); **B+** è solidale alla rail **P+** |
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

## IC29 — identification

**Package**: MCP7940M Low-Cost I2 C™ Real-Time Clock/Calendar with SRAM
The routing of **GPIO21 / GPIO22** (I²C) go to the **IC29 is an I²C real‑time clock (RTC)**

### Associated timing element

| Ref‑des | Mark code  | Type                           | Connection                                                        |
| ------- | ---------- | ------------------------------ | ----------------------------------------------------------------- |
| **Y2**  | “32 C 040” | 32.768 kHz tuning‑fork crystal | Pins X1/X2 of IC29 (through short tracks) — provides RTC timebase |

### Identification steps taken

```
>>> from machine import Pin, I2C
>>> import time
>>>
>>> i2c = I2C(0,            # use GPIO21/22 on the ESP32 board
...           scl=Pin(22),
...               sda=Pin(21),
...           freq=100_000)
>>>
>>> print("IC addresses:", [hex(a) for a in i2c.scan()])
IC addresses: ['0x6f']
>>> def bcd2dec(x): return (x >> 4) * 10 + (x & 0x0F)
...
>>> for _ in range(8):
...     sec = i2c.readfrom_mem(0x6F, 0x00, 1)[0] & 0x7F   # seconds reg., mask ST bit
...     print("second =", bcd2dec(sec))
...     time.sleep(1)
...
second = 38
second = 39
second = 40
second = 41
second = 42
second = 43
second = 44
second = 45
>>>
```


### Hypotheses still to confirm

1. 8‑bit GPIO expander (PCA9554A) — less likely because VBAT present.
2. Low‑power ADC — also possible but VBAT pin matches RTC.
3. Power monitor.

> **Next steps**: scan I²C (expect address 0x6F), read time registers, remove Y2 to see clock stop.

---

## Tests to perform

| Rif. | Tipo / passo | Pin ↓ | Segnale | Tensione     | Descrizione |
| --------------- | ------------------------ | ------------------------------ | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| **J1 (DC\_IN)** | Jack barrel Ø2.1 mm | Tip = **V+**, Sleeve = GND | TBD (12 V?) | Ingresso alimentazione continua; negativo a massa, positivo protetto da **D1** (diodo anti‑inversione, 1.22 kΩ) e instradato al bus **P+** via **H6** | |
| **AC\_IN** | Fast-on 2 p | L / N | 230 VAC | Alimentazione di rete     | |
| **PUMP\_OUT** | Fast-on 2 p | **PUMP P / PUMP N** | 230 VAC | Pompa nebulizzatore (relè)    | |
| **BAT** | Fast-on 2 p | **B+ / B-** | 12 V DC | Backup battery; **B+ rail is hard‑wired to bus P+**    | |
| **P1** | JST-XH 3 p | 1 = GPIO5, 2 = GND, 3 = **P+** | 0–3 V3 | I/O esterno; pin 3 condiviso con **P+** tramite jumper **H6**   | |
| **J3** | Pin header (unpopulated) | 8 = **IO15** (via R89 470 Ω) | 0–3 V3 | Porta di espansione riservata (non cablata di serie)    | |
| **LED\_EXT** | JST-XH 3 p | TBD | 0–3 V3 | Connettore LED esterni    | |

### Silkscreen power rail labels (bottom edge)

| Label serigrafia | Rail / Signal                                          |
| ---------------- | ------------------------------------------------------ |
| **P+**           | "PUMP P" – Positive supply rail for pump & peripherals |
| **P-**           | "PUMP N" – Return/ground for pump                      |
| **B+**           | "BATT P" – Battery positive (internally tied to P+)    |
| **B-**           | "BATT N" – Battery negative                            |

## Connettore **J4** — Header di programmazione ESP32

| Rif. | Tipo / passo | Pin ↓ | Segnale | Tensione | Descrizione |
| --------------- | ------------------- | ------------------------------ | ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ----------- |
| **J1 (DC\_IN)** | Jack barrel Ø2.1 mm | Tip = **V+**, Sleeve = GND | TBD (12 V?) | Ingresso alimentazione continua; negativo a massa, positivo protetto da **D1** (diodo anti‑inversione, 1.22 kΩ) e instradato al bus **P+** via **H6** | |
| **AC\_IN** | Fast-on 2 p | L / N | 230 VAC | Alimentazione di rete | |
| **PUMP\_OUT** | Fast-on 2 p | **PUMP P / PUMP N** | 230 VAC | Pompa nebulizzatore (relè) | |
| **BAT** | Fast-on 2 p | **B+ / B-** | 12 V DC | Backup battery; **B+ rail is hard‑wired to bus P+** | |
| **P1** | JST‑XH 3 p | 1 = GPIO5, 2 = GND, 3 = **P+** | 0–3 V3 | I/O esterno; pin 3 condiviso con **P+** tramite jumper **H6** | |
| **LED\_EXT** | JST‑XH 3 p | TBD | 0–3 V3 | Connettore LED esterni | |

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
| 4      | **VCC 3,3V**    | 3 V 3 FTDI (≥ 500 mA) | —          | Alimenta la logica durante il flash |
| 5      | **EN / RESET**   | —                     | Pin 3      | CHIP\_EN — LOW ⇒ reset              |
| 6      | **GND**          | GND FTDI              | —          | Riferimento comune                  |

> **Procedura di flash**
>  1 Disconnettere la scheda da potenza.
>  2 Collegare FTDI 3 V 3.
>  3 Tenere BOOT e EN LOW, quindi rilasciare EN → HIGH, poi BOOT → HIGH.
>  4 `esptool.py --chip esp32 --baud 921600 write_flash 0x0 firmware.bin`

---

## Pin‑out **ESP32‑WROOM‑32E** (basato su LastMinuteEngineer + mapping Freezanz)

| Pin # | Pin Label | GPIO |**Freezanz Function** |Tipo |Note |Reason |Safe to use? |
| - | ---------- | ---- |------------------------- |--------------- |------ |----------- |------------ |
| 1 | GND | — | — | — | Ground pad | Ground | ✔︎ |
| 2 | 3V3 | — | — | — | Main 3.3 V rail | Power supply | ✔︎ |
| 3 | **EN** | — | **Reset (CHIP_EN)** | Input | J1‑5 · LOW ⇒ reset | Chip enable / strapping | ⚠︎ |
| 4 | SENSOR\_VP | 36 | — | ADC input |  TBD — SW probe   | Input‑only | ✗ |
| 5 | SENSOR\_VN | 39 | — | ADC input |  TBD — SW probe   | Input‑only | ✗ |
| 6 | IO34 | 34 | N/C | ADC / input | Footprint **D5** (non popolato) | Input‑only | ✗ |
| 7 | IO35 | 35 | N/C | ADC / input | Footprint **D45** (non popolato) | Input‑only | ✗ |
| 8 | IO32 | 32 | **SW3** (Start/Stop) | Input (PULL‑UP) | Pulsante LOW ⇒ pressed   | — | ✔︎ |
| 9 | IO33 | 33 | **SW2** (On/Off) | Input (PULL‑UP) | Pulsante LOW ⇒ pressed | — | ✔︎ |
| 10 | IO25 | 25 | **LED D7** (Red) | Output | High = ON | — | ✔︎ |
| 11 | IO26 | 26 | **Buzzer** | PWM Out | Frequenza TBD | — | ✔︎ |
| 12 | IO27 | 27 | — | — | R24->R23->Q7->X20->P- | — | ✔︎ |
| 13 | IO14 | 14 | — | — | Non usato (VERIFICARE) | — | ✔︎ |
| 14 | IO12 | 12 | N/C | — | Collegato a **R35** (N/C) | Strapping · LOW al boot | ⚠︎ |
| 15 | GND | — | — | — | Non usato sulla scheda (pad GND) | Ground | ✔︎ |
| 16 | IO13 | 13 | **IPOTESI** SW2_DRV (via Q4, accensione via software) - routing R27 -> R9 -> Q4 (piedino destro) -> Q4 (piedino centrale) -> R100 -> R71(?) -> Piedino vicino D11 SW2 ON/OFF | — | — | Strapping | ⚠︎ | Non disponibile |
| 17 | SD2 | 9 | — | — | Non disponibile | SPI flash interno | ✗ |
| 18 | SWP/SD3 | 10 | — | — | Non disponibile | SPI flash interno | ✗ |
| 19 | SCS/CMD | 11 | — | — | Non disponibile | SPI flash interno | ✗ |
| 20 | SCK/CLK | 6 | — | — | Non disponibile | SPI flash interno | ✗ |
| 21 | SDO/SD0 | 7 | — | — | Non disponibile | SPI flash interno | ✗ |
| 22 | SDI/SD1 | 8 | — | — | Non disponibile | SPI flash interno | ✗ |
| 23 | IO15 | 15 | **Expansion line (J3‑8)** | I/O | Via **R91** (not fitted) & **C30→GND**, then through **R89 470 Ω** to connector **J3‑pin 8** (header currently unpopulated) – reserved for future external signal | Boot‑strapping pin (must be **HIGH** at reset) | ⚠︎ |
| 24 | IO2 | 2 | **Line via R95 → D9** | I/O | Series **R95 150 Ω** to node with **D9** and pulldown **R76 8.3 kΩ**; D9 centre via **R8 10 kΩ** to GND. Purpose TBD (possible indicator or external sense). | Must be LOW at boot (strapping) | ⚠︎ |
| 25 | IO0 | 0 | **BOOT (J1‑1)** | Input | LOW al reset ⇒ flash | Boot / flash mode | ⚠︎ |
| 26 | IO4 | 4 | — | — | Non usato | — | ✔︎ |
| 27 | IO16 | 16 | — | — | Non usato | — | ✔︎ |
| 28 | IO17 | 17 | — | — | Non usato | — | ✔︎ |
| 29 | IO5 | 5 | **P1‑1** | TBD input | Connettore P1 pin sinistro | Must be HIGH at boot | ⚠︎ |
| 30 | IO18 | 18 | **LED D6** (Green) | Output | High = ON | — | ✔︎ |
| 31 | IO19 | 19 | **LED D3** (Blue) | Output | High = ON | — | ✔︎ |
| 33 | IO21 | 21 | **RTC I²C SDA (addr 0x6F; clock ticking)** | I/O | **SDA** which keep the clock through IC29 & Y2. Path: **R455** (290 Ω) Ω -> **R69 220 Ω** -> IC29 3rd pin from top‑left | Default I²C **SDA** | ✔︎ |
| 34 | RXD0 | 3 | **UART RX0 (J1‑3)** | Input | 115 200 8N1 console | UART / flashing | ⚠︎ |
| 35 | TXD0 | 1 | **UART TX0 (J1‑2)** | Output | 115 200 8N1 console | UART / flashing | ⚠︎ |
| 36 | IO22 | 22 | **RTC I²C SCL (addr 0x6F; clock ticking)** | I/O | **SCL** which keep the clock through IC29 & Y2. Path: **R452** (290 Ω) -> **R69 220 Ω** → IC29 3rd pin from top‑left | Default I²C **SCL** | ✔︎ |
| 37 | IO23 | 23 | **LED D7** (Green) | Output | High = ON | — | ✔︎ |
| 38 | GND | — | — | — | Non usato sulla scheda (pad GND termico) | Ground | ✔︎ |

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
| **H6** | CHIUSO (fabbrica) | **P1-3 (P+)** ↔ **12 VCC**  | Collega il pin 3 del connettore P1 alla linea VCC (DC_IN/BATT P); aprirlo isola P1 dalla VCC esterna |

---

## Tests to perform

### IO13 — Virtual ON/OFF Driver

**Goal**
Verificare che **GPIO13** possa emulare la pressione del pulsante **SW2** (On/Off) tramite il transistor Q4 (BC817-25).

**Test procedure**
1. Inizializza i pin in MicroPython:
   ```python
   from machine import Pin
   import time

   sw2_drv = Pin(13, Pin.OUT)
   sw2_drv.value(0)                       # transistor inizialmente spento

   # GPIO33 normally read the ON/OFF (SW2) button state
   sw2_in = Pin(33, Pin.IN, Pin.PULL_UP)  # legge lo stato del pulsante SW2
   ```

2. Baseline: premi fisicamente SW2 e verifica in REPL:
   ```python
   print("SW2 manual press:", "LOW" if sw2_in.value()==0 else "HIGH")
   ```
   Deve stampare LOW quando premi.

3.	Emulazione: senza toccare il pulsante, esegui:
   ```python
   sw2_drv.value(1)
   time.sleep(0.2)
   sw2_drv.value(0)
   print("SW2 emulated press:", "LOW" if sw2_in.value()==0 else "HIGH")
   ```
   * Controlla che sw2_in.value() ritorni 0 (LOW) durante il pulse.
   * Verifica che l’unità si accenda/spegna come con una pressione fisica.

   3. (Opzionale) Collega un oscilloscopio alla linea SW2 per osservare il fronte netto grazie al feedback R9.

---

IO2 — RTC Square-Wave / Alarm Output

Goal
Confermare che GPIO2 riceva un segnale a 1 Hz dall’MFP/SQW pin dell’RTC MCP7940M attraverso la rete R95 → D9 → R76.

Test procedure
```python
from machine import Pin, I2C
import time

# 1) Inizializza I²C su GPIO21/22
i2c = I2C(0, scl=Pin(22), sda=Pin(21), freq=100000)

# 2) Abilita il square-wave 1 Hz (registro 0x07)
addr = 0x6F
ctrl = i2c.readfrom_mem(addr, 0x07, 1)[0]
ctrl |= 0x10      # SQWE = 1 (enable square-wave)
ctrl &= ~0x03     # RS = 00 (set 1 Hz)
i2c.writeto_mem(addr, 0x6F, bytes([ctrl]))

# 3) Conta i fronti su GPIO2
pulse = Pin(2, Pin.IN)
edges = 0
def on_rise(pin):
    global edges
    edges += 1

pulse.irq(trigger=Pin.IRQ_RISING, handler=on_rise)
time.sleep(5)
print("Edges in 5 s:", edges)  # Atteso ≈ 5
```
* Se conti circa 5 fronti in 5 s, il crystal e l’RTC funzionano.

---

Quando entrambi i test passano, l’emulazione del pulsante e il segnale RTC sono confermati.


---


## TODO

* [ ] Investigate **IO15 / J3‑8** expansion line: measure the default voltage, test with external pull‑down/up through R89, and determine intended peripheral use.
* [ ] SENSOR\_VP/VN: log ADC values in real operation.
* [ ] Characterize PWM buzzer on GPIO26.
* [ ] Phase‑2 test: board re‑installed in unit.
* [ ] Draw partial schematic in KiCad.
* [ ] Document P1 signal P+ after trace.

