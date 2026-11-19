# Montaje de la base en placa perforada

Versión 1.0 · Noviembre 2026 · **Guía de montaje del módulo base (controlador) sin PCB**

> **Alcance.** Cómo montar la base del prototipo en placa perforada de doble cara metalizada, con la SuperMini enchufada. Implementa `02_diseno_hardware.md` §10 y usa la asignación de pines de `04_diseno_software.md` §4.5.
>
> **La base no se pasa a PCB**: es una sola unidad y pedir una placa para una pieza no sale rentable, así que se queda en placa perforada. La base es además un cajón más: lleva su propio LED RGB y su final de carrera (`02_diseno_hardware.md` §10).

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
| Diodo Schottky 1N5818 o 1N5819 | 1 | Farnell 4050106 (1N5818) o 4050107 (1N5819) |
| Polyfuse 30R185UU (1,85 A) | 1 | Farnell 1822236 |
| TVS P6KE6V8A | 1 | Farnell 2981330 |
| Electrolítico 1000 µF 16 V (KSY108M016S1G5H15K) | 1 | Farnell 2901345 |
| Cerámico 100 nF, de patillas | 2 | Farnell (ver BOM) |
| LED RGB de 5 mm, ánodo común | 1 | Ver `bom_5_cajones.md` §8 |
| MOSFET N lógico (2N7002 en adaptador SOT-23, BS170 o RFD14N05L) | 3 | El de la PCB es el 2N7002; en perforada vale un MOSFET de patillas (comprobar el pinout) |
| Resistencias del LED: 150 Ω (rojo), 100 Ω (verde y azul) | 3 | Mismas que la PCB |
| Resistencias 100 kΩ (puerta a GND) | 3 | Mismas que la PCB |
| Microinterruptor (final de carrera) | 1 | Farnell DM1-01P-30-3 |
| Cable de 22 AWG (silicona) | 1 m | Farnell o casa |
| Cable de 26 AWG (silicona) | — | Para señales |

Los 5,1 kΩ de CC1 y CC2 vienen en el adaptador USB-C; si no, son dos más de Farnell (MFR-25FRF52-5K1).

---

## 2. Asignación de pines

La asignación de GPIO es la de `02_diseno_hardware.md` §10 (**fuente única**; no cambiar sin actualizar también el firmware). Esta tabla solo añade a dónde se conecta cada pin al montar la base:

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
| GPIO4 / GPIO5 / GPIO7 | LED rojo / verde / azul | Puerta de su MOSFET (100 kΩ de la puerta a GND); drenador a su resistencia y al cátodo del color |
| GPIO6 | Final de carrera | COM a GND, NO al pin |

Evitar GPIO2, GPIO8 (salvo el LED de estado de la placa) y GPIO9: son pines de arranque (*strapping*) o el botón BOOT.

---

## 3. Esquema de conexiones

```
  ENTRADA DE 5 V
  Breakout USB-C (solo VBUS y GND; CC1 y CC2 llevan 5,1 kΩ a GND en el breakout)
      VBUS ──[polyfuse 30R185UU, 1,85 A]──┬── RAÍL +5V
                                           ├── TVS P6KE6V8A (banda a +5V) ── GND
                                           └── 1000 µF, 16 V (+ a +5V) ── GND
      RAÍL +5V ──┬─► pin 2 y pin 6 de los 4 box header
                 ├─► SuperMini, pin 5V   (obligatorio)
                 └─► ánodo común del LED

  BUS DATA  (3V3 de la SuperMini)
      3V3 ──┬──[4,7 kΩ]──┐
            │            DATA ──► pin 3 de los 4 box header (L, R, U, D)
            │             ▲
      GPIO21 (TX) ──►|──┘        (Schottky 1N5818 o 1N5819, banda hacia GPIO21)
      GPIO20 (RX) ───────────────► DATA

  SENSE  (uno por cara, nunca unidos entre sí)
      GPIO0  ──[1 kΩ]──► pin 4 del box header L
      GPIO1  ──[1 kΩ]──► pin 4 del box header R
      GPIO3  ──[1 kΩ]──► pin 4 del box header U
      GPIO10 ──[1 kΩ]──► pin 4 del box header D
      GND ────────────── pin 1 y pin 5 de los 4 box header

  LED RGB Y FINAL DE CARRERA  (mismo circuito que un módulo)
      +5V ──► ánodo común del LED (pata larga)
      cátodo rojo  ──[150 Ω]──► DRAIN del MOSFET Q1   GATE ◄── GPIO4   SOURCE ──► GND
      cátodo verde ──[100 Ω]──► DRAIN del MOSFET Q2   GATE ◄── GPIO5   SOURCE ──► GND
      cátodo azul  ──[100 Ω]──► DRAIN del MOSFET Q3   GATE ◄── GPIO7   SOURCE ──► GND
      cada GATE ──[100 kΩ]── GND
      GPIO6 ──► NO del microinterruptor      COM ──► GND

  Cerámicos de 100 nF (×2) cerca de la alimentación de la SuperMini, a GND.
```

