# Montaje de la base en placa perforada

Versión 1.0 · Noviembre 2026 · **Guía de montaje del módulo base (controlador) sin PCB**

> **Alcance.** Cómo montar la base del prototipo en placa perforada de doble cara metalizada, con la SuperMini enchufada. Implementa `02_diseno_hardware.md` §10 y usa la asignación de pines de `04_diseno_software.md` §4.5.
>
> La base se pasa a PCB después de la fase H3, si el prototipo funciona.

---

## 1. Piezas

| Pieza | Cant. | Dónde |
|---|---|---|
| ESP32-C3 SuperMini | 1 | Ya la tienes |
| Tiras hembra 1×8, 2,54 mm (zócalo) | 2 | Farnell 2212S-08SG-85 (1593463) |
| Tiras macho 1×8 (soldadas a la SuperMini) | 2 | Cortadas de la SPC20484 (Farnell 1462888) |
| Placa perforada doble cara metalizada 5×7 cm | 1 | AliExpress |
| Box header 2×3 T821106A1S100CEU | 4 | Farnell 2215302 |
| Adaptador USB-C 16 pines a 2,54 mm (con 5,1 kΩ en CC1/CC2) | 1 | AliExpress o Nafcom (108.MD9245) |
| Resistencia 4,7 kΩ | 1 | Farnell 3496941 |
| Resistencia 1 kΩ | 4 | Farnell 4560128 |
| Diodo Schottky 1N5818 | 1 | Farnell 4050106 |
| Polyfuse 30R185UU (1,85 A) | 1 | Farnell 1822236 |
| TVS P6KE6V8A | 1 | Farnell 2981330 |
| Electrolítico 1000 µF 16 V (KSY108M016S1G5H15K) | 1 | Farnell 2901345 |
| Cerámico 100 nF, de patillas | 2 | Farnell (ver BOM) |
| Cable de 22 AWG (silicona) | 1 m | Farnell o casa |
| Cable de 26 AWG (silicona) | — | Para señales |

Los 5,1 kΩ de CC1 y CC2 vienen en el adaptador USB-C; si no, son dos más de Farnell (MFR-25FRF52-5K1).

---

## 2. Asignación de pines

Son los de `02_diseno_hardware.md` §10. No cambiar sin actualizar también el firmware.

| GPIO SuperMini | Función | A dónde va |
|---|---|---|
| 5V | Entrada de 5 V del bus (solo si la base se alimenta por el pin 5V) | Raíl +5V |
| GND | Masa común | Raíl GND |
| 3V3 | Salida del regulador de la SuperMini | Pull-up 4,7 kΩ y pad V de los módulos |
| GPIO21 (TX) | Bus DATA, a través del Schottky | Línea DATA |
| GPIO20 (RX) | Bus DATA, directo | Línea DATA |
| GPIO0 | SENSE L | Box header L, pin 4 |
| GPIO1 | SENSE R | Box header R, pin 4 |
| GPIO3 | SENSE U | Box header U, pin 4 |
| GPIO10 | SENSE D | Box header D, pin 4 |
| GPIO8 | LED de estado (en la SuperMini) | — |

Evitar GPIO2, GPIO5, GPIO6 y GPIO9.

---

## 3. Esquema de conexiones

```
  USB-C adaptador (VBUS, GND, CC1 5,1k, CC2 5,1k)
      VBUS ──[polyfuse 1,85 A]──┬──[TVS]── GND
                                 ├──[1000 µF]── GND
                                 └── RAIL +5V ──┬► pads 5V de los box header
                                                └► SuperMini pin 5V (obligatorio)

  SuperMini
      3V3 ──┬──[4,7 kΩ]──┐
            │            DATA ──► pin 3 de los box header (L, R, U, D)
            │             ▲
      GPIO21 (TX) ──►|──┘        (Schottky 1N5818, banda hacia GPIO21)
      GPIO20 (RX) ───────────────► DATA
      GPIO0  ──[1 kΩ]──► SENSE L ──► pin 4 de box header L
      GPIO1  ──[1 kΩ]──► SENSE R ──► pin 4 de box header R
      GPIO3  ──[1 kΩ]──► SENSE U ──► pin 4 de box header U
      GPIO10 ──[1 kΩ]──► SENSE D ──► pin 4 de box header D
      GND    ────────────────────── GND común
```

Notas del esquema:
- **Schottky:** la banda (cátodo) va hacia GPIO21. Sin el diodo, el TX empujaría la línea a 5 V si hubiera un módulo a 5 V.
- **Pull-up:** solo uno en todo el sistema, de DATA a 3V3 de la SuperMini.
- **Los cuatro SENSE** llevan cada uno su 1 kΩ en serie. No se unen entre sí.

---

## 4. Orden de montaje

1. **Zócalos.** Suelda las dos tiras hembra 1×8 en la perforada, separadas 15,24 mm entre filas (las dos filas de la SuperMini). Enchufa la SuperMini para alinear antes de soldar.
2. **Box headers.** Los cuatro, uno por lado. Muesca hacia el centro de la placa, como en los módulos.
3. **Entrada de alimentación.** Adaptador USB-C, polyfuse, TVS y electrolítico en el raíl de 5 V; el TVS y el electrolítico van a GND.
4. **Bus DATA.** Pull-up 4,7 kΩ, Schottky y los cuatro pines de DATA de los box header en la misma línea.
5. **SENSE.** Cuatro resistencias de 1 kΩ, cada una a su box header. Mide continuidad: cada SENSE debe ir solo a su conector.
6. **Cerámicos de 100 nF** cerca de la alimentación de la SuperMini.

---

## 5. Alimentación de la base

- **Un solo USB-C:** el adaptador de 16 pines alimenta el raíl de 5 V, y la SuperMini sale de ese mismo raíl por su pin **5V**. No hay segundo cable de alimentación.
- **Programar o ver el monitor:** desenchufa el adaptador y enchufa el USB-C de la SuperMini. **Nunca los dos a la vez**: son dos fuentes en la misma red.
- **Corriente:** el pin 5V de la SuperMini solo alimenta su regulador (unos 100–200 mA). Los módulos van por el raíl de 5 V, no por ese pin.
- **Para la prueba de un solo módulo**, la SuperMini puede alimentar al módulo por su 3V3 (sin USB-C en el módulo).

---

## 6. Comprobaciones antes de enchufar

Con el multímetro, sin nada alimentado:

| Medida | Resultado esperado |
|---|---|
| Continuidad GND del USB-C con GND de la SuperMini | Sí |
| VBUS del adaptador con los GPIO | No hay conexión directa: solo por el polyfuse |
| Cada SENSE con su box header | Solo con el suyo |
| DATA con los cuatro box header | Sí, la misma línea |
| CC1 y CC2 a GND | 5,1 kΩ cada una |
| 3V3 a GND | Alta resistencia (sin cortocircuito) |

Si una medida falla, no enchufes nada.

---

## 7. Pruebas en orden

1. **SuperMini sola:** su USB-C, el autotest de `bus.h` y el `[diag]` en el monitor.
2. **Un módulo** enchufado a la base por un box header, con la base alimentada por el 3V3 de la SuperMini. Espera los `[ack]` que ya funcionan.
3. **Bus en las dos direcciones:** la base manda PING, el módulo responde. Esta es la prueba de half-duplex (`04_diseno_software.md` §6, fase 3).
4. **Adaptador USB-C:** con los dos conectores y 5 V reales.

---

## 8. Lo que no cambia con respecto a la PCB futura

Los nombres de los pines, las resistencias y las posiciones de los box header se mantienen. Si la base se pasa a PCB, el esquema de §3 es la base del layout.
