# BOM — Prototipo de cajonera modular con 5 cajones

Configuración: **4 cajones 1×1 (50×50) + 1 cajón 2×1 (100×50) + 1 módulo base con el ESP32**. Al haber un módulo por unidad de retícula, son **6 módulos + la base**: el 2×1 son dos módulos idénticos emparejados.

Versión de prototipo: placas de desarrollo y placa perforada, sin PCB a medida. Conexión entre módulos con **latiguillos de cable plano con conector hembra IDC en los dos extremos**; el cable nunca se suelda a la placa (ver `02_diseno_hardware.md` §3). Precios orientativos de AliExpress/LCSC (octubre 2026); pueden variar según el vendedor.

> **Cambio en curso (29/10/2026):** se estudia sustituir el latiguillo IDC por **cable FFC de 10 hilos y 1 mm** con un conector FFC SMD en la placa del módulo (ver §8). Mientras no se decida, las filas del IDC (A4, A5, A6) y el carrito de Farnell de §7 siguen siendo válidos para la **base en placa perforada** y para el prototipo.

---

## 1. Cuántos componentes usa cada módulo

| Componente | Módulo (×6) | Base (×1) | Total usado |
|---|---|---|---|
| Placa CH32V003F4P6 | 1 | — | 6 |
| ESP32-C3 SuperMini | — | 1 | 1 |
| Conector macho con carcasa 2×3 (box header), uno por cara | 4 | 4 | **28** |
| Latiguillo (≈70 mm de cable plano de 6 hilos + 1 hembra IDC 2×3 en cada extremo) | — | — | **6 como mínimo** (cada módulo con alguna cadena hasta la base); hasta ~10 si se cablean todas las caras vecinas → 12–20 conectores IDC y ≈1 m de cable |
| LED RGB de 5 mm, **ánodo común** (4 patillas, sin driver; sin canal blanco) | 1 | — (la SuperMini trae el suyo) | 6 |
| MOSFET 2N7002 (SOT-23), uno por color — **van en la PCB, los monta JLCPCB** | 3 | — | 18 |
| Resistencias del LED: 150 Ω (rojo) y 100 Ω (verde y azul) — en la PCB | 3 | — | 18 |
| Resistencias de puerta 100 kΩ (una por MOSFET) — en la PCB | 3 | — | 18 |
| Microinterruptor (final de carrera) | — | — | 5 (uno por cajón: el 2×1 comparte uno con cable en Y hacia sus dos módulos) |
| Cable Dupont hembra de 3 hilos, ~150 mm (LED → J6) | 1 | 1 | 7 |
| Cable Dupont hembra de 2 hilos (interruptor → J7) | 1 (el 2×1 usa un cable en Y) | — | 6 |
| Regulador AMS1117-3.3 | 1 | — (el ESP32 lleva el suyo) | 6 |
| Resistencia de 1 kΩ (1 en DATA + 4 en SENSE) | 5 | 4 (SENSE) | 34 |
| Condensador de 10 µF | 1 | 1 | 7 |
| Condensador de 100 nF | 2 | 1 | 13 |
| Condensador de 1 µF | 1 | — | 6 |
| Jumpers de dirección (5) y de pareja (2); sustituyen a los puentes de soldadura | 7 | — | 42 |
| Pull-up de 4,7 kΩ en DATA | — | 1 | 1 |
| Diodo Schottky en el TX del ESP32 (BAT85 o 1N5819) | — | 1 | 1 |
| Polyfuse de 2 A + condensador de 1000 µF + TVS | — | 1 + 1 + 1 | 1 + 1 + 1 |
| Placa USB-C de alimentación | — | 1 | 1 |
| Resistencias de 5,1 kΩ (CC1 y CC2 del USB-C), **si la placa USB-C no las trae** | — | 2 | 2 |

> ⚠️ **Las dos resistencias de 5,1 kΩ en CC1 y CC2 no son opcionales.** Sin ellas, un cargador USB-C moderno no entrega corriente (ver `02_diseno_hardware.md` §7.2). Comprueba que la placa USB-C las lleve, o añádelas.

> Solo hay **un** pull-up de 4,7 kΩ en todo el sistema, en la base. Los módulos no llevan ninguno.

---

## 2. Lista de compra

### A. Componentes específicos del proyecto (imprescindibles)

