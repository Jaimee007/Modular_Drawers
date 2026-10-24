# BOM — Prototipo de cajonera modular con 5 cajones

Configuración: **4 cajones 1×1 (50×50) + 1 cajón 2×1 (100×50) + 1 módulo base con el ESP32**.

Versión de prototipo: placas de desarrollo y placa perforada, sin PCB a medida. Conexión entre cajones con coletas de cable plano IDC (plan v1.1). Precios orientativos de AliExpress/LCSC (octubre 2026); pueden variar según el vendedor.

---

## 1. Cuántos componentes usa cada módulo

| Componente | 1×1 (×4) | 2×1 (×1) | Base (×1) | Total usado |
|---|---|---|---|---|
| Placa CH32V003F4P6 | 1 | 1 | — | 5 |
| ESP32-C3 SuperMini | — | — | 1 | 1 |
| Coleta (≈70 mm de cable plano de 6 hilos + 1 conector hembra IDC 2×3), en las caras derecha y arriba | 2 | 3 | 2 | **13 coletas** → 13 conectores IDC y ≈1 m de cable |
| Conector macho con carcasa 2×3 (box header), en las caras izquierda y abajo | 2 | 3 | 2 | **13** |
| LED SK6812 RGBW | 1 | 2 | 1 (estado) | 7 |
| Microinterruptor (final de carrera) | 1 | 1 | 1 (el cajón de la base) | 6 |
| Regulador AMS1117-3.3 | 1 | 1 | — (el ESP32 lleva el suyo) | 5 |
| Diodo de alimentación del LED (1N4001) | 1 | 1 | 1 | 6 |
| Resistencia de 1 kΩ (SENSE + DATA) | 4 + 1 | 6 + 1 | 4 | 31 |
| Condensadores de 10 µF + 100 nF | 1 + 1 | 1 + 1 | 1 + 1 | 6 + 6 |
| Pull-up de 4,7 kΩ en DATA | — | — | 1 | 1 |
| Diodo Schottky en el TX del ESP32 (BAT85 o 1N5819) | — | — | 1 | 1 |
| Polyfuse de 2 A + condensador de 1000 µF + TVS | — | — | 1 + 1 + 1 | 1 + 1 + 1 |
| Placa USB-C de alimentación | — | — | 1 | 1 |

---

## 2. Lista de compra

### A. Componentes específicos del proyecto (imprescindibles)

| # | Componente | Qué buscar | Cantidad a comprar | Precio aprox. |
|---|---|---|---|---|
| A1 | Microcontrolador | "CH32V003F4P6 core board / development board" (con todos los pines accesibles) | 7 (5 + 2 de repuesto) | 10,50 € |
| A2 | Programador | WCH-LinkE (programa el CH32V003 por el pin SWIO) | 1 | 4,50 € |
| A3 | ESP32 | ESP32-C3 SuperMini | 2 (1 de repuesto) | 6,00 € |
| A4 | Conector macho | "IDC box header 2x3P 2.54mm", recto (straight), de patillas | 20 (13 + pruebas + repuesto) | 1,50 € |
| A5 | Conector hembra IDC | "FC-6P IDC female 2.54mm" (2×3, para cable plano de 1,27 mm) | 20 | 1,50 € |
| A6 | Cable plano | "Flat ribbon cable 6 pin 1.27mm", gris con hilo rojo | 2 m | 1,50 € |
| A7 | LEDs | Tira SK6812 **RGBW** de 5 V, 60 LEDs/m, 1 m (alternativa más barata: WS2812B, ~4 €) | 1 | 7,00 € |
| A8 | Final de carrera | Microinterruptor mini con palanca, tipo KW10 o KW11 | 10 | 2,00 € |
| A9 | Regulador 3,3 V | Módulo mini AMS1117-3.3 | 10 | 2,50 € |
| | **Subtotal A** | | | **≈ 37,00 €** |

### B. Componentes sueltos (packs; quizá ya tengas algunos)

