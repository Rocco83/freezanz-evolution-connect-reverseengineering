Freezanz Zhalt Evolution Connect — Reverse Engineering

Avvertenza legale

Documento redatto esclusivamente a fini di studio, analisi tecnica e interoperabilità (art. 5 DLGS 518/92). L’autore declina ogni responsabilità per usi impropri o violazioni di licenze/marchi.

⸻

Scopo del documento

Raccogliere tutte le informazioni di pin-out, connettori e I/O presenti sulla scheda elettronica di Zhalt Evolution Connect per consentire lo sviluppo di firmware o software di controllo alternativi.

⸻

Stato del lavoro

Versione	Data	Autore	Note
0.3	2025-05-12	ChatGPT + 	Aggiornata mappatura pad ESP32-WROOM-32E e header di programmazione J1


⸻

Materiale di riferimento
	•	Foto PCB fronte/retro (vedi repo)
	•	Datasheet Espressif ESP32-WROOM-32E
	•	Manuale d’uso Freezanz (pendente)
	•	Foto prodotto: 


⸻

Mappa connettori esterni (in sintesi)

Rif.	Tipo	Pin	Segnale	Tensione	Descrizione
AC_IN	Fast-on 2 p	L / N	230 VAC	Rete in ingresso	
PUMP_OUT	Fast-on 2 p	PUMP L / PUMP N	230 VAC	Uscita pompa (relè)	
BAT	Fast-on 2 p	BAT P / BAT N	12 V DC	Alimentazione di backup	
P1	JST-XH 3 p	1 = GPIO5, 2 = GND, 3 = P+ (J9)	0-3.3 V	Funzione TBD — sensore o trigger esterno	
LED_EXT	JST-XH 3 p	TBD	0-3.3 V	Connettore striscia LED esterni (serig. “LEDs”)	

(altri connettori in raccolta — vedere sezione in dettaglio)

⸻

Connettore J1 – Header di programmazione ESP32

Pin J1	Segnale scheda	Collegare FTDI	Pad ESP32	Descrizione
1	BOOT / GPIO0	—	25	Tenere LOW all’accensione per entrare in bootloader
2	TX0 / GPIO1	RX FTDI	35	Console seriale 115 200 8N1
3	RX0 / GPIO3	TX FTDI	34	Console seriale
4	VCC 3 V 3	3V3 FTDI (≥ 500 mA)	—	Alimenta la logica durante il flash
5	EN / RESET	—	3	CHIP_EN, LOW ⇒ reset
6	GND	GND FTDI	—	Riferimento comune

Sequenza flash
	1.	Collegare FTDI (3 V 3) al J1.
	2.	Tenere BOOT e EN LOW, poi rilasciare EN→HIGH, quindi BOOT→HIGH.
	3.	Eseguire esptool.py --chip esp32 --baud 921600 write_flash 0x0 firmware.bin.

⸻

Pin-out ESP32-WROOM-32E (pad fisici ↔ funzioni scheda)

Pad #	GPIO	Funzione Freezanz	Direzione	Note
3	— (EN)	Reset (CHIP_EN)	I	Collegato a J1-5
4	IO36 (VP)	sensor_vp	I (ADC)	Solo input analogico; funzione ancora da identificare
5	IO34	— (N/C – footprint D5)	—	Non connesso
6	IO35	— (N/C – footprint D45)	—	Non connesso
7	IO39 (VN)	sensor_vn	I (ADC)	Solo input analogico; funzione ancora da identificare
8	IO32	SW3 (Start / Stop)	INPUT_PULLUP	Pulsante, LOW = pressed
9	IO33	SW2 (On / Off)	INPUT_PULLUP	Pulsante, LOW = pressed
10	IO25	LED D7 (Rosso)	O	Active HIGH
11	IO26	Buzzer	O (PWM)	Da confermare
13	IO14	—	—	Non connesso
17–22	(GPIO17-22)	—	—	Non connesse (N/C)
25	IO0	BOOT	I	J1-1
29	IO5	P1-1	TBD	Pin sinistro connettore P1
30	IO18	LED D6 (Verde)	O	Active HIGH
31	IO19	LED D3 (Blu)	O	Active HIGH
38	GND	—	—	Pad GND libero