| # | Componente | Qué buscar | Cantidad a comprar | Precio aprox. |
|---|---|---|---|---|
| A1 | Microcontrolador | "CH32V003F4P6 core board / development board" (con todos los pines accesibles) | 8 (6 + 2 de repuesto) | 12,00 € |
| A2 | Programador | WCH-LinkE (programa el CH32V003 por el pin SWIO) | 1 | 4,50 € |
| A3 | ESP32 | ESP32-C3 SuperMini | 2 (1 de repuesto) | 6,00 € |
| A4 | Conector macho (box header) | **Amphenol T821106A1S100CEU** (Farnell 2215302): box header con carcasa, 2×3, 2,54 mm, **recto** (el latiguillo sale perpendicular a la placa), de patillas. Misma familia que el hembra T812 de A5. La versión de ángulo recto (T821106A1R100CEU, Farnell 2215288) estaba sin stock. Precio y stock sin verificar | 40 en el carrito (4 por módulo → 10 módulos; la base necesita 4 más) | 13,52 € (0,338 € la unidad, Farnell) |
| A5 | Conector hembra IDC | **Amphenol T812106A101CEU** (Farnell 2215245): hembra, 2×3, 2,54 mm, para cable plano de 1,27 mm, con descarga de tracción. Alternativa sin descarga: T812106A100CEU (Farnell 2215230). Precio no verificado | 40 en el carrito (2 por latiguillo → 20 latiguillos, para 10 módulos) | 14,44 € (0,361 € la unidad, Farnell) |
| A6 | Cable plano | **Würth 63910615521CAB** (Farnell, DigiKey, Mouser, RS): 6 hilos, 1,27 mm, 28 AWG, gris con hilo 1 rojo, 1 m (≈ 14 latiguillos de 70 mm) | 1 | ≈ 1,20 $ (DigiKey) |
| A7 | LEDs | **LED RGB de 5 mm, difuso, ánodo común, 4 patillas** (ver "Cómo buscar el LED" en §9). Vale cualquier RGB normal, sin chip. Por ejemplo, en Farnell el Multicomp Pro MP017043 (ref. 5122721) ya estaba en tu carrito, pero su ficha no confirma si es de ánodo común: **compruébalo** | 11 (+ repuestos) | ≈ 4–5 € (0,40 € la unidad, Farnell) |
| A8 | Final de carrera | Microinterruptor mini con palanca, tipo KW10 o KW11 | 10 | 2,00 € |
| A9 | Regulador 3,3 V | Módulo mini AMS1117-3.3 | 10 | 2,50 € |
| A10 | Cables Dupont hembra ya crimpados | De 3 hilos (LED) y de 2 hilos (interruptor), ~20 cm, 24 AWG. Evita comprar terminales y crimpadora. Ejemplo en LCSC: SHOU HAN 2.54-3P DT 200mm 24AWG (C5363464), de un solo extremo | 2 packs | 3,00 € (estimación mía, sin precio verificado) |
| | **Subtotal A** | | | **≈ 38,75 €** |

### B. Componentes sueltos (packs; quizá ya tengas algunos)

