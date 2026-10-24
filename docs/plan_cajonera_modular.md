# Cajonera modular inteligente — Plan de diseño

Versión 1.1 · Octubre 2026 · Conexión entre cajones con cable plano IDC (sustituye a los pogo pins)

---

## 0. Requisitos confirmados

| # | Requisito | Decisión de diseño |
|---|---|---|
| R1 | Cada cajón es independiente y tiene su propio micro | CH32V003F4P6 (TSSOP20) en cada módulo |
| R2 | Dirección configurable con jumpers soldados en la PCB | 6 puentes de soldadura → direcciones 1–63 (la 0 se reserva para broadcast) |
| R3 | Detectar en qué posición se añade un cajón | 1 línea SENSE por conector + descubrimiento de topología dirigido por el ESP32 |
| R4 + R5 | Un solo LED RGB por cajón para todo: búsqueda desde HA y luz al abrir | 1 LED direccionable dentro de una caja de luz translúcida en la parte alta del frente del armazón. Ilumina el frontal (visible con el cajón cerrado) y el interior (al abrir). El micro cambia de color según el estado |
| R6 | Final de carrera para detectar la apertura | Microinterruptor con palanca leído por el micro |
| R7 | Cajones cuadrados de 50×50 mm y dobles de 100×50 mm | Retícula de 50 mm, módulos 1×1 y 2×1 |
| R8 | Unión mecánica con cola de milano y eléctrica con cable plano | Los módulos se meten deslizando desde el frente. Por detrás, cada unión lleva un trozo de cable plano de 6 hilos: una coleta soldada en un cajón que se enchufa en un conector macho con carcasa (box header 2×3) del vecino |
| R9 | Hasta 64 cajones | Bus compartido, 6 bits de dirección |
| — | Integración con Home Assistant | ESP32-C3 en un módulo base (ESPHome o MQTT) |

**¿Hace falta un expansor de GPIO?** No. El CH32V003F4P6 tiene 18 pines de E/S. Con un solo pin para el LED, el módulo 2×1 necesita 16, así que cabe sin expansor (ver §2.3). Un expansor tipo PCF8574 solo haría falta para un futuro módulo 2×2 (necesita 8 SENSE). Incluso en ese caso, la alternativa más barata es leer la dirección con un divisor resistivo en un pin analógico.

---

## 1. Arquitectura del sistema

```
                 Wi-Fi
 Home Assistant ◄──────► ESP32-C3 (módulo base, posición 0,0)
                              │
                              │  Cable plano de 6 hilos entre cada par de vecinos:
                              │  GND · 5V · DATA (UART half-duplex) · SENSE · GND · 5V
                              ▼
   ┌────────┬────────┬─────────────────┐
   │ Cajón  │ Cajón  │   Cajón 2×1     │   Cada módulo:
   │  #12   │  #07   │      #21        │   · CH32V003 (dirección por jumpers)
   ├────────┼────────┼────────┬────────┤   · 1 LED RGB (frontal + interior)
   │  BASE  │ Cajón  │ Cajón  │ Cajón  │   · Final de carrera
   │ (ESP)  │  #03   │  #04   │  #05   │   · 1 SENSE por conector (cara/unidad)
   └────────┴────────┴────────┴────────┘
```

**Las cuatro líneas del bus:**

| Línea | Cómo va cableada | Función |
|---|---|---|
| 5V | En paralelo en todos los conectores (2 hilos por cable) | Alimentación |
| GND | En paralelo en todos los conectores (2 hilos por cable) | Masa |
| DATA | En paralelo en todos los conectores | Bus compartido UART half-duplex de 3,3 V con salidas open-drain. Lo dirige el ESP32 |
| SENSE | Un pin del micro distinto para cada conector, que llega solo al conector de enfrente | Detectar qué vecino hay en cada lado |

**Cómo se identifica cada módulo:** la **dirección** (los jumpers) dice *quién* es; los **SENSE** dicen *dónde* está.

---

## 2. Hardware

### 2.1 Conexión entre módulos: coletas de cable plano

**Un conector por cada unidad de retícula y por cada cara**, todos en la PCB trasera, cerca de su borde:

| Módulo | Izquierda | Derecha | Arriba | Abajo | Total |
|---|---|---|---|---|---|
| 1×1 (50×50) | 1 | 1 | 1 | 1 | 4 |
| 2×1 (100×50) | 1 | 1 | 2 | 2 | 6 |

**Coleta o conector macho, según la cara:**

| Caras | Qué lleva | Cantidad 1×1 / 2×1 |
|---|---|---|
| Derecha y arriba | **Coleta**: unos 70 mm de cable plano de 6 hilos (paso 1,27 mm) soldado a 6 pads de la PCB, con un **conector hembra IDC 2×3 (FC-6P)** en la punta | 2 / 3 |
| Izquierda y abajo | **Conector macho con carcasa 2×3** (box header IDC, paso 2,54 mm), recto, apuntando hacia atrás | 2 / 3 |

