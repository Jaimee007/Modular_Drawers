# Montaje en protoboard: base (ESP32) + 2 TENSTAR

Versión 1.0 · Noviembre 2026 · **Tabla de conexiones para las fases 3, 4 y 5 del plan de firmware**

> **Alcance.** Cómo cablear en protoboard la SuperMini (la base) y dos placas **TENSTAR CH32V003** (los cajones A y B) para probar el bus DATA y SENSE antes de tener la PCB. Usa la asignación de pines de `02_diseno_hardware.md` §10 y `05_montaje_base.md` §2. Las pruebas están en `plan_firmware.md` (fases 3, 4 y 5) y los programas, con sus entornos, en `07_pruebas_banco.md`.
>
> **No cubre** el LED RGB con sus MOSFET ni el final de carrera (fase 2): son pruebas de una sola placa.

---

## 1. Piezas

| Pieza | Cant. | Notas |
|---|---|---|
| ESP32-C3 SuperMini | 1 | Alimentada por su USB-C durante las pruebas |
| TENSTAR CH32V003F4P6 (ver `07_pruebas_banco.md` §3.1) | 2 | Cajón A (dirección 1) y cajón B (dirección 2) |
| WCH-LinkE | 1 | Para grabar; de una placa en una placa |
| Resistencia 1 kΩ | 6 | 2 en DATA (una por cajón) y 4 en SENSE (2 por unión, ver §2.3) |
| Resistencia 4,7 kΩ | 1 | Único pull-up de DATA |
| Diodo Schottky (1N5819 del pedido) | 1 | En el TX de la SuperMini |
| Cables Dupont | los necesarios | Los jumpers de dirección van a GND |

---

## 2. Conexiones

Todas las placas comparten **GND**. Cada red se monta una sola vez.

### 2.1 Alimentación

| Red | Conexión |
|---|---|
| GND | SuperMini GND ↔ pad G de la TENSTAR A ↔ pad G de la TENSTAR B ↔ GND del WCH-LinkE |
| 3V3 | SuperMini 3V3 → pad **V** de las TENSTAR A y B (consumo bajo en estas pruebas) |

> ⚠️ **Una sola fuente por placa.** En la TENSTAR todos los pads **V** son un único raíl sin regulador: con USB-C está a ~5 V. En este montaje se alimentan **desde la SuperMini, sin USB-C en las TENSTAR**, para que sus GPIO estén a 3,3 V como los del ESP32-C3 (datos de una guía de un tercero; ver `07_pruebas_banco.md` §3.1).
>
> **Al grabar una TENSTAR:** con USB-C, solo SWD y G al WCH-LinkE, y **desconecta antes el pad V, DATA y SENSE de la SuperMini**. Sin USB-C, SWD, V y G. Detalle en `07_pruebas_banco.md` §3.1.

### 2.2 Bus DATA

| Desde | Hasta | Componente |
|---|---|---|
| SuperMini **GPIO20 (RX)** | DATA | Directo |
| SuperMini **GPIO21 (TX)** | DATA | **Schottky**: la banda (cátodo) hacia GPIO21 |
| SuperMini 3V3 | DATA | **4,7 kΩ** (único pull-up del sistema) |
| TENSTAR A **PD5** | DATA | 1 kΩ |
| TENSTAR B **PD5** | DATA | 1 kΩ |

> ⚠️ **Sentido del diodo.** El dibujo ASCII de `05_montaje_base.md` §3 puede leerse al revés. Vale el texto: la banda va hacia GPIO21, de modo que el TX solo puede tirar de la línea a 0 y nunca empujarla a 3,3 V (bus open-drain).

En el firmware de la base, el UART del bus es `Serial1.begin(115200, SERIAL_8N1, 20, 21)` (RX = 20, TX = 21). Con `ARDUINO_USB_CDC_ON_BOOT=1`, `Serial` sigue saliendo por el USB-C.