| # | Componente | Qué buscar | Cantidad a comprar | Precio aprox. |
|---|---|---|---|---|
| B1 | — (ya no hace falta) | El diodo D1 desaparece: el LED ya no es direccionable (ver `02_diseno_hardware.md` §5) | — | — |
| B2 | Diodo del TX del bus | Schottky de patillas: **1N5818** (el que has elegido; 1N5817 también vale). Banda (cátodo) hacia el TX del ESP32 | 1 | 0,14 € |
| B3 | Polyfuse | 2 A de mantenimiento, de patillas (p. ej. RXEF200) | pack de 10 | 1,50 € |
| B4 | TVS | P6KE6V8A (protección del raíl de 5 V) | pack de 10 | 1,50 € |
| B5 | Condensador de reserva | Electrolítico de 1000 µF y 10 V. **Solo en la base** | pack de 5 | 1,50 € |
| B6 | Condensadores | Surtido cerámico (100 nF y 1 µF) + 10 µF. **Máximo 10 µF por módulo en el 5 V** | 1 surtido | 3,00 € |
| B7 | Resistencias | Surtido de 1/4 W (se usan 1 kΩ, 4,7 kΩ y 5,1 kΩ) | 1 surtido | 3,00 € |
| B8 | Entrada de alimentación | **Placa conector USB-C hembra, serie básica, con pines de 2,54 mm** (AliExpress, placa roja): según la foto lleva una resistencia **512 (5,1 kΩ) en CC1 y otra en CC2, las dos a GND**. Se cablean solo VBUS y GND; CC1, CC2, D+ y D− quedan sin conectar. **Mide 5,1 kΩ entre CC1 y GND y entre CC2 y GND al recibirla.** Alternativa europea con las resistencias confirmadas en el anuncio: Nafcom, placa breakout USB-C hembra a DIP 2,54 mm con resistencias CC1/CC2 (ref. 108.MD9245, 0,823 € con IVA) | 1–2 | Precio no verificado |
| B9 | Placa perforada | **Doble cara**, metalizada, de 5×7 cm (para los módulos; el conector macho 2×3 encaja en su paso de 2,54 mm) | 10 | 3,50 € |
| B10 | Zócalos para las placas | Tiras de pines hembra de 2,54 mm y 40 pines (así puedes cambiar un micro sin desoldar) | 10 | 2,00 € |
| B11 | Jumpers | Pines macho de 2,54 mm + 100 jumpers. Sustituyen a los puentes de soldadura del prototipo (8 por módulo) | 1 pack | 2,00 € |
| B11b | Cabezales J5 (programación, 1×3), J6 (LED, **1×4**) y J7 (interruptor, 1×2) | **Multicomp Pro SPC20484** (Farnell): tira de 40 vías, 1 fila, 2,54 mm, recta, latón dorado. Buscarla en `es.farnell.com` (el enlace que vi era `cpc.farnell.com`). Comprobar que se corta en grupos de 2–4 pines. **9 pines por módulo** | 6 en el carrito (10 módulos usan 90 pines y las tiras macho de la SuperMini otros 16; bastan 3). Para 31 módulos: 7 | 9,54 € (1,59 € la unidad, Farnell) |
| B12 | Cable | Silicona de 26 AWG, kit de 6 colores. Para el **cable en Y** del 2×1 y para alargar el cable del interruptor | 1 | 6,00 € |
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
| D4 | Tornillo de banco o alicates de boca plana | Para cerrar a presión los conectores hembra IDC | — |
| D5 | Pintura blanca o filamento blanco | Interior de la caja de luz (opcional) | — |

Se supone que ya tienes soldador, estaño, flux y multímetro.

---

## 3. Resumen de costes

| Bloque | Coste |
|---|---|
| A. Componentes específicos del proyecto | ≈ 38,75 € |
| B. Componentes sueltos (packs) | ≈ 35,00 € |
| **Electrónica total** | **≈ 74 €** |
| C. Filamento (si no tienes) | 26–36 € |
| D. Herramientas y opcionales | 0–22 € |

**Lo que se reaprovecha:** casi todo el bloque B y buena parte del A son packs que sobran. Los límites son los interruptores y los reguladores (sobran para ~5 y ~4 módulos más) y los box header (cada módulo gasta 4). Un módulo extra cuesta poco más que la placa del micro (~1,50 €), los conectores y el latiguillo (~0,40 €) y el filamento (~1,30 €).

---

## 4. Notas de compra

1. **Conectores y cable primero.** Son lo único que bloquea el diseño 3D. Cuando lleguen, enchufa un hembra en un macho, **mide el conjunto enchufado** (tiene que caber en los 20 mm de zona trasera) y prueba cuánto se puede doblar el cable.
2. **Montar los latiguillos:** cada uno lleva un conector hembra IDC en **cada extremo**, cerrado a presión con un tornillo de banco o unos alicates de boca plana. Pon el **hilo rojo en el pin 1 (marcado con un triángulo) en los dos extremos**, para que el pin 1 de un módulo llegue siempre al pin 1 del vecino. Longitud ≈70 mm: solo alcanza al vecino inmediato. **No se suelda nada a la placa.**
3. **Placas CH32V003:** compra todas al mismo vendedor, para que tengan la misma distribución de pines. Comprueba que estén accesibles PA1, PA2 y todo el puerto C y el D.
4. **LEDs:** RGB de 5 mm de **ánodo común**, controlados por tres MOSFET 2N7002 en la PCB. No tienen canal blanco: el "blanco" al abrir es R+G+B. Un LED de cátodo común no sirve con el circuito de `02_diseno_hardware.md` §5.1: comprueba la polaridad en la ficha y prueba uno en protoboard antes de pedir las placas.
5. **Placa perforada:** que sea de **doble cara metalizada**. Los conectores macho quedan mucho más firmes que en la de una sola cara.
6. **Placa USB-C:** antes de usarla, mide con el multímetro que haya 5,1 kΩ entre CC1 y GND y entre CC2 y GND. Si no, añade las resistencias.
7. **Para pasar a PCB propia** (versión final): los reguladores AMS1117, los zócalos y los jumpers se sustituyen por componentes SMD y puentes de soldadura en la PCB (ver `03_diseno_pcb.md`).

---