Así, cada unión lleva **un solo cable**: la coleta de un cajón se enchufa en el conector macho de su vecino. Cada cable une solo a dos vecinos, así que el SENSE sigue siendo de punto a punto y la detección automática de posición (§2.9) funciona igual.

**Orden de los pines** (igual en los pads de la coleta y en el conector macho):

| Pin | Señal | Pin | Señal |
|---|---|---|---|
| 1 | GND | 2 | 5V |
| 3 | DATA | 4 | SENSE |
| 5 | GND | 6 | 5V |

GND y 5V van por dos hilos cada uno, para repartir la corriente (cada hilo aguanta ~1 A).

**Para que nunca se conecte mal:**

- **Llave:** la carcasa del conector macho tiene una muesca y el conector hembra una pestaña, así que solo entra en una posición.
- **Orientación:** todos los conectores macho se colocan igual en la PCB (muesca hacia el centro de la placa).
- **Cable:** al soldar la coleta, el **hilo rojo del cable va al pin 1 (GND)**. Con eso, el pin 1 de cada coleta siempre llega al pin 1 del vecino.

**Longitud de la coleta:** ~70 mm. Basta para llegar al conector del vecino a través de la muesca de la pared, con algo de holgura para enchufar. Al ser corta, solo puede llegar a un cajón adyacente, así que no hay riesgo de unir dos cajones que no están al lado. La longitud exacta se ajusta con la pieza de prueba T2 (§3.7).

**No hace falta unir todas las caras.** Basta con que cada cajón esté conectado con la base a través de alguna cadena de cables. Solo se pierde la detección de vecinos en los lados que se dejen sin cable. Las coletas que no se usan (cajones del borde) se guardan en un clip impreso.

### 2.2 Microcontrolador: CH32V003F4P6

| Característica | Valor |
|---|---|
| Núcleo | RISC-V a 48 MHz con oscilador interno (no necesita cristal) |
| Memoria | 16 KB de flash, 2 KB de RAM |
| E/S | 18 pines de E/S, alimentación de 2,7 a 5,5 V |
| Precio | 0,31 $ en JLCPCB para 1–49 unidades (C5187096); 0,24 $ a partir de 50 |
| Programación | Un solo hilo (SWIO, pin PD1) con un WCH-LinkE |
| SDK | ch32v003fun (recomendado) o el core de Arduino para CH32 |

### 2.3 Presupuesto de pines y reparto

| Función | 1×1 | 2×1 |
|---|---|---|
| DATA (bus) | 1 | 1 |
| LED RGB (WS2812B o SK6812) | 1 | 1 |
| Final de carrera | 1 | 1 |
| Dirección (6 jumpers) | 6 | 6 |
| Jumper de tamaño (abierto = 1×1, cerrado = 2×1) | 1 | 1 |
| SENSE | 4 | 6 |
| **Total** | **14** | **16** |
| Disponibles (sin contar PD1 para programar ni PD7 de reset) | 16 | 16 |

**Asignación de pines** (la misma PCB para los dos tamaños):

| Pin | Función | Notas |
|---|---|---|
| PD1 | SWIO | Solo programación, no tocar |
| PD7 | NRST | Se deja como reset |
| PD5 | DATA | TX de la USART1 en modo half-duplex. Resistencia de 1 kΩ en serie hasta el conector |
| PC6 | LED RGB | MOSI del SPI, que ch32v003fun usa con DMA para los WS2812 |
| PD6 | Final de carrera | Entrada con pull-up; a 0 cuando el cajón está cerrado |
| PC0 | Dirección, bit 0 | Puente a GND, con pull-up interna |
| PC1 | Dirección, bit 1 | Igual que el bit 0 |
| PC2 | Dirección, bit 2 | Igual que el bit 0 |
| PC3 | Dirección, bit 3 | Igual que el bit 0 |
| PC4 | Dirección, bit 4 | Igual que el bit 0 |
| PD0 | Dirección, bit 5 | Igual que el bit 0 |
| PC5 | Jumper de tamaño | Abierto = 1×1, cerrado = 2×1 |
| PA1 | SENSE 1 | Izquierda |
| PA2 | SENSE 2 | Derecha |
| PD2 | SENSE 3 | Arriba, unidad 0 |
| PD3 | SENSE 4 | Abajo, unidad 0 |
| PD4 | SENSE 5 | Arriba, unidad 1 (solo 2×1) |
| PC7 | SENSE 6 | Abajo, unidad 1 (solo 2×1) |

PA1 y PA2 son los pines del cristal externo. Como se usa el oscilador interno, quedan libres para usarse como pines normales. Cada SENSE lleva una resistencia de 1 kΩ en serie, que protege el pin si se enchufa un cable con el cajón encendido.

**Tabla de conectores de cada tipo** (la usa el ESP32 para calcular posiciones):

| SENSE | Cara | Unidad (1×1) | Unidad (2×1) |
|---|---|---|---|
| 1 | izquierda | 0 | 0 |
| 2 | derecha | 0 | 0 |
| 3 | arriba | 0 | 0 |
| 4 | abajo | 0 | 0 |
| 5 | arriba | — | 1 |
| 6 | abajo | — | 1 |