### 2.3 SENSE

Cada unión lleva **1 kΩ a cada lado** (`plan_firmware.md` §5.1): pin → 1 kΩ → cable → 1 kΩ → pin.

| Unión | Un lado | Otro lado |
|---|---|---|
| Base → cajón A | SuperMini **GPIO1** (SENSE R) | TENSTAR A **PA1** (SENSE L) |
| Cajón A → cajón B | TENSTAR A **PA2** (SENSE R) | TENSTAR B **PA1** (SENSE L) |

Dejan sin conectar: GPIO0, GPIO3 y GPIO10 de la base, y PD2 y PD3 de las dos TENSTAR. Se usan en la prueba 4 de `plan_firmware.md` §5.0 (independencia de caras): deben seguir en 1 cuando actúa PA1/PA2.

Todos los SENSE son entradas con pull-up interna. Nunca se fuerza un 1.

### 2.4 Dirección

Cierre el puente = bit a 1 (`02_diseno_hardware.md` §4.1). En la protoboard, un cable a GND.

| Placa | Dirección | Cable a GND en |
|---|---|---|
| TENSTAR A | 1 | PC0 |
| TENSTAR B | 2 | PC1 |

Usa solo **PC0–PC4**. PD0 queda libre (en el diseño v2 es el LED azul).

**Prueba de dirección duplicada** (fase 4): pon PC0 a GND en las dos placas.

---

## 3. Diagrama

```
  SuperMini (base)                TENSTAR A (dir 1)         TENSTAR B (dir 2)
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
2. **Alimentación y GND** de las dos TENSTAR, sin las líneas de señal.
3. **Grabar cada TENSTAR** (una cada vez), con el entorno que toque de `07_pruebas_banco.md`.
4. **Bus DATA** y la placa A (fase 3). Después la B (fase 4).
5. **SENSE** (fase 5, empezando por la §5.0).

---

## 5. Comprobaciones antes de enchufar

Con el multímetro, **sin alimentar**:

| Medida | Resultado esperado |
|---|---|
| 3V3 – GND | Alta resistencia (sin cortocircuito) |
| DATA – 3V3 | ≈ 4,7 kΩ |
| DATA – PD5 de cada TENSTAR | ≈ 1 kΩ |
| GPIO1 de la base – PA1 de la TENSTAR A | ≈ 2 kΩ |
| PA2 de la TENSTAR A – PA1 de la TENSTAR B | ≈ 2 kΩ |
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

Analizador lógico (fase 3, `plan_firmware.md` §3.4): DATA, y SENSE L y R de una TENSTAR.

---

## 7. Diferencias con la PCB

- Aquí las TENSTAR **no llevan regulador** y se alimentan desde el 3V3 de la SuperMini; en la PCB real, cada módulo lleva su LDO (XC6206) desde el bus de 5 V.
- Las TENSTAR no llevan los 4 box header: las uniones de SENSE y DATA son cables Dupont, no latiguillos de 6 hilos.
- No se prueban los 5 V del bus ni el USB-C de la base (`05_montaje_base.md` §7).
- **1 kΩ en DATA y SENSE con 5 módulos** (`03_diseno_pcb.md` §9, punto 5) no se puede validar con dos placas.

---

## 8. Pendiente

- **Unificado con `plan_firmware.md`:** el bus va en GPIO20 (RX) y GPIO21 (TX), como en `02`, `04` y `05`; el LED es RGB con tres MOSFET (PC6, PC7, PD0) y la dirección tiene 5 bits (PC0–PC4).
- **Strapping del ESP32-C3:** GPIO2, GPIO8 y GPIO9 (datasheet de Espressif). Los docs decían GPIO5 y GPIO6, que era un error y ya está corregido en `02_diseno_hardware.md` §10.
- **`plan_cajonera_modular.md`** se archivó en `docs/archivo/` con un aviso de que no es vigente; manda `02_diseno_hardware.md`.