## 5. Versión con PCB montada por JLCPCB

Piezas comprobadas en el catálogo de JLCPCB el 23/10/2026. Los precios están en USD, son por unidad para 1–49 unidades y no incluyen las tasas de montaje. La lista completa de la placa de módulo (referencias, huellas) está en `03_diseno_pcb.md` §4, y el pedido (≈25 unidades, Economic PCBA, una cara) en §8.3.

**Tipos de pieza en JLCPCB:**

- **Básicas (Basic):** sin coste de montaje extra.
- **Extendidas (Extended):** JLCPCB cobra una tasa fija por cada referencia distinta y pedido (unos 3 $ en sus tarifas habituales; compruébalo en el presupuesto).

La placa de módulo lleva **una sola pieza Extended** (el micro). El 2N7002 (C8545) es Basic.

| Función | Pieza | Código JLC/LCSC | Tipo | Precio (1–49) | Stock |
|---|---|---|---|---|---|
| Microcontrolador | CH32V003F4P6 (TSSOP-20) | C5187096 | Extendida | 0,31 $ (0,24 $ a partir de 50) | ~29 600 |
| LDO 3,3 V | XC6206P332MR-G (SOT-23) | C5446 | Básica | 0,15 $ | ~462 000 |
| LDO 3,3 V (alternativa) | AMS1117-3.3 (SOT-223) | C6186 | Básica | 0,21 $ | ~987 000 |
| MOSFET del LED (Q1–Q3), uno por color | JSCJ 2N7002 (SOT-23, 60 V, 115 mA) | C8545 | Básica | — | Sin leer |
| Resistencias del LED: 150 Ω, y 100 Ω (×2) | Uni-Royal 0603WAF1500T5E y 0603WAF1000T5E | C22808, C22775 | Básicas | < 0,01 $ | — |
| Resistencias de puerta 100 kΩ (×3) | Uni-Royal 0603WAF1003T5E | C25803 | Básica | < 0,01 $ | — |
| Resistencias 1 kΩ, condensadores 100 nF / 1 µF / 10 µF (0603/0805) | Genéricos | Varios | Básicas | < 0,01 $ | — |
| *Solo placa base:* diodo del TX del bus | B5819W SL (SOD-123) | C8598 | Básica | 0,03 $ | ~601 000 |
| *Solo placa base:* USB-C | TYPE-C-31-M-12 | C165948 | Extendida | 0,19 $ | ~330 000 |
| *Solo placa base:* resistencias 5,1 kΩ (CC1 y CC2) | Genéricas 0603 | Varios | Básicas | < 0,01 $ | — |
| *Solo placa base (opcional, en vez de la SuperMini):* ESP32 | ESP32-C3-MINI-1-N4 | C2838502 | Extendida | 3,05 $ | ~1 160 |

> El USB-C de la base debe ser de **16 pines con patas pasantes de anclaje**, no solo SMD (ver `02_diseno_hardware.md` §7.2). Comprueba la ficha de C165948 antes de pedir.

**Referencia para una futura placa de LED** (no se pide en este pedido: el prototipo usa un trozo de tira SK6812):

| Función | Pieza | Código JLC/LCSC | Tipo | Precio (1–49) | Stock |
|---|---|---|---|---|---|
| LED RGBW | SKC6812RGBW-BW (5050) | C5181320 | Extendida | 0,16 $ | ~8 500 |
| LED RGBW (otra variante) | SKC6812RGBW-WS | C5378724 | Extendida | 0,10 $ | **sin stock** |
| LED RGB (alternativa) | WS2812B-B/T (5050) | C2761795 | Extendida | 0,12 $ | ~14 000 |
| LED RGB 2020 | WS2812B-2020 | C965555 | Extendida | — | **sin stock** |

**Lo que no se pide montado en JLCPCB** (todo THT, se suelda a mano, ~10 min por placa):

- **Cuatro box header 2×3** (J1–J4): se marcan como "excluir del BOM". Sin verificar código LCSC; busca "Box Header 2x3P 2.54mm" (~0,05–0,10 $).
- **Tiras de pines J5, J6 y J7:** programación (1×3), LED (1×3) e interruptor (1×2).
- **Latiguillos (conector hembra IDC ×2 + cable plano):** no van en la PCB; se compran aparte (AliExpress/LCSC) y se montan a mano.
- **Final de carrera:** va por cable fuera de la placa (J7), montado en la pared trasera del armazón. El DM1-12UP-1 (C405949, 0,21 $, de patillas) sigue siendo candidato, pero no se pide montado en la placa.
- **Fuera de JLCPCB:** WCH-LinkE, filamento, cable de silicona, insertos, analizador lógico y placas de desarrollo para el prototipo.