### 2.4 Dirección por jumpers

- **Cómo se lee:** son 6 puentes de soldadura (JP0–JP5) a GND. Un puente **cerrado vale 1**: dirección = suma de los bits de los puentes cerrados. El micro la lee al arrancar.
- **Dirección 0** (todos abiertos): queda como "sin configurar". El cajón parpadea y el ESP32 avisa en HA.
- **Serigrafía:** pon en la PCB el peso de cada puente (1, 2, 4, 8, 16, 32) y una casilla para escribir la dirección a rotulador.
- **Direcciones duplicadas:** si dos módulos tienen la misma dirección, sus respuestas chocan en el bus y el CRC falla. El ESP32 lo detecta y lo notifica en HA ("dirección 12 duplicada").

### 2.5 Iluminación

**Un solo LED RGB por cajón**, colocado dentro de una **caja de luz** translúcida en la parte alta del frente del armazón (§3.5). La caja tiene dos caras que dejan pasar la luz:

- **La frontal**, que se ve siempre, también con el cajón cerrado. Sirve para la búsqueda y para los avisos de estado.
- **La inferior**, que da al hueco del cajón. Cuando lo abres, ilumina el interior.

**El micro elige el color según el estado:**

| Estado | Color / efecto | Quién lo activa |
|---|---|---|
| Reposo | Apagado (o un tenue "piloto", si lo quieres) | — |
| Buscado desde HA | Azul parpadeando | ESP32 → `SET_LED` |
| Cajón abierto | Blanco | El propio micro, al leer el final de carrera |
| Abierto y además buscado | Verde ("encontrado") durante 2 s y luego blanco; la búsqueda se cancela | El propio micro |
| Sin dirección (todos los jumpers abiertos) | Ámbar parpadeando | El propio micro |
| Error (dirección duplicada, contacto malo) | Rojo parpadeando | ESP32 |
| Identificar | Arcoíris o parpadeo rápido | ESP32 → `IDENTIFY` |

**Qué LED usar:**

| Opción | Ventaja | Inconveniente |
|---|---|---|
| **WS2812B** (5050) | Lo más barato (~0,03–0,05 €); vale un trozo de tira | El blanco sale algo azulado y con poca luz |
| **SK6812 RGBW** (5050) | Tiene un chip blanco aparte: el interior queda mucho mejor iluminado (~0,10 €) | Usa tramas de 32 bits en lugar de 24 (lo resuelve el firmware) |

Recomendación: **SK6812 RGBW** si te importa ver bien los componentes al abrir; WS2812B si prima el precio. El pin y el circuito son los mismos.

**En el 2×1**, la caja de luz mide unos 93 mm. Un solo LED en el centro deja los extremos más oscuros. Se pueden encadenar **2 LEDs en el mismo pin**: el firmware los trata como uno solo y les envía siempre el mismo color.

**Detalles eléctricos:**

- **Placa del LED:** va en una plaquita (en el prototipo, un trozo de tira) unida a la PCB principal con 3 hilos (5V, GND, DIN).
- **Señal de datos:** el LED a 5 V pide en teoría 0,7 × VDD = 3,5 V en la entrada, y el micro da 3,3 V. Para tener margen, **alimenta el LED a través de un diodo** (1N4148 o SS14). Baja a unos 4,3–4,6 V y el umbral queda por debajo de 3,3 V.
- **Brillo:** limítalo por firmware (~30 % para los colores de búsqueda; el blanco al abrir puede ir más alto) y como máximo unos 10 cajones encendidos a la vez (ver §2.7).

### 2.6 Final de carrera

- **Componente:** microinterruptor mini con palanca (tipo KW10), montado en la pared trasera. El fondo del cajón lo pulsa cuando está cerrado.
- **Conexión:** contacto normalmente abierto entre PD6 y GND. Cajón cerrado = pulsado = 0; abierto = 1.
- **Rebotes:** se filtran por software (20–30 ms). Un condensador de 100 nF es opcional.
- **Funciona sin el ESP32:** el micro pone el LED en blanco por sí solo al abrir, aunque el ESP32 no responda.

### 2.7 Alimentación

| Parte | Componentes |
|---|---|
| Entrada (en la base) | USB-C 5 V / 3 A + polyfuse de 2 A + condensador de 1000 µF + diodo TVS de 5 V |
| Por módulo | LDO de 3,3 V (XC6206P332 o AP2112K-3.3) solo para el micro; condensadores de 10 µF y 100 nF. **Sin** condensadores grandes, para limitar el pico de corriente al enchufar un módulo en marcha |
| Consumo en reposo | ~3–5 mA por módulo (64 módulos ≈ 0,3 A) |
| Consumo con LED | ~20–40 mA por LED encendido con el brillo limitado |