| # | Componente | Qué buscar | Cantidad a comprar | Precio aprox. |
|---|---|---|---|---|
| B1 | Diodo de alimentación del LED | 1N4001 o 1N4007 (de patillas) | pack de 20 | 1,00 € |
| B2 | Diodo del TX del bus | BAT85 o 1N5819 (Schottky, de patillas) | pack de 10 | 1,00 € |
| B3 | Polyfuse | 2 A de mantenimiento, de patillas (p. ej. RXEF200) | pack de 10 | 1,50 € |
| B4 | TVS | P6KE6V8A (protección del raíl de 5 V) | pack de 10 | 1,50 € |
| B5 | Condensador de reserva | Electrolítico de 1000 µF y 10 V | pack de 5 | 1,50 € |
| B6 | Condensadores | Surtido cerámico (100 nF) + electrolítico o cerámico de 10 µF | 1 surtido | 3,00 € |
| B7 | Resistencias | Surtido de 1/4 W (se usan 1 kΩ y 4,7 kΩ) | 1 surtido | 3,00 € |
| B8 | Entrada de alimentación | Placa breakout USB-C hembra (solo alimentación, 5 V) | 3 | 2,00 € |
| B9 | Placa perforada | **Doble cara**, metalizada, de 5×7 cm (para las placas principales; el conector macho 2×3 encaja en su paso de 2,54 mm) | 10 | 3,50 € |
| B10 | Zócalos para las placas | Tiras de pines hembra de 2,54 mm y 40 pines (así puedes cambiar un micro sin desoldar) | 10 | 2,00 € |
| B11 | Jumpers de dirección | Tiras de pines macho de 2,54 mm + 100 jumpers. En el prototipo sustituyen a los puentes de soldadura | 1 pack | 2,00 € |
| B12 | Cable | Silicona de 26 AWG, kit de 6 colores | 1 | 6,00 € |
| B13 | Termorretráctil | Surtido | 1 | 2,00 € |
| B14 | Tornillería | 50 insertos de calor M2 + 50 tornillos M2×6 | 1 + 1 | 5,00 € |
| | **Subtotal B** | | | **≈ 35,00 €** |

### C. Filamento

| # | Material | Para qué | Consumo estimado | Precio aprox. |
|---|---|---|---|---|
| C1 | PETG opaco, 1 kg | Armazones, cajones y piezas de prueba | ~630 g (1×1 ≈ 75 g, 2×1 ≈ 140 g, base ≈ 85 g, pruebas ≈ 100 g) | 18 € |
| C2 | PETG natural o translúcido, 250 g–1 kg | Cajas de luz (y pruebas de espesor) | ~50 g | 8–18 € |

### D. Herramientas y opcionales

| # | Componente | Comentario | Precio aprox. |
|---|---|---|---|
| D1 | Cargador USB-C de 5 V y 3 A | Seguramente ya tienes uno | 0–8 € |
| D2 | Analizador lógico de 8 canales y 24 MHz | Muy recomendable para depurar el bus y los SENSE (con PulseView) | 6 € |
| D3 | Pie de rey | Para medir los conectores enchufados y ajustar la zona trasera | 8 € (si no tienes) |
| D4 | Silicona caliente o epoxi | Para sujetar las coletas junto a la soldadura | 3 € |
| D5 | Pintura blanca o filamento blanco | Interior de la caja de luz (opcional) | — |

Se supone que ya tienes soldador, estaño, flux y multímetro.

---

## 3. Resumen de costes

| Bloque | Coste |
|---|---|
| A. Componentes específicos del proyecto | ≈ 37,00 € |
| B. Componentes sueltos (packs) | ≈ 35,00 € |
| **Electrónica total** | **≈ 72 €** |
| C. Filamento (si no tienes) | 26–36 € |
| D. Herramientas y opcionales | 0–25 € |

**Lo que se reaprovecha:** casi todo el bloque B y buena parte del A (cable plano, tira de LEDs, interruptores, reguladores, programador) son packs que sobran. Con lo que compras aquí te queda material para unos **15–20 cajones más** (comprando algún conector más), y cada uno te costaría poco más que la placa del micro (~1,50 €), los conectores (~0,30 €) y el filamento (~1,30 €).

---

## 4. Notas de compra