---

## 6. ¿Cabe todo en un único pedido a LCSC?

**No.** Revisado en LCSC el 28/10/2026. Cubre la mayor parte de los componentes, pero faltan tres piezas clave. Los precios y el stock que se muestran son los que devolvió la búsqueda; no he abierto cada ficha, así que confírmalos en el carrito.

| Pieza | Código LCSC | Estado | Notas |
|---|---|---|---|
| Box header 2×3 recto (J1–J4) | C11214 (BOOMELE), C492441 (XFCN, 9,05 mm sobre la placa), C5157626 (kinghelm) | Existe | **Elegido el T821106A1S100CEU de Farnell (A4), también recto.** Estos LCSC quedan como alternativa. Comprobar la altura enchufada frente a los 20 mm de zona trasera |
| **Hembra IDC 2×3 para cable plano** | — | **No encontrado en LCSC** | Resuelto con Farnell: Amphenol T812106A101CEU (2215245) o T812106A100CEU (2215230) |
| Cable plano 1,27 mm | — | **No de 6 vías en LCSC** | Resuelto con Würth 63910615521CAB (Farnell, DigiKey) |
| CH32V003F4P6 | C5187096 | Existe | Las versiones QFN (C5299908) y SOP-16 (C5346357) tienen otra huella: no son intercambiables |
| WCH-LinkE | C5236707 ("CH32V003F4P6-EVT+WCH-LinkE") | Existe | Parece un kit con placa de evaluación; no verificado qué incluye |
| LED WS2812D-F5 | C190565 | **Datos contradictorios** | Una página dice "8 490 en stock, Discontinued"; otra, "Out of Stock". Desde 0,185 $ (5 uds) |
| LED WS2812D-F5-12mA-C1 | C4154875 | **Datos contradictorios** | 240 en stock en una página, agotado en otra. Desde 0,203 $ (5 uds) |
| Cable Dupont de 3 hilos ya hecho | C5363464 (SHOU HAN, 200 mm, 24 AWG, un extremo) | Existe | Precio no leído |
| Tira de pines macho 1×3 | C49257 (BOOMELE) | Existe | Alternativa a la Multicomp Pro SPC20484 de Farnell (B11b), que es la elegida para J5–J7 |
| Microinterruptor con palanca | Omron SS-01GL (C2896769); candidatos: K9-1267Q (0,12–0,15 $, 325 uds), SS-5GL (0,75 $), KW10-1P-C02 (0,72 $, 23 uds), DM1-16UP-1 (C405950) | Existen | **No he comprobado cuál tiene la palanca y las medidas del diseño.** Elige según la pieza T3 |
| Polyfuse 2 A de mantenimiento | SMD: C545213 (1812, 16 V), C2153878 (1210, 6 V) | **Solo SMD** | No encontré uno de patillas con 2 A de mantenimiento (los de patillas hallados tenían 1 A y 2 A de disparo) |
| TVS 5 V | P6KE6.8A (C409413, DO-15, Vrwm 5,8 V) | Existe | Equivale al P6KE6V8A de la lista |
| Electrolítico 1000 µF 10 V | C59324 (CapXon 8×14 mm, 0,05 $), C10752 (8×12), C2960391 (10×17) | Existe | C59324 tenía poco stock |
| Conector USB-C | C165948 (TYPE-C-31-M-12, 16 contactos) | Existe | Es el conector suelto, no una placa breakout. Patas de anclaje sin verificar |
| ESP32-C3 SuperMini | — | **No está en LCSC** | Solo módulos (ESP32-C3-MINI-1-H4X C41349510, ESP32-C3-WROOM-02-H4 C2944070). La SuperMini hay que comprarla en AliExpress |
| 1N4148W, XC6206P332, AMS1117, B5819W | C81598, C5446, C6186, C8598 | Códigos del 23/10 | No reverificados hoy |
| Placas de desarrollo CH32V003, placa perforada, cable de silicona, insertos M2, jumpers, filamento | — | No los busqué en LCSC | Normalmente AliExpress |

**Alternativas para lo que falta**

- **Hembra IDC 2×3 y cable plano:** ya no son un problema, ver las filas de A5 y A6. LCSC no es necesario para ellos.
- **Polyfuse:** en el prototipo, un 1812 de 2 A en un adaptador SMD-a-DIP, o dos de 1 A de patillas en paralelo (no verificado que sea buena práctica para este circuito). En la PCB de la base, el 1812 encaja.