**Caída de tensión:** cada salto entre módulos pasa por dos contactos IDC y unos centímetros de cable plano (unos 20–40 mΩ por línea, con los hilos de 5V y GND doblados). Con muchos saltos y mucha corriente, los cajones más lejanos reciben menos de 5 V. El micro sigue funcionando (el LDO aguanta hasta ~3,6 V), pero los LEDs pierden brillo. Las medidas para evitarlo:

1. Un límite de brillo y de LEDs encendidos a la vez en el ESP32.
2. Colocar la base en el centro de la cajonera, no en una esquina.
3. Si la cajonera crece mucho, un segundo módulo de alimentación (como la base, pero sin ESP32).

### 2.8 Bus DATA y protocolo

**Capa eléctrica:**

- UART half-duplex de 3,3 V a 115200 baudios.
- Salidas open-drain, con un único pull-up de 4,7 kΩ en la base.
- En el ESP32: RX directo a la línea; TX a través de un diodo BAT54 con el cátodo hacia el TX.

**Formato de trama:**

```
[0xA5] [DIR] [CMD] [LEN] [DATOS...] [CRC8]
```

- `DIR = 0`: broadcast. `DIR = 1–63`: un módulo concreto.
- Solo habla el módulo al que se ha preguntado, así que no hay colisiones (salvo con direcciones duplicadas).

| Comando | Qué hace |
|---|---|
| `PING` | El módulo responde con su dirección, su tamaño, la versión de firmware y el estado del cajón (abierto/cerrado) |
| `SET_LED` | Cambia el color o el efecto del LED (fijo, parpadeo, fundido, apagado). Al abrir el cajón, el micro tiene prioridad y lo pone en blanco |
| `GET_STATE` | Devuelve el estado del cajón y los avisos pendientes (por ejemplo, "vecino nuevo") |
| `SENSE_DRIVE n` | Pone a 0 el SENSE número n |
| `SENSE_RELEASE` | Suelta todos los SENSE |
| `SENSE_QUERY` | "¿Ves algún SENSE a 0?" → el módulo responde con su dirección y el número de SENSE, o no responde |
| `IDENTIFY` | Hace parpadear el cajón para localizarlo físicamente |

**Sondeo:** el ESP32 pregunta a todas las direcciones conocidas cada ~100 ms. Así detecta aperturas con unos 0,1 s de retraso. Cada cierto tiempo también pregunta por las direcciones libres, para descubrir módulos nuevos.

### 2.9 Descubrimiento de la posición (SENSE)

**Parte eléctrica:**

- En reposo, cada SENSE es una entrada con pull-up y lee 1.
- Para avisar, el micro pone su pin a 0. Nunca lo pone a 1 a la fuerza, así que dos módulos que lo activen a la vez no provocan un cortocircuito.

**Algoritmo** (en el ESP32):

1. La base está en (0,0) y es el punto de partida.
2. Para cada módulo conocido y cada uno de sus SENSE:
   1. Envía `SENSE_DRIVE` al módulo.
   2. Envía `SENSE_QUERY` en broadcast; como mucho responde un módulo.
   3. Envía `SENSE_RELEASE`.
3. Con cada pareja de conectores encontrada calcula la posición del vecino, usando la tabla de §2.3. Por ejemplo, para la cara de arriba:
   - `B.x = A.x + iA − iB`
   - `B.y = A.y + 1` (el alto de cualquier módulo es 1)

   Las caras izquierda y derecha funcionan igual, cambiando x por y.
4. Recorre así todos los módulos, empezando por los vecinos de la base.
5. Si dos módulos caen en la misma celda o una conexión no es recíproca, lo marca como "contacto defectuoso".

**Coste:** unos 64 módulos × 6 SENSE × ~10 ms ≈ 4 s para un mapa completo. Solo se hace al arrancar o cuando hay cambios.

**Detección de un cajón recién añadido:**

1. Al recibir corriente, el módulo nuevo pone todos sus SENSE a 0 durante 50 ms.
2. Sus vecinos lo detectan con una interrupción y guardan el aviso "vecino nuevo en el SENSE k".
3. El ESP32 ve el aviso en el siguiente sondeo, descubre la dirección nueva y repite el descubrimiento solo en esa zona.
4. En HA aparece "Cajón #21 (2×1) añadido en (3,1)".

### 2.10 Módulo base

| Elemento | Detalle |
|---|---|
| Carcasa | Un módulo 1×1 con los mismos conectores (2 coletas + 2 conectores macho) y la misma cola de milano. Lleva cajón, aunque más bajo |
| ESP32 | ESP32-C3 SuperMini. Pines usados: TX/RX para el bus, 4 SENSE propios y 1 LED de estado |
| Entrada de alimentación | USB-C en la cara trasera o inferior |
| Firmware | ESPHome con un componente externo (recomendado) o Arduino + MQTT con descubrimiento automático en HA |

### 2.11 PCBs (versión final)

Todas van en **un solo pedido a JLCPCB**, en placa de 1,6 mm. Basta con el acabado estándar (HASL): ya no hay pads de contacto que necesiten ENIG.