1. **Conectores y cable primero.** Son lo único que bloquea el diseño 3D. Cuando lleguen, enchufa un conector hembra en un macho y mide la altura total y el ancho, y prueba cuánto se puede doblar el cable.
2. **Montar los conectores hembra IDC:** se cierran a presión con un tornillo de banco o unos alicates de boca plana. Coloca siempre el hilo rojo en el pin 1 (marcado con un triángulo en el conector). En la PCB, el hilo rojo de la coleta va al pad 1 (GND).
3. **Placas CH32V003:** compra todas al mismo vendedor, para que tengan la misma distribución de pines. Comprueba que estén accesibles PA1, PA2 y todo el puerto C y el D.
4. **Tira de LEDs:** tiene que ser de **5 V** (no de 12 V) y SK6812 **RGBW** (no la "RGB" a secas, si quieres el blanco aparte). Las tiras IP30, sin silicona, se cortan y sueldan mejor.
5. **Placa perforada:** que sea de **doble cara metalizada**. Los conectores macho y las coletas quedan mucho más firmes que en la de una sola cara.
6. **Para pasar a PCB propia** (versión final): los reguladores AMS1117, los zócalos y los jumpers se sustituyen por componentes SMD y puentes de soldadura en la PCB (ver §2.11 del plan).

---

## 5. Versión con PCB montada por JLCPCB

Piezas comprobadas en el catálogo de JLCPCB el 23/10/2026. Los precios están en USD, son por unidad para 1–49 unidades y no incluyen las tasas de montaje.

**Tipos de pieza en JLCPCB:**

- **Básicas (Basic):** sin coste de montaje extra.
- **Extendidas (Extended):** JLCPCB cobra una tasa fija por cada referencia distinta y pedido (unos 3 $ en sus tarifas habituales; compruébalo en el presupuesto).

| Función | Pieza | Código JLC/LCSC | Tipo | Precio (1–49) | Stock |
|---|---|---|---|---|---|
| Microcontrolador | CH32V003F4P6 (TSSOP-20) | C5187096 | Extendida | 0,31 $ (0,24 $ a partir de 50) | ~29 600 |
| LDO 3,3 V | XC6206P332MR-G (SOT-23) | C5446 | Básica | 0,15 $ | ~462 000 |
| LDO 3,3 V (alternativa) | AMS1117-3.3 (SOT-223) | C6186 | Básica | 0,21 $ | ~987 000 |
| Diodo de alimentación del LED | 1N4148W (SOD-123) | C81598 | Básica | 0,01 $ | ~4,1 M |
| Diodo de alimentación del LED (más corriente, para el 2×1) | SS14 (SMA) | C2480 | Básica | 0,02 $ | ~1,8 M |
| Diodo del TX del bus | B5819W SL (SOD-123) | C8598 | Básica | 0,03 $ | ~601 000 |
| LED RGBW | SKC6812RGBW-BW (5050) | C5181320 | Extendida | 0,16 $ | ~8 500 |
| LED RGBW (otra variante) | SKC6812RGBW-WS | C5378724 | Extendida | 0,10 $ | **sin stock** |
| LED RGB (alternativa) | WS2812B-B/T (5050) | C2761795 | Extendida | 0,12 $ | ~14 000 |
| LED RGB 2020 | WS2812B-2020 | C965555 | Extendida | — | **sin stock** |
| Final de carrera | DM1-12UP-1 (de patillas, SPDT, 12,7×5,7 mm) | C405949 | Extendida | 0,21 $ | ~44 500 |
| USB-C de la base | TYPE-C-31-M-12 | C165948 | Extendida | 0,19 $ | ~330 000 |
| ESP32 en la PCB de la base (opcional, en vez de la SuperMini) | ESP32-C3-MINI-1-N4 | C2838502 | Extendida | 3,05 $ | ~1 160 |
| Resistencias y condensadores 0603/0805 | Genéricos | Varios | Básicas | < 0,01 $ | — |
| Conector macho 2×3 con carcasa (box header, 2,54 mm) | Buscar "Box Header 2x3P 2.54mm" | Sin verificar | — | ~0,05–0,10 $ | — |

**Lo que no se pide montado en JLCPCB:**

- **Conector macho 2×3:** es una pieza de patillas; puedes pedir que JLCPCB lo monte (montaje THT, con coste extra) o soldarlo tú, que es fácil.
- **Coletas (conector hembra IDC + cable plano):** no van en la PCB; se compran aparte (AliExpress/LCSC), se montan a mano y se sueldan a los pads.
- **Fuera de JLCPCB:** WCH-LinkE, filamento, cable de silicona, insertos, analizador lógico y placas de desarrollo para el prototipo.