**Reparto sugerido**

1. **LCSC:** micro, LEDs, interruptores, TVS, electrolítico, cable Dupont y pasivos.
2. **Farnell:** hembras IDC y lo que falte, hasta superar el envío gratis de 75 €.
3. **AliExpress:** ESP32-C3 SuperMini, placas de desarrollo CH32V003, placa perforada, silicona e insertos M2.

---

## 7. Carrito de Farnell y lo que falta (28/10/2026)

Pedido de partida: **10 módulos + la base**. La base va en **placa perforada**, con piezas de patillas (no SMD), y la SuperMini va enchufada en zócalos. Los precios son los del carrito, sin IVA.

### 7.1 Lo que ya está en el carrito

| Pieza | Ref. Farnell | Cant. | Total | Para qué | Estado |
|---|---|---|---|---|---|
| Box header T821106A1S100CEU (recto, 2×3) | 2215302 | 40 | 13,52 € | Conectores de bus J1–J4 de cada módulo | OK. La base necesita 4 más |
| Hembra IDC T812106A101CEU (con descarga de tracción) | 2215245 | 40 | 14,44 € | Extremos de los latiguillos | OK. Comprobar que entra en el T821 y el pin 1 |
| Cable plano Würth 63910615521CAB (1 m) | 3583701 | 2 | 4,10 € | Hilos de los latiguillos (≈ 28 de 70 mm) | OK |
| Tira de pines SPC20484 (1×40) | 1462888 | 6 | 9,54 € | J5, J6, J7 y las tiras macho de la SuperMini | **Sobran**: bastan 3 (ahorro 4,77 €) |
| Microinterruptor DM1-01P-30-3 | 1735364 | 10 | 3,37 € | Final de carrera de cada cajón | OK. Pedir 2 de repuesto y validar con la pieza T3 |
| Resistencia 1 kΩ (MFR-25FRE52-1K) | 4560128 | 10 | 0,38 € | 4 SENSE de la base | OK (necesitas 4) |
| Resistencia 4,7 kΩ (MFR25SFTF52-4K7) | 3496941 | 10 | 0,36 € | Pull-up de DATA | OK (necesitas 1) |
| Resistencia 5,1 kΩ (MFR-25FRF52-5K1) | 4729302 | 10 | 0,36 € | CC1 y CC2 del USB-C | OK (necesitas 2) |
| TVS P6KE6V8A | 2981330 | 5 | 2,44 € | Protección del 5 V de la base | OK (necesitas 1) |
| Polyfuse 30R185UU (1,85 A de mantenimiento) | 1822236 | 1 | 0,72 € | Protección del 5 V de la base | OK |
| Schottky 1N5819 | 4050107 | 1 | 0,14 € | En el TX del ESP32 | OK |
| **LED MP017043** (RGB 5 mm) | 5122721 | 10 | 4,02 € | LED de cada módulo | **SIRVE**: RGB normal de ánodo común (la pata larga es el común y va a +5 V; confirmado por quien lo ha medido). Pendiente solo de comprobarlo con una pila al recibirlo. Quita el MP008266 del carrito, es redundante |
| **Conector USB4105-GF-A** (USB-C, 16 vías) | 3588205 | 1 | 0,82 € | Entrada de 5 V | **QUITAR**: es SMD suelto, no se suelda a una perforada. Se sustituye por el adaptador USB-C de 2,54 mm de AliExpress (B8). Tampoco sirve el Amphenol USBCBRA0101SR (24 posiciones, SMD) |
| | | | **54,21 €** | | Sin IVA. Sin el LED: 50,19 €. Sin LED ni 3 tiras: 45,42 € |

### 7.2 Lo que falta

**Para los módulos**

| Pieza | Cant. | Qué buscar | Dónde |
|---|---|---|---|
| **LED RGB de 5 mm, difuso, ánodo común** (4 patillas, sin driver) | 11 (+ repuestos) | Ver "Cómo buscar el LED" en §9 | Farnell, BricoGeek, AliExpress, LCSC |
| Cables Dupont hembra | 20 | "cable puente hembra-hembra 2,54 mm 20 cm" (se cortan por la mitad: 40 colas) | Farnell o LCSC C5363464 |
| Insertos roscados M2 | 20 | "inserto roscado M2 para plástico" | Farnell o AliExpress |
| Tornillos M2 de 6 mm | 20 | "tornillo M2 6 mm" | Farnell o AliExpress |
| Termorretráctil | 1 surtido | "termorretráctil surtido" | Farnell |
| Microinterruptores de repuesto | 2 | Los mismos que ya tienes | Farnell |