| Placa | Tamaño aprox. | Cantidad por módulo | Contenido |
|---|---|---|---|
| Principal 1×1 | 44×44 mm | 1 (en el 1×1) | Micro, LDO, jumpers, pads de programación, final de carrera, 2 conectores macho (izquierda y abajo) y 2 juegos de 6 pads para las coletas (derecha y arriba) |
| Principal 2×1 | 94×44 mm | 1 (en el 2×1) | El mismo circuito, con los SENSE 5 y 6 montados: 3 conectores macho y 3 juegos de pads para coletas |
| LED | 12×8 mm (1×1) / 30×8 mm (2×1, con 2 LEDs) | 1 | SK6812 RGBW 5050 mirando hacia abajo y en ángulo hacia delante + diodo |

**Detalles de diseño de la placa principal:**

- **Cara delantera** (hacia el cajón): componentes SMD y el final de carrera, que asoma por una ventana del tabique (§3.3).
- **Cara trasera:** conectores macho y pads de las coletas. Los conectores de los lados quedan a unos 3 mm del borde de la placa y centrados en la franja de 6–22 mm de su unidad (§3.1).
- **Fijación de las coletas:** además de soldadas, sujetas con un puente impreso o una gota de silicona caliente, para que el cable no tire de las soldaduras.
- **Programación:** 3 pads (3V3, GND, SWIO) para programar con unas puntas pogo, sin soldar conector.
- **Placa del LED:** unida a la principal con cable de silicona de 26 AWG soldado (3 hilos).
- **Jumpers accesibles:** la dirección tiene que poder verse y cambiarse desde detrás.

### 2.12 Lista de materiales

**Prototipo (fase H1–H2):** 3 módulos 1×1, 1 módulo 2×1 y la base.

| Componente | Cantidad | Precio aprox. |
|---|---|---|
| Placa de desarrollo CH32V003F4P6 | 6 | 9 € |
| Programador WCH-LinkE | 1 | 4 € |
| ESP32-C3 SuperMini | 2 | 6 € |
| Conector macho con carcasa 2×3 (box header IDC, 2,54 mm, recto) | 20 | 1–2 € |
| Conector hembra IDC 2×3 (FC-6P) | 20 | 1–2 € |
| Cable plano de 6 hilos, paso 1,27 mm | 2 m | 1–2 € |
| Tira SK6812 RGBW de 60 LEDs/m, 1 m (o WS2812B, unos 4 €) | 1 | 6–8 € |
| Microinterruptores mini KW10 | 10 | 2 € |
| Módulos AMS1117-3.3 (en el prototipo, en vez del LDO SMD) | 6 | 2 € |
| Diodos 1N4148 o SS14, BAT54 | pack | 1 € |
| Polyfuse de 2 A, condensador de 1000 µF, TVS de 5 V | pack | 2 € |
| Surtido de resistencias y de condensadores cerámicos | 1 + 1 | 5 € |
| Placa perforada de 5×7 cm | 10 | 3 € |
| Cable de silicona de 26 AWG | 1 juego | 5 € |
| Insertos de calor M2 + tornillos M2 | pack | 4 € |
| Analizador lógico de 8 canales (muy recomendable) | 1 | 6 € |
| **Total** | | **~55–65 €** (+ filamento) |

**Coste estimado por módulo en la versión final con PCB:**

| Componente | 1×1 | 2×1 |
|---|---|---|
| CH32V003F4P6 + LDO + componentes pasivos | 0,50 € | 0,50 € |
| LED SK6812 RGBW + diodo (1 en el 1×1, 2 en el 2×1) | 0,12 € | 0,22 € |
| Microinterruptor | 0,05 € | 0,05 € |
| Conectores macho, conectores IDC y cable plano (2 coletas en el 1×1, 3 en el 2×1) | 0,35 € | 0,55 € |
| Parte proporcional de las PCBs | 0,40 € | 0,60 € |
| Filamento (~75 g en el 1×1, ~140 g en el 2×1) | 1,30 € | 2,40 € |
| **Total** | **~2,7 €** | **~4,3 €** |

### 2.13 Fases del hardware

| Fase | Objetivo | Criterio de éxito |
|---|---|---|
| H0 | Compras | Conectores macho, hembra IDC y cable recibidos y **medidos** (alto del conector enchufado, radio de doblado del cable) |
| H1 | Protoboard: ESP32 + 2 placas CH32V003, bus DATA, LEDs, interruptor, jumpers y SENSE entre los dos | `PING`, `SET_LED` y un descubrimiento correcto de 2 módulos, comprobado con el analizador lógico |
| H2 | Montaje en placa perforada dentro de los armazones impresos v1 (§3). Los conectores macho y las coletas se sueldan directamente en la placa perforada | 5 módulos con el mapa correcto en HA; enchufar y desenchufar un cajón en marcha sin que se reinicie nada |
| H3 | Diseño en KiCad de las placas (principal 1×1, principal 2×1 y LED) y pedido a JLCPCB | Las placas encajan en el armazón v2 sin retocar nada |
| H4 | Firmware estable + componente de ESPHome + panel en HA | Buscar por texto o por voz enciende el cajón correcto; abrirlo enciende la luz interior |

