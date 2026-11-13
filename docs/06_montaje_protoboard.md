# Montaje en protoboard: base (ESP32) + 2 placas de desarrollo

Versión 1.0 · Noviembre 2026 · **Tabla de conexiones para las fases 3, 4 y 5 del plan de firmware**

> **Alcance.** Cómo cablear en protoboard la SuperMini (la base) y dos placas de desarrollo CH32V003 (los cajones A y B) para probar el bus DATA y SENSE antes de tener la PCB. Usa la asignación de pines de `02_diseno_hardware.md` §10 y `05_montaje_base.md` §2. Las pruebas están en `plan_firmware.md` (fases 3, 4 y 5).
>
> **No cubre** el LED RGB con sus MOSFET ni el final de carrera (fase 2): son pruebas de una sola placa.

---

## 1. Piezas

| Pieza | Cant. | Notas |
|---|---|---|
| ESP32-C3 SuperMini | 1 | Alimentada por su USB-C durante las pruebas |
| Placa de desarrollo CH32V003 (`Hardware/devboard_ch32v003`) | 2 | Cajón A (dirección 1) y cajón B (dirección 2) |
| WCH-LinkE | 1 | Para grabar; de una placa en una placa |
| Resistencia 1 kΩ | 8 | 2 en DATA (una por cajón) y 6 en SENSE (ver §2.3) |
| Resistencia 4,7 kΩ | 1 | Único pull-up de DATA |
| Diodo Schottky (1N5819 del pedido) | 1 | En el TX de la SuperMini |
| Cables Dupont | los necesarios | Los jumpers de dirección van a GND |

---

## 2. Conexiones

Todas las placas comparten **GND**. Cada red se monta una sola vez.

### 2.1 Alimentación

| Red | Conexión |
|---|---|
| GND | SuperMini GND ↔ GND de la devboard A ↔ GND de la devboard B ↔ GND del WCH-LinkE |
| 3V3 | SuperMini 3V3 → 3V3/VCC de las devboards A y B (consumo bajo en estas pruebas) |

> ⚠️ **Una sola fuente por placa.** Las devboards tienen su propio LDO y un conector PWR_5V (J4) con un puente VDD_SEL (JP1). No he podido comprobar qué hace ese puente: antes de alimentar una devboard desde el 3V3 de la SuperMini, mira en `devboard_ch32v003.kicad_sch` qué conecta cada posición, y no conectes a la vez J4 y el 3V3 externo.
>
> **Al grabar,** el WCH-LinkE solo lleva GND y SWIO si la placa ya está alimentada por la SuperMini (`plan_firmware.md` §1.1).

### 2.2 Bus DATA

| Desde | Hasta | Componente |
|---|---|---|
| SuperMini **GPIO20 (RX)** | DATA | Directo |
| SuperMini **GPIO21 (TX)** | DATA | **Schottky**: la banda (cátodo) hacia GPIO21 |
| SuperMini 3V3 | DATA | **4,7 kΩ** (único pull-up del sistema) |
| Devboard A **PD5** | DATA | 1 kΩ |
| Devboard B **PD5** | DATA | 1 kΩ |

> ⚠️ **Sentido del diodo.** El dibujo ASCII de `05_montaje_base.md` §3 puede leerse al revés. Vale el texto: la banda va hacia GPIO21, de modo que el TX solo puede tirar de la línea a 0 y nunca empujarla a 3,3 V (bus open-drain).

En el firmware de la base, el UART del bus es `Serial1.begin(115200, SERIAL_8N1, 20, 21)` (RX = 20, TX = 21). Con `ARDUINO_USB_CDC_ON_BOOT=1`, `Serial` sigue saliendo por el USB-C.

### 2.3 SENSE

Cada unión lleva **1 kΩ a cada lado** (`plan_firmware.md` §5.1): pin → 1 kΩ → cable → 1 kΩ → pin.

| Unión | Un lado | Otro lado |
|---|---|---|
| Base → cajón A | SuperMini **GPIO1** (SENSE R) | Devboard A **PA1** (SENSE L) |
| Cajón A → cajón B | Devboard A **PA2** (SENSE R) | Devboard B **PA1** (SENSE L) |

Dejan sin conectar: GPIO0, GPIO3 y GPIO10 de la base, y PD2 y PD3 de las dos devboards. Se usan en la prueba 4 de `plan_firmware.md` §5.0 (independencia de caras): deben seguir en 1 cuando actúa PA1/PA2.

Todos los SENSE son entradas con pull-up interna. Nunca se fuerza un 1.

### 2.4 Dirección

Cierre el puente = bit a 1 (`02_diseno_hardware.md` §4.1). En la protoboard, un cable a GND.

| Placa | Dirección | Cable a GND en |
|---|---|---|
| Devboard A | 1 | PC0 |
| Devboard B | 2 | PC1 |