**Para la base (placa perforada)**

| Pieza | Cant. | Qué buscar | Dónde |
|---|---|---|---|
| **Adaptador USB-C hembra de 16 pines a 2,54 mm** | 1 (2 de repuesto) | El anuncio de AliExpress que encontraste. Hay que añadir las dos 5,1 kΩ de CC1/CC2 (ya en el carrito) | AliExpress |
| Condensador electrolítico 1000 µF, 10 V o más | 1 | "condensador electrolítico 1000µF 10V radial" (vale de 16 V) | Farnell |
| Tira hembra 1×8, 2,54 mm, de patillas | 2 | "conector hembra 1 fila 8 contactos 2.54mm" | Farnell |
| Placa perforada de doble cara metalizada | 1 | "placa prototipos doble cara 2.54mm" | Farnell o AliExpress |
| Cable 22 AWG para el 5 V y GND | 1 m | "cable de conexión 22AWG silicona" | Farnell |
| 4 box headers + 4 hembras IDC extra | 4 + 4 | Los mismos T821 y T812 | Si los 40 + 40 ya son de los módulos, súmalos |

**Fuera de Farnell**

| Pieza | Cant. | Dónde |
|---|---|---|
| ESP32-C3 SuperMini | 2 (1 de repuesto) | AliExpress |
| Placas de desarrollo CH32V003 | 11 (si pruebas antes de la PCB) | AliExpress |
| WCH-LinkE | 1 | LCSC C5236707 (kit con placa de evaluación, ≈ 7 $, 2–3 uds en stock) o AliExpress |
| Filamento: PETG opaco 1 kg y translúcido 250 g | — | Tu proveedor habitual |
| Analizador lógico, pie de rey, tornillo de banco | 1 de cada | Herramientas |

### 7.3 Pendiente de comprobar

- **Micro:** el CH32V003F4P6 (C5187096) figura agotado en JLCPCB y con stocks contradictorios en LCSC. Confirma el stock en la herramienta de BOM de JLCPCB al subir el pedido.
- **Compatibilidad T821 ↔ T812:** misma serie Amphenol, pero no he visto confirmada la llave. Pruébalo con unidades sueltas antes de montar los 10 módulos.
- **Altura enchufada:** header + hembra tiene que caber en los 20 mm de zona trasera.
- **Envío gratis:** Farnell España da 75 € en las fichas que vi, y otras indican 80 € o 95 €. Míralo en el pago.
- **Alimentar por la SuperMini:** no he verificado cuánta corriente aguanta su pin de 5 V, ni qué pasa si enchufas a la vez su USB-C.
- **Pines del ESP32:** `04_diseno_software.md` no fija TX, RX ni los cuatro SENSE. GPIO5 y GPIO6 son pines de strapping en la SuperMini.
- **LED RGB:** el diseño ya usa ánodo común con un MOSFET por color. Comprueba que el LED comprado es de ánodo común y prueba uno en protoboard con las resistencias de `03_diseno_pcb.md` §4 antes de pedir las placas.
- **2N7002 en JLCPCB:** el código es C8545 (JSCJ), confirmado como Basic. Falta comprobar su stock.
- **PWM en PC6, PC7 y PD0:** no está comprobado que los tres tengan PWM por hardware. Si no, el PWM es por software.
- **Adaptador USB-C:** el anuncio no menciona las 5,1 kΩ. Añádelas y mide CC1/CC2 antes de cablear.

---

## 8. Opción en estudio: FFC de 10 hilos en vez de IDC (29/10/2026)

**Estado: no decidido.** Sustituiría el latiguillo IDC (A4, A5, A6) por cable FFC y un conector SMD en la placa del módulo. Cambia `02_diseno_hardware.md` (§3: conector, pinout, llave) y `03_diseno_pcb.md` (huella, serigrafía), y habría que redibujar la placa de módulo.

### 8.1 Piezas

| Pieza | Referencia | Datos | Estado |
|---|---|---|---|
| Cable FFC 10 hilos, tipo A | "10-10-A-0050-C-4-08-4-T" (el que encontraste): 1 mm, 50 mm, refuerzo de 8 mm en cada punta, 60 V, 1 A | Sirve para 10 hilos | Precio no dado. El de 6 hilos costaba 0,70 € |
| Conector FFC 10 contactos, 1 mm | **TE 1-84952-0** (LCSC C590890): bottom contact, SMD, ZIF, **1 A** | Cumple la corriente | Precio, tipo Basic/Extended y si JLCPCB lo monta: **sin verificar** |
| Otros conectores de 1 mm | JUSHUO AFA07-S06FCA-00 (C262712) y S08FCA-00 (C262714), 400–500 mA; Amphenol HFW8R-1STE1LF (C7500641), 1 A pero **sin stock** y de patillas | 6 y 8 contactos | No cumplen o no están disponibles |