---

## 3. Diseño 3D

### 3.1 Herramienta y parámetros

**Herramienta:** OpenSCAD (paramétrico; todo se recalcula al cambiar una medida) o FreeCAD/Fusion con una tabla de parámetros.

**Ejes:**

| Eje | Dirección |
|---|---|
| X | Ancho |
| Y | Alto |
| Z | Profundidad, que es también el sentido en que deslizan los módulos y el cajón. Z = 0 en el frente |

**Parámetros principales** (valores iniciales):

| Parámetro | Valor | Notas |
|---|---|---|
| `U` (unidad de retícula) | 50 mm | 1×1 = 50×50 y 2×1 = 100×50 de frente |
| `D` (profundidad) | 130 mm | Se puede cambiar. Sube de 120 a 130 mm para compensar la zona trasera |
| `pared` | 2,0 mm | En PETG |
| `cdm_base` / `cdm_punta` / `cdm_alto` | 8 / 12 / 3 mm | Cola de milano: ancho en la base, en la punta y altura |
| `cdm_holgura` | 0,25 mm por lado | Ajustar con las piezas de prueba |
| `cdm_pos` | 35 mm | Centro de la cola de milano, medido desde el inicio de cada unidad |
| `zona_trasera` | 20 mm | Hueco abierto detrás de la PCB para los conectores, las coletas y los conectores enchufados |
| `tabique` | 2 mm | Pared interior en la que se atornilla la PCB, a `D − zona_trasera` del frente |
| `muesca_pos` | 6–22 mm | Franja de cada unidad (medida desde su inicio) donde van el conector y la muesca de paso del cable |
| `muesca` | 16 × 12 mm | Ancho × profundidad de la muesca en el borde trasero de cada pared, por donde pasa la coleta al vecino |
| `coleta` | 70 mm | Longitud de la coleta (ajustar con la pieza de prueba T2) |
| `holgura_cajon` | 0,35 mm por lado | — |
| `franja_luz` | 6 mm | Altura de la caja de luz (parte alta del frente del armazón, por encima del cajón) |
| `caja_luz_fondo` | 15 mm | Profundidad de la caja de luz desde el frente |
| `zona_interruptor` | 8 mm | Espacio delante del tabique para el final de carrera |

**Regla para que todo encaje:** las posiciones de la cola de milano, el conector y la muesca se miden siempre **desde el inicio de cada unidad de retícula**, en todas las caras. Así un 2×1 encaja con dos 1×1 en cualquier posición, y la muesca de un cajón siempre coincide con la de su vecino. La cola de milano (en 35 mm) y la muesca (de 6 a 22 mm) no se solapan en ninguna unidad.

### 3.2 Armazón

**Cola de milano en las cuatro caras exteriores:**

- **Derecha y arriba:** la lengüeta sobresale 3 mm. Al ir hacia fuera, el paso entre módulos se mantiene en 50 mm exactos.
- **Izquierda y abajo:** la ranura entra 3 mm en la pared. Ahí la pared se engrosa formando una costilla interior de unos 5 mm.
- **La costilla de abajo hace de guía del cajón**, que lleva una ranura debajo. La costilla de la izquierda solo resta unos 2,5 mm de ancho útil.
- **En el 2×1:** dos lengüetas arriba y dos ranuras abajo (una por unidad), y una a cada lado.

**Tope y retén:**

- La ranura está cerrada en los últimos 3 mm del fondo. Así el módulo que entra se detiene cuando su frente queda alineado con el de los vecinos.
- Un **retén elástico**, un saliente pequeño en la ranura que encaja en un hueco de la lengüeta, evita que el módulo se salga hacia delante al tirar del cajón.

**Facilidad de montaje:**

- Los primeros 5 mm de cada ranura se ensanchan en embudo para que la lengüeta entre fácilmente.

**Zona trasera** (ver §3.3): abierta por detrás, con una muesca por unidad y por cara en el borde trasero de las paredes.

**Tabique de la PCB:**

- Soportes para la PCB principal (44×44 mm en el 1×1, 94×44 mm en el 2×1) con insertos M2, por la cara trasera del tabique.
- Una ventana para que el final de carrera asome hacia el cajón.

**Interior:**

- La caja de luz en la parte alta del frente (§3.5).
- Un canal para el cable de la placa del LED, desde el tabique hasta la caja de luz.

**Medidas útiles aproximadas del hueco (1×1):** 43,5 mm de ancho × 40 mm de alto × 100 mm de fondo (D = 130 mm menos zona trasera, tabique y zona del interruptor).

### 3.3 Zona trasera: conectores, coletas y guías

```
 vista trasera de dos cajones 1×1 (sin tapa)

  ┌─────────────────────┬─────────────────────┐
  │        [▤] ←coleta  │        [▤]          │   [▤] conector macho 2×3
  │   ┌───────────┐ ║   │   ┌───────────┐     │   ═ / ║ coleta de cable plano
  │[▤]│  PCB  A   │═╪═══╪═▶[▤] PCB  B   │═╗   │   ▶ conector hembra IDC enchufado
  │   └───────────┘     │   └───────────┘ ║   │
  │        ║            │        [▤]      ╚═clip (coleta sin usar)
  └────────╨────────────┴─────────────────────┘
          muesca → hacia el cajón de abajo
```

