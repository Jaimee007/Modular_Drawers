# Pruebas de banco

Versión 1.0 · Noviembre 2026 · **Qué programa prueba qué, cómo lanzarlo y cuándo está bien**

> **Alcance.** Índice de los programas de prueba de `firmware/`: su nombre, el entorno de PlatformIO, el cableado y el criterio de aceptación. El orden y las razones de cada fase están en `plan_firmware.md`; las conexiones del montaje, en `06_montaje_protoboard.md`.
>
> **Estado.** Los programas compilan. Ninguno ha sido verificado en hardware por quien lo escribió: el estado de cada prueba lo marca quien la ejecuta en el banco.

---

## 1. Convención de nombres

- **El nombre del programa, el del entorno de PlatformIO y el del archivo fuente son el mismo**: `src/<nombre>.c` (módulo) o `src/<nombre>.cpp` (base) y `[env:<nombre>]`.
- **Módulo (CH32V003)**: `phaseN_<función>`, con `N` = fase de `plan_firmware.md`. Carpeta `firmware/module/`.
- **Base (ESP32-C3)**: `base_<función>`. Carpeta `firmware/base_test/`.
- **Doc**: cada programa tiene una sección en este archivo con el mismo nombre como ancla.
- Cada archivo fuente empieza con una cabecera que cita su entorno y su sección.

Se lanza con `pio run -e <nombre> -t upload` desde la carpeta correspondiente. El entorno por defecto (`pio run -t upload` sin `-e`) es el último programa estable.

### Nombres antiguos

| Antes | Ahora |
|---|---|
| `module` (`main.c`) | `phase3_bus_tx` |
| `module_minichlink` | `phase3_bus_tx_minichlink` |
| `phase2` (`phase2.c`) | `phase2_peripherals` |
| `base` (`main.cpp`) | `base_bus_rx` |
| `led_test` (`led_test.cpp`) | `base_rgb_led` |

---

## 2. Índice