Para dibujar este esquema en KiCad, paso a paso: `08_guia_kicad_base.md`.

### 3.1 Resistencias del LED

Calculadas para **≈ 20 mA por color** con el raíl a 5 V y las tensiones directas **medidas** en el LED MP017043 (6 V, un color a la vez, ≈ 19,5 mA). El cálculo supone ~0,1 V de caída en el MOSFET encendido; es una estimación, no una medida.

| Color | Vf medida | Resistencia | Corriente esperada |
|---|---|---|---|
| Rojo | 1,923 V | **150 Ω** | ≈ 19,8 mA |
| Verde | 2,92 V | **100 Ω** | ≈ 19,8 mA |
| Azul | 2,93 V | **100 Ω** | ≈ 19,7 mA |

Si el raíl cae a 4,75 V (el mínimo de un USB), salen ≈ 18 mA (rojo) y ≈ 17 mA (verde y azul): se sigue viendo bien y queda lejos de los 30 mA que el LED no debe superar.

### 3.2 Resumen de componentes de la base

| Componente | Cantidad | Para qué |
|---|---|---|
| Resistencia 4,7 kΩ | 1 | Pull-up de DATA (único del sistema) |
| Resistencia 1 kΩ | 4 | Una por línea SENSE |
| Resistencia 100 kΩ | 3 | Puerta de cada MOSFET a GND |
| Resistencias 150 Ω / 100 Ω / 100 Ω | 3 | LED (§3.1) |
| Condensador 1000 µF, 16 V | 1 | Reserva del raíl de 5 V; el único electrolítico del sistema |
| Condensador 100 nF | 2 | Desacoplo de la SuperMini |
| Polyfuse 30R185UU (1,85 A) | 1 | Fusible reiniciable: corta si hay un cortocircuito en el raíl de 5 V |
| TVS P6KE6V8A | 1 | Recorta picos de tensión (enchufar un cargador, descargas); no conduce a 5 V |
| Schottky 1N5818 o 1N5819 | 1 | Hace que el TX push-pull del ESP32 se comporte como open-drain en DATA |
| 5,1 kΩ en CC1 y CC2 | 2 | Los trae el breakout; mide antes de enchufar un cargador |

Notas:
- **Schottky:** la banda (cátodo) va hacia GPIO21. El bus DATA es open-drain: solo se baja a 0 y el pull-up lo sube. El TX del ESP32 es push-pull, así que, con el diodo, un 0 baja la línea y un 1 la deja libre (el diodo queda cortado). Sin él, el TX pelearía con los módulos.
- **TVS:** unidireccional; la banda (cátodo) va a +5V. Compruébalo en la hoja de datos de tu pieza.
- **Pull-up:** solo uno en todo el sistema, de DATA a 3V3 de la SuperMini.
- **Los cuatro SENSE** llevan cada uno su 1 kΩ en serie.

---

## 4. Orden de montaje

1. **Zócalos.** Suelda las dos tiras hembra 1×8 en la perforada, separadas 15,24 mm entre filas (las dos filas de la SuperMini). Enchufa la SuperMini para alinear antes de soldar.
2. **Box headers.** Los cuatro, uno por lado. Muesca hacia el centro de la placa, como en los módulos.
3. **Entrada de alimentación.** Adaptador USB-C, polyfuse, TVS y electrolítico en el raíl de 5 V; el TVS y el electrolítico van a GND.
4. **Bus DATA.** Pull-up 4,7 kΩ, Schottky y los cuatro pines de DATA de los box header en la misma línea.
5. **SENSE.** Cuatro resistencias de 1 kΩ, cada una a su box header. Mide continuidad: cada SENSE debe ir solo a su conector.
6. **LED RGB y final de carrera.** Tres MOSFET con su resistencia de puerta (100 kΩ a GND) y su resistencia de color, el ánodo común a +5 V y el microinterruptor entre GPIO6 y GND. Comprueba el pinout de los MOSFET con el multímetro en modo diodo antes de alimentar.
7. **Cerámicos de 100 nF** cerca de la alimentación de la SuperMini.

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

Los nombres de los pines, las resistencias y las posiciones de los box header coinciden con los de la placa de módulo (`03_diseno_pcb.md`), para que la base se comporte como un módulo más.