- **Profundidad:** la PCB queda en el tabique, a 20 mm (`zona_trasera`) del borde trasero. Los conectores, enchufados, ocupan unos 16–18 mm y quedan dentro del contorno del módulo, así que la cajonera puede apoyarse contra la pared.
- **Muescas:** en el borde trasero de cada pared, una por unidad, en la franja de 6–22 mm. Como la muesca está en la misma posición en todos los módulos, la de un cajón siempre coincide con la de su vecino y forma un paso para el cable.
- **Guías:** cada coleta va desde sus pads hasta la muesca por un canal impreso, con un puente que la sujeta cerca de la soldadura.
- **Clip:** junto a cada coleta, un clip para recogerla cuando no se usa.
- **Tapa trasera (opcional):** una tapa a presión que tapa los cables. Debe poder quitarse para enchufar y desenchufar.

**Acceso para enchufar.** Las coletas se enchufan por detrás, así que al añadir un cajón hay que poder llegar a la parte trasera de la cajonera. Hay dos opciones:

1. Montar la cajonera sobre una base o carril de la que se pueda sacar o girar.
2. Usar coletas más largas (~150 mm) y **enchufar antes de deslizar**: sacas por el hueco libre la coleta del vecino, la enchufas en el conector del cajón nuevo y lo deslizas en su sitio; el cable se pliega en la zona trasera. Se comprueba con la pieza de prueba T2.

### 3.4 Cajón

**Medidas:** las del hueco menos la holgura de 0,35 mm por lado.

- **1×1:** ~42,8 × 38,5 × 98 mm.
- **2×1:** el ancho pasa a ~92,8 mm.

**Frontal:**

- Cubre el frente desde abajo hasta justo debajo de la caja de luz. En el 1×1 mide unos 49,4 × 41,5 mm, de modo que la franja de luz de 6 mm queda siempre a la vista.
- 0,3 mm más pequeño que la retícula por cada lado, para que se vea la junta entre cajones.
- Un hueco para meter el dedo y tirar.
- Un portaetiquetas.
- El cajón se imprime entero en un solo material opaco: la luz va en el armazón, no en el cajón.

**Resto del cajón:**

- **Debajo:** la ranura que encaja en la costilla guía y un pequeño saliente que lo mantiene cerrado.
- **Detrás:** una superficie plana que pulsa el final de carrera, y un tope elástico que impide que se salga entero.
- **Interior:** opcionalmente, ranuras para separadores (2 o 3 compartimentos).

### 3.5 Caja de luz (un solo LED RGB)

```
 corte lateral (frente a la izquierda)

   ┌──────────────────┐ ← techo del armazón (opaco)
   ║░░░░  ◣ LED       │   caja de luz: 6 mm de alto × 15 mm de fondo
   ║░░░░░░░░░░░░░░░░░░│ ← cara inferior translúcida (hacia el cajón)
   ║ frontal del cajón
   ║   (opaco)       ...
   ↑
   cara frontal translúcida (visible siempre)
```

- **Dónde va:** ocupa los 6 mm de arriba del frente del armazón, a todo el ancho, por encima del cajón.
- **Cómo es:** una pieza aparte impresa en **PETG natural o PLA transparente**, con forma de "L": cara frontal y cara inferior de 0,8 mm. Se encaja a presión en el armazón, que es opaco, así que la luz no pasa a los cajones vecinos.
- **El LED:** va al fondo de la caja (a unos 12–15 mm del frente), en ángulo de unos 45°, mirando hacia abajo y hacia delante. Con eso, una sola fuente ilumina la cara frontal y la cara inferior.
- **Interior de la caja:** píntalo de blanco (o imprímelo en blanco) para que la luz rebote y se reparta de forma uniforme.
- **Con el cajón cerrado**, se ve la franja frontal encendida (búsqueda o estado).
- **Con el cajón abierto**, la cara inferior deja caer la luz blanca dentro del cajón.
- **En el 2×1**, dos LEDs encadenados, a ¼ y ¾ del ancho, para que la franja se ilumine de forma uniforme.

### 3.6 Impresión

| Pieza | Material | Orientación | Notas |
|---|---|---|---|
| Armazón | PETG | Apoyado sobre la cara trasera, con la boca hacia arriba | Las colas de milano quedan verticales: sin soportes y con la mejor precisión. Usa 4 perímetros |
| Cajón | PLA o PETG | Sobre su base | Un solo material, sin cambios de filamento |
| Caja de luz | PETG natural o PLA transparente | Tumbada sobre la cara frontal | Paredes de 0,8 mm (2 perímetros), con el 100 % de relleno |
| Piezas de prueba | El mismo material que la pieza final | Igual que la pieza final | Imprescindibles antes del primer armazón |