### 8.2 Por qué 10 contactos

La corriente por contacto en los conectores FFC de 1 mm es de 0,4–1 A. Con 10 contactos se reparten **4 de 5V, 4 de GND, 1 de DATA y 1 de SENSE**, y el bus de paso aguanta ≈ 4 A con los contactos de 1 A.

### 8.3 Ahorro estimado (10 módulos, matriz, sin IVA)

| | IDC (Farnell) | FFC |
|---|---|---|
| Conectores de placa (40) | 13,52 € | ≈ 3,6 € (a 0,09 €, **precio de un conector de 6 vías**; el de 10 contactos y 1 A costará más) |
| Hembras IDC | 7,22–10,83 € | — |
| Cable | 2,05–4,10 € | ≈ 7–10,5 € (a 0,70 €, **precio del de 6 hilos**) |
| **Total** | **≈ 22,8–28,5 €** | **≈ 10,6–14,1 €**, y subirá con el conector de 10 contactos |

Cálculo mío, sin ficha de precios del conector de 10 contactos.

### 8.4 Qué se pierde con el FFC

- **Llave mecánica (requisito R4):** el cable plano cabe en las dos orientaciones físicas. El pin 1 se distingue solo por la serigrafía y por la posición del conector.
- **Robustez:** el FFC aguanta menos ciclos de enchufado (10–30, según JLCPCB).
- **Prototipo en placa perforada:** el FFC SMD no se suelda a una perforada; el IDC sigue siendo la vía para la base y el prototipo.
- **Orientación:** el tipo A (mismo lado) obliga a colocar los dos conectores con el pin 1 del mismo lado absoluto.

### 8.5 Pendiente antes de decidir

1. Precio y tipo (Basic/Extended) del TE 1-84952-0 en JLCPCB, y que lo monten en Economic.
2. Precio del cable de 10 hilos y que el refuerzo sea de 0,3 mm.
3. Redibujar el pinout de 10 contactos (`02_diseno_hardware.md` §3.2) y la huella en KiCad.
4. Si los 40 + 40 conectores IDC del carrito de Farnell se mantienen, solo valen para la base y el prototipo.

---

## 9. Cómo buscar el LED RGB

El diseño usa un **RGB normal de 5 mm, difuso, de ánodo común y 4 patillas**. No hace falta chip: lo controlan tres MOSFET de la PCB.

**Qué debe cumplir**

| Característica | Valor |
|---|---|
| Tipo | LED RGB de 4 patillas (rojo, verde, azul y un común) |
| **Polaridad** | **Ánodo común** (el pin común va a +5 V). Si es de cátodo común, no sirve tal cual |
| Tamaño | 5 mm (T-1 3/4) |
| Lente | Difusa / esmerilada (ayuda a repartir la luz en la caja) |
| Corriente por color | 20 mA |
| Caída directa aproximada | Rojo ≈ 2 V, verde y azul ≈ 3–3,5 V |

**Qué escribir en el buscador**
- "LED RGB 5mm ánodo común difuso"
- "5mm RGB LED common anode diffused"
- "LED RGB 4 pines ánodo común"

**Cómo reconocer el ánodo común**
- La ficha dice "common anode" o "ánodo común".
- La pata más larga es el común. En el de ánodo común va a +V y cada color se enciende conectando su pata a GND.
- Sin ficha, se comprueba con una pila de 3 V y una resistencia: pila + en la pata larga y − en cada pata corta, y se enciende cada color.

**Qué descartar**
- LEDs "cátodo común" (la pata larga va a GND).
- LEDs con chip integrado tipo WS2812 o SK6812 (direccionables): no se manejan con tres MOSFET.
- RGB de más de 4 patas o de formato SMD.

**Dónde mirar**
- **Farnell:** el MP017043 (5122721) ya está en tu carrito. Falta confirmar si es de ánodo común.
- **Resto:** BricoGeek, AliExpress o LCSC. El precio de un RGB normal es de céntimos.

**Antes de pedir las placas:** compra un par, móntalos en protoboard con un MOSFET 2N7002 y las resistencias de `03_diseno_pcb.md` §4, y comprueba que los tres colores se encienden y que el verde y el azul no quedan apagados.