—––|——|——————|———–|——|
| 3 | — (EN) | Reset (CHIP_EN) | I | Collegato a J1-5 |
| 4 | IO36 (VP) | sensor_vp | I (ADC) | Solo input analogico; funzione ancora da identificare |
| 5 | IO34 | — (N/C – footprint D5) | — | Non connesso |
| 6 | IO35 | — (N/C – footprint D45) | — | Non connesso |
| 7 | IO39 (VN) | sensor_vn | I (ADC) | Solo input analogico; funzione ancora da identificare |
| 8 | IO32 | SW3 (Start / Stop) | INPUT_PULLUP | Pulsante, LOW = pressed |
| 9 | IO33 | SW2 (On / Off) | INPUT_PULLUP | Pulsante, LOW = pressed |
| 10 | IO25 | LED D7 (Rosso) | O | Active HIGH |
| 11 | IO26 | Buzzer | O (PWM) | Da confermare |
| 13 | IO14 | — | — | Non connesso |
| 17–22 | (GPIO17-22) | — | — | Non connesse (N/C) |
| 25 | IO0 | BOOT | I | J1-1 |
| 29 | IO5 | P1-1 | TBD | Pin sinistro connettore P1 |
| 30 | IO18 | LED D6 (Verde) | O | Active HIGH |
| 31 | IO19 | LED D3 (Blu) | O | Active HIGH |
| 38 | GND | — | — | Pad GND libero |

—––|——|——————|———–|——|
| 3 | — (EN) | Reset (CHIP_EN) | I | Collegato a J1-5 |
| 5 | IO34 | — (N/C – footprint D5) | — | Non connesso |
| 7 | IO35 | — (N/C – footprint D45) | — | Non connesso |
| 8 | IO32 | SW3 (Start / Stop) | INPUT_PULLUP | Pulsante, LOW = pressed |
| 9 | IO33 | SW2 (On / Off) | INPUT_PULLUP | Pulsante, LOW = pressed |
| 10 | IO25 | LED D7 (Rosso) | O | Active HIGH |
| 11 | IO26 | Buzzer | O (PWM) | Da confermare │
| 13 | IO14 | — | — | Non connesso |
| 17–22 | Varie | — | — | Non connesse (N/C) |
| 25 | IO0 | BOOT | I | J1-1 |
| 29 | IO5 | P1-1 | TBD | Pin sinistro connettore P1 |
| 30 | IO18 | LED D6 (Verde) | O | Active HIGH |
| 31 | IO19 | LED D3 (Blu) | O | Active HIGH |
| 34 | IO3 | RX0 | I | J1-3 |
| 35 | IO1 | TX0 | O | J1-2 |
| 38 | GND | — | — | Non usato sulla scheda (pad GND libero) |

⸻

Indicatori luminosi e segnalazioni

LED	GPIO	Stato	Significato
D6 Verde	18	Solid	Nebulizzazione in corso
		Blink	Pulse cycle
D3 Blu	19	Solid	Wi-Fi attivo
		Blink	Wi-Fi connesso
D7 Bi-color	25 (Rosso) / 23 (Verde)	Rosso = allarme, Verde = pronto	(Verde su GPIO23 da verificare)

Pulsanti

Pulsante	GPIO	Uso	HW
SW2	33	ON/OFF	Pull-up interno
SW3	32	START/STOP	Pull-up interno


⸻

TODO a breve
	•	Confermare segnali LED verde D7 (GPIO23?) con oscilloscopio.
	•	Verificare il corretto funzionamento del buzzer (IO26).
	•	Mappare resto dei pad ESP32 (34–37) e bus I²C se presente.
	•	Test “Fase 2”: scheda programmata e rimontata nel sistema reale.
	•	Disegnare schema elettrico parziale (KiCad) per sezione logica.

⸻

Licenza

Creative Commons CC-BY-SA 4.0
© 2025 — Progetto comunitario Reverse Freezanz