### 3.7 Piezas de prueba (antes del primer armazón)

| Prueba | Qué comprobar |
|---|---|
| T1: cola de milano de 20 mm de largo, macho y hembra, con 3 holguras (0,15 / 0,25 / 0,35 mm) | Que deslice con la mano y sin juego. Elegir la holgura definitiva |
| T2: zona trasera de dos módulos (tabique, conector macho, coleta y muescas), en sección corta | Que la coleta llegue al vecino por la muesca, se enchufe con la mano, quepa con la tapa puesta y se recoja en el clip. Ajustar la longitud de la coleta |
| T3: tramo de hueco con su cajón (en sección corta) | Holgura del cajón y pulsación del final de carrera |
| T4: retén de la cola de milano | Fuerza de retención frente a la fuerza de tirar del cajón |

### 3.8 Fases del diseño 3D

| Fase | Objetivo | Depende de |
|---|---|---|
| M1 | Modelo paramétrico base y piezas de prueba T1–T4 | Conectores y cable medidos (H0) |
| M2 | Armazón y cajón 1×1 v1 (para placa perforada) | T1–T3 aprobadas |
| M3 | Caja de luz: probar el ángulo del LED y el espesor de las caras translúcidas | M2 |
| M4 | Versión 2×1 (a partir de los mismos parámetros) | M2 |
| M5 | Módulo base con el ESP32 y la entrada USB-C | M2 |
| M6 | Armazón v2 ajustado a las PCBs definitivas | H3 |
| M7 | Extras: separadores interiores, patas o anclaje a pared, plantilla de etiquetas | — |

---

## 4. Firmware e integración (resumen)

**Firmware del módulo** (ch32v003fun):

- Lee la dirección y el tamaño al arrancar.
- Responde a los comandos del bus (§2.8) y gestiona los SENSE.
- Filtra los rebotes del final de carrera.
- Gestiona los estados del LED (§2.5): blanco al abrir, verde si el cajón abierto estaba buscado, y lo que le pida el ESP32 en el resto de casos.
- Hace los efectos (parpadeo, fundido).

**Firmware del ESP32** (componente externo de ESPHome):

- Sondea el bus, descubre la topología y guarda el mapa.
- Limita el brillo y el número de LEDs encendidos a la vez.
- Publica las entidades en Home Assistant.

**Home Assistant**, por cada cajón:

| Entidad | Uso |
|---|---|
| `light.cajon_NN` | Encender el LED RGB en cualquier color o efecto |
| `binary_sensor.cajon_NN_abierto` | Saber si el cajón está abierto |
| `text.cajon_NN_contenido` | Lista de lo que guarda |
| Sensores de posición X/Y y tamaño | Dibujar la cajonera |

Además:

- **Script "Buscar componente":** busca el texto en los contenidos, hace parpadear los cajones que coinciden y los apaga al abrirlos o a los 30 s.
- **Assist (voz):** frases como "¿dónde están los condensadores de 100 nanos?".
- **Panel:** una cuadrícula dibujada a partir de las posiciones X/Y.

---

## 5. Riesgos y cómo mitigarlos

| Riesgo | Mitigación |
|---|---|
| El módulo se sale hacia delante al tirar del cajón | Retén en la cola de milano (T4) y cajón suave (holgura correcta) |
| Coleta enchufada al revés o en otro cajón | Conector macho con llave; coleta corta que solo llega al vecino; el ESP32 detecta conexiones incoherentes con el SENSE |
| Se rompe un hilo de la coleta de tanto doblarla | Puente de sujeción junto a la soldadura, guía impresa con curvas suaves, coleta fácil de cambiar |
| No se llega a la parte trasera para enchufar | Cajonera desmontable o giratoria, o coletas largas para enchufar antes de deslizar (§3.3) |
| Reinicios al enchufar un módulo en marcha | Poca capacidad de entrada por módulo, 1000 µF en la base, resistencias de 1 kΩ en DATA y SENSE |
| Los LEDs no reconocen la señal de 3,3 V | Alimentarlos a través de un diodo; si hace falta, poner un 74AHCT1G125 |
| Direcciones duplicadas | Detección por CRC y aviso en HA; casilla de dirección en la serigrafía |
| Caída de tensión en cajoneras grandes | Límite de brillo, base en el centro, segundo módulo de alimentación |
| Medidas de los conectores distintas a las esperadas | Diseño paramétrico; medir antes de M1 |

---

## 6. Orden de trabajo recomendado

```
H0 Compras ──► H1 Protoboard ─────────────► H2 Placa perforada ──► H3 PCB ──► H4 Firmware + HA
      │                                        ▲                     │
      └──► M1 Piezas de prueba ──► M2 1×1 ──► M3 Luz ──► M4 2×1 ──► M5 Base ──► M6 Armazón v2 ──► M7
```

Las fases H1 (electrónica en protoboard) y M1–M3 (impresión) se pueden hacer a la vez mientras llegan los componentes. Lo único que tiene que llegar antes de empezar el 3D son los conectores macho, los hembra IDC y el cable plano.