Usa solo **PC0–PC4**. PD0 queda libre (en el diseño v2 es el LED azul).

**Prueba de dirección duplicada** (fase 4): pon PC0 a GND en las dos placas.

---

## 3. Diagrama

```
  SuperMini (base)                Devboard A (dir 1)         Devboard B (dir 2)
  ───────────────                 ──────────────────         ──────────────────
  3V3 ──┬── 4,7k ──┐
        │          │
  GPIO21 ◄|─────── DATA ═══╤═══ 1k ── PD5          ┌─── 1k ── PD5
   (banda a GPIO21)        ╰═════════════════════════╯
  GPIO20 ───────── DATA

  GPIO1 ─1k─╌╌╌╌╌╌╌╌╌╌╌╌╌╌─1k─ PA1(L)
                              PA2(R) ─1k─╌╌╌╌╌╌╌╌╌╌╌╌╌╌─1k─ PA1(L)
  GND ═════════════════════ GND ═══════════════════════ GND ═══ GND WCH-LinkE
  3V3 ───────────────────── 3V3 ═══════════════════════ 3V3
                         PC0 ── GND                 PC1 ── GND   (direcciones 1 y 2)
```

---

## 4. Orden de montaje

1. **Solo la SuperMini.** Prueba su USB-C y el monitor serie. Aún sin cajones.
2. **Alimentación y GND** de las dos devboards, sin las líneas de señal.
3. **Grabar cada devboard** (una cada vez) con el firmware de `plan_firmware.md` fase 1.
4. **Bus DATA** y la placa A (fase 3). Después la B (fase 4).
5. **SENSE** (fase 5, empezando por la §5.0).

---

## 5. Comprobaciones antes de enchufar

Con el multímetro, **sin alimentar**:

| Medida | Resultado esperado |
|---|---|
| 3V3 – GND | Alta resistencia (sin cortocircuito) |
| DATA – 3V3 | ≈ 4,7 kΩ |
| DATA – PD5 de cada devboard | ≈ 1 kΩ |
| GPIO1 de la base – PA1 de la devboard A | ≈ 2 kΩ |
| PA2 de la devboard A – PA1 de la devboard B | ≈ 2 kΩ |
| Cada SENSE – DATA | Sin conexión |
| Cada SENSE – otro SENSE | Sin conexión (salvo la unión de arriba) |

Si una medida falla, no enchufes nada.

---

## 6. Qué prueba cada fase

| Fase | Montaje | Qué se valida |
|---|---|---|
| 3 | Base + A | `PING`, `SET_LED`, `GET_STATE`; half-duplex y eco; `stress` sin errores |
| 4 | Base + A + B | Dos cajones en el bus; dirección duplicada (CRC incorrecto) |
| 5.0 | A + B | Capa eléctrica de SENSE: niveles, open-drain, «hola» al encender |
| 5.1–5.3 | Base + A + B | `SENSE_DRIVE`/`SENSE_QUERY` por DATA y mapa de posiciones |

Analizador lógico (fase 3, `plan_firmware.md` §3.4): DATA, PC6 (LED), y SENSE L y R de una devboard.

---

## 7. Diferencias con la PCB

- Aquí **cada devboard lleva su LDO** (XC6206 propio); en la PCB real, cada módulo lo lleva.
- Las devboards no llevan los 4 box header: las uniones de SENSE y DATA son cables Dupont, no latiguillos de 6 hilos.
- No se prueban los 5 V del bus ni el USB-C de la base (`05_montaje_base.md` §7).
- **1 kΩ en DATA y SENSE con 5 módulos** (`03_diseno_pcb.md` §9, punto 5) no se puede validar con dos placas.

---

## 8. Pendiente

- **Unificado con `plan_firmware.md`:** el bus va en GPIO20 (RX) y GPIO21 (TX), como en `02`, `04` y `05`; el LED es RGB con tres MOSFET (PC6, PC7, PD0) y la dirección tiene 5 bits (PC0–PC4).
- **Los docs dicen que GPIO5 y GPIO6 son pines de strapping** (`02_diseno_hardware.md` §10, `05_montaje_base.md` §2, `bom_5_cajones.md`). Según mi conocimiento, en el ESP32-C3 los de strapping son GPIO2, GPIO8 y GPIO9, y GPIO4–GPIO7 son de JTAG. No lo he comprobado con la datasheet: confírmalo. Si es así, GPIO20/21 se mantienen, pero GPIO8 (LED de estado) y GPIO9 (BOOT) sí merecen cuidado.
- **`plan_cajonera_modular.md` sigue desfasado** (LED WS2812/SK6812, 6 bits de dirección, direcciones 1–63). No lo he tocado; el diseño vigente es `02_diseno_hardware.md`.