| Prueba | Carpeta | Fase | Comando | Cableado | Estado |
|---|---|---|---|---|---|
| [`phase2_peripherals`](#phase2_peripherals) | `firmware/module` | 2 | `pio run -e phase2_peripherals -t upload` | Una placa de módulo | Compila |
| [`base_rgb_led`](#base_rgb_led) | `firmware/base_test` | 2.1 | `pio run -e base_rgb_led -t upload` | SuperMini + 3 MOSFET + LED + final de carrera | Compila |
| [`phase3_bus_tx`](#phase3_bus_tx) | `firmware/module` | 3 (parcial) | `pio run -e phase3_bus_tx -t upload` | Módulo → DATA → base | Compila |
| [`base_bus_rx`](#base_bus_rx) | `firmware/base_test` | 3 (parcial) | `pio run -e base_bus_rx -t upload` | Base escucha DATA | Compila |

Previstos, aún sin escribir: `phase3_bus` y `base_bus` (bus half-duplex con `PING`/`GET_STATE`, menú `ping` y `stress`), y `phase5_sense` y `base_sense` (SENSE por DATA y comando `map`).

---

## 3. Placas usadas en el banco

| Placa | Uso | Notas |
|---|---|---|
| **ESP32-C3 SuperMini** | Base | USB-C al PC: monitor serie y grabación |
| **TENSTAR CH32V003F4P6** (×2) | Módulos A y B | Ver §3.1 |
| **WCH-LinkE** | Grabar las TENSTAR | Modo RISC-V |

### 3.1 TENSTAR

> Datos de una guía de un tercero (dillonsimeone.com), **no del fabricante**. Confírmalos con tu unidad antes de fiarte de ellos.

- Todos los pads marcados **V** son un único raíl, **sin regulador de 3,3 V**: con USB-C está a ~5 V y con otra fuente toma esa tensión. El CH32V003 admite 2,7–5,5 V.
- Pads de programación: **SWD** (PD1/SWIO), **V** y **G**.
- Sin LED de usuario en esa revisión.
- **No es** la placa de `Hardware/devboard_ch32v003`, que es un diseño propio con XC6206 y puente de alimentación.

**Alimentación en el banco:** el pad V de cada TENSTAR va al **3V3 de la SuperMini**, sin USB-C en las TENSTAR. Así los GPIO de las TENSTAR están a 3,3 V, igual que los del ESP32-C3. **Una sola fuente por TENSTAR.**

**Grabar una TENSTAR**

| Situación | Conexiones del WCH-LinkE |
|---|---|
| Sin USB-C (alimentada por la SuperMini) | SWD, V y G. Sin cables de V desde la SuperMini a la vez |
| Con USB-C | Solo SWD y G. **Desconecta de la SuperMini el pad V y los cables de DATA y SENSE**: con USB-C los GPIO están a 5 V |

---

## 4. Pruebas

### phase2_peripherals

**Valida:** lectura de los puentes de dirección (PC0–PC4), de pareja (PC5, PD4), final de carrera (PD6, antirrebote de 25 ms) y de los cuatro SENSE (PA1, PA2, PD2, PD3). Solo `printf`, sin bus.

- **Archivo:** `firmware/module/src/phase2_peripherals.c`
- **Salida:** `printf` por SWIO. Abre `minichlink -T` **antes** de probar: sin terminal, cada `printf` espera ~200 ms.
- **Cableado:** puente a GND = bit a 1; final de carrera entre PD6 y GND; cada SENSE con 1 kΩ en serie.
- **Criterio:** direcciones 1, 2, 21 y 31 se leen bien; con dirección 0 sale «unconfigured»; 50 aperturas y cierres del final de carrera dan **exactamente 100 eventos**; cada SENSE cambia a 0 solo al llevarlo a GND.
- **Fase del plan:** 2.2, 2.3 y 2.4.

### base_rgb_led

**Valida:** controlar un LED RGB de ánodo común con tres MOSFET desde la SuperMini, con PWM, y cambiar de modo con un final de carrera.

- **Archivo:** `firmware/base_test/src/base_rgb_led.cpp`
- **Cableado (una rama por color, tres iguales):**
  ```
  +V (4,5–6 V; luego +5 V) ──► ánodo común (pata larga)
  cátodo del color ──[R]──► DRAIN
                            SOURCE ──► GND común
                            GATE ◄── GPIO     GATE ──[100 kΩ]── GND
  ```
  Rojo en **GPIO4**, verde en **GPIO5**, azul en **GPIO7**. Resistencias a 5–6 V: rojo ≈ 150–200 Ω, verde y azul ≈ 100–150 Ω. GND de la fuente, de la SuperMini y de los MOSFET unidos.
- **Final de carrera:** **COM a GND y NO a GPIO6**; NC sin conectar. Usa la pull-up interna.
- **Monitor serie** (115200). Comandos: `r`, `g`, `b`, `w`, `off`; `rgb R G B` (0–255); `fade`; `mode N`; `help`.
- **Cada pulsación del final de carrera** pasa al siguiente modo: 0 apagado, 1 rojo, 2 verde, 3 azul, 4 blanco, 5 ciclo de color con PWM.
- **Criterio:** cada color enciende solo, el blanco es R + G + B, `rgb 255 80 0` da naranja, `fade` sube y baja y se para con una tecla, y cada pulsación cambia de modo una sola vez (sin dobles por rebote).
- **Medidas del LED** (MP017043, 6 V, un color a la vez): Vf rojo 1,923 V a 19,64 mA; verde 2,92 V a 19,2 mA; azul 2,93 V a 19,3 mA.
- **En el proyecto final** estos mismos pines (GPIO4/5/7 y GPIO6) son el LED y el final de carrera de la base (`02_diseno_hardware.md` §10).
- **MOSFET:** el BS170 (TO-92) o el RFD14N05L (TO-251) sirven para el banco, pero **no validan el 2N7002 de la PCB**. Comprueba el pinout con el multímetro en modo diodo antes de alimentar.
- **Fase del plan:** 2.1 (el LED, en el ESP32 en lugar del módulo).

### phase3_bus_tx

**Valida:** que el módulo emite tramas válidas por DATA. Solo transmite; TX en push-pull.

- **Archivo:** `firmware/module/src/phase3_bus_tx.c`
- **Cableado:** PD5 → 1 kΩ → DATA; DATA a RX de la base (GPIO20) y 4,7 kΩ a 3V3; GND común.
- **Criterio:** la base (`base_bus_rx`) imprime `[ack] frame ok` una vez por segundo, con el contador subiendo y `bad=0`.
- **Fase del plan:** 3 (parcial).

### base_bus_rx

**Valida:** el lado receptor de la prueba anterior y el módulo `bus.h` (autotest de la trama y del CRC).

- **Archivo:** `firmware/base_test/src/base_bus_rx.cpp`
- **Salida:** monitor serie; el LED de la SuperMini parpadea con cada trama válida.
- **Criterio:** `bus.h self-test: PASS`; `[ack] frame ok` periódico; `[err] bad CRC` solo si hay un fallo real.
- **Fase del plan:** 3 (parcial).

---

## 5. Pendientes

- **`phase3_bus` / `base_bus`:** half-duplex en el módulo (bit `HDSEL`, open-drain, descartar el eco, esperar ~100 µs antes de contestar) y `PING` / `GET_STATE` con el menú `ping 1` y `stress 1 1000` (criterio: 0 errores).
- **`phase5_sense` / `base_sense`:** `SENSE_DRIVE`, `SENSE_RELEASE`, `SENSE_QUERY` y el comando `map`.
- Verificar con multímetro los datos de la TENSTAR de §3.1.
