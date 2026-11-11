# Diseño de hardware

Versión 2.0 · Octubre 2026 · **Arquitectura, componentes y decisiones técnicas**

> **Alcance.** Cómo está construido el sistema: arquitectura eléctrica, elección de componentes, protocolo del bus, alimentación y mecánica. Implementa `01_especificacion_funcional.md`.
>
> El layout concreto de las placas está en `03_diseno_pcb.md`. El firmware, en `04_diseno_software.md`.

---

## 1. Arquitectura

```
                 Wi-Fi
 Home Assistant ◄──────► Controlador ESP32-C3 (módulo base, posición 0,0)
                              │
                              │  Latiguillo de 6 hilos entre cada par de vecinos:
                              │  GND · 5V · DATA · SENSE · GND · 5V
                              ▼
   ┌────────┬────────┬────────┬────────┐
   │ Módulo │ Módulo │ Módulo │ Módulo │   Cada módulo:
   │  #12   │  #07   │  #21   │  #22   │   · 1 micro CH32V003
   ├────────┼────────┼────────┼────────┤   · 1 LED RGB
   │  BASE  │ Módulo │ Módulo │ Módulo │   · 1 final de carrera
   │ (ESP)  │  #03   │  #04   │  #05   │   · 4 conectores (una por cara)
   └────────┴────────┴────────┴────────┘
             #21 y #22 emparejados = un cajón 2×1
```

**Un módulo = una unidad de retícula.** No existe una placa "2×1": un cajón doble son dos módulos idénticos emparejados (§4.3).

### 1.1 Las cuatro líneas del bus

| Línea | Cableado | Función |
|---|---|---|
| **5V** | En paralelo en los 4 conectores (2 hilos) | Alimentación |
| **GND** | En paralelo en los 4 conectores (2 hilos) | Masa |
| **DATA** | En paralelo en los 4 conectores | Bus compartido UART half-duplex de 3,3 V, open-drain. Lo dirige el controlador |
| **SENSE** | **Un pin distinto del micro para cada conector**, que llega solo al conector de enfrente | Detectar qué vecino hay en cada cara |

> ⚠️ **La asimetría entre DATA y SENSE es el fundamento del descubrimiento de posición.** DATA, 5V y GND van en paralelo a los cuatro conectores: son buses. **SENSE no**: son cuatro redes independientes, punto a punto. Si se unieran, el sistema perdería la capacidad de saber por qué cara está cada vecino (requisito **F6**).

---

## 2. El microcontrolador

**CH32V003F4P6** (TSSOP-20), de WCH.

| Característica | Valor |
|---|---|
| Núcleo | RISC-V QingKe V2A a 48 MHz, **oscilador interno** (sin cristal) |
| Memoria | 16 kB de flash, 2 kB de RAM |
| E/S | 18 pines, alimentación de 2,7 a 5,5 V |
| Programación | Un solo hilo (SWIO) con un WCH-LinkE |
| Precio | ~0,15 $ (LCSC C5187096) |
| SDK | `ch32v003fun` |

**Por qué este:** es el micro de 32 bits más barato del mercado, no necesita cristal (lo que libera dos pines para SENSE), y se programa con un solo hilo. A 31 unidades, la diferencia de precio frente a cualquier alternativa es significativa.

**Disponibilidad:** WCH no está en el catálogo de Mouser, Farnell ni RS. Se compra en **LCSC** o AliExpress.

### 2.1 Asignación de pines

| Pin | Señal | Función |
|---|---|---|
| 18 | PD1 | **SWIO** — programación. No usar para nada más |
| 4 | PD7 | **NRST** — reset (pull-up interna) |
| 2 | PD5 | **DATA** — TX de la USART1 en half-duplex |
| 16 | PC6 | **LED rojo** — puerta del MOSFET Q1 (PWM) |
| 3 | PD6 | **Final de carrera** — entrada con pull-up |
| 10 | PC0 | Dirección, bit 0 (peso **1**) |
| 11 | PC1 | Dirección, bit 1 (peso **2**) |
| 12 | PC2 | Dirección, bit 2 (peso **4**) |
| 13 | PC3 | Dirección, bit 3 (peso **8**) |
| 14 | PC4 | Dirección, bit 4 (peso **16**) |
| 8 | PD0 | **LED azul** — puerta del MOSFET Q3 (PWM). Antes era el bit 5 de la dirección: con 31 módulos ya no hace falta |
| 15 | PC5 | **Pareja a la izquierda** |
| 1 | PD4 | **Pareja abajo** |
| 5 | PA1 | **SENSE L** — izquierda |
| 6 | PA2 | **SENSE R** — derecha |
| 19 | PD2 | **SENSE U** — arriba |
| 20 | PD3 | **SENSE D** — abajo |
| 17 | PC7 | **LED verde** — puerta del MOSFET Q2 (PWM) |
| 9 | VDD | +3,3 V |
| 7 | VSS | GND |

**Los 16 pines utilizables están asignados.** PA1 y PA2 son los pines del cristal externo; como se usa el oscilador interno, quedan libres.

**No hace falta expansor de GPIO.** Con un módulo por unidad de retícula, el presupuesto de pines alcanza justo (no queda ninguno libre); un futuro 2×2 serían cuatro módulos normales.

---

## 3. Conexión entre módulos

### 3.1 El principio

> **En la PCB solo se suelda el conector. El cable nunca toca la placa.**

**Cada módulo lleva cuatro box header 2×3 con carcasa** (paso 2,54 mm), uno por cara. El cable entre dos módulos es un **latiguillo de cable plano de 6 hilos con conector hembra IDC en los dos extremos**.

| | Cómo se hace | Si falla |
|---|---|---|
| **La placa** | Box header THT: 6 patillas en taladros metalizados, anclaje mecánico | No falla |
| **El cable** | IDC cerrado **a presión**, sin soldadura en ningún extremo | Se sustituye el latiguillo |

El IDC lleva cuchillas que perforan el aislante del cable plano al cerrarlo con un tornillo de banco. Es el método estándar para cable plano, más fiable que soldar hilos de 1,27 mm a mano.

**Por qué no pads de soldadura** (como contemplaba el diseño inicial): ahorra 12 soldaduras de hilo fino por módulo, evita dibujar una huella propia, hace los cables reemplazables sin desoldar dentro del armazón, y permite que cualquier cara vaya a cualquier vecino. Cuesta ~0,10 € por placa.

### 3.2 Orden de pines

| Pin | Señal | Pin | Señal |
|---|---|---|---|
| 1 | GND | 2 | 5V |
| 3 | DATA | 4 | SENSE |
| 5 | GND | 6 | 5V |

GND y 5V van por dos hilos cada uno, para repartir corriente (~1 A por hilo).

### 3.3 Imposibilidad de conectar mal (requisito R4)

| Medida | Cómo |
|---|---|
| **Llave mecánica** | La carcasa del box header tiene muesca y el IDC pestaña: solo entra en una posición |
| **Orientación uniforme** | Todos los conectores se montan igual en la PCB: muesca hacia el centro |
| **Hilo rojo al pin 1** | En los dos extremos del latiguillo. Así el pin 1 de un módulo siempre llega al pin 1 del vecino |
| **Longitud corta (~70 mm)** | Solo alcanza al vecino inmediato: no se pueden unir dos módulos no adyacentes |

> ⚠️ **Lo que la llave NO protege:** enchufar el latiguillo en la cara equivocada (el de la izquierda en el conector de arriba). Eso produciría un mapa incorrecto. Mitigación: serigrafía `L R U D` en letra grande (ver `03_diseno_pcb.md`) y detección de conexiones no recíprocas por software.

### 3.4 No hace falta conectar todas las caras

Basta con que cada módulo tenga alguna cadena de cables hasta el controlador. Solo se pierde la detección de vecinos en las caras sin cable. Los conectores sin usar quedan libres.

---

## 4. Identificación y configuración de un módulo

Tres cosas distintas, resueltas de tres maneras:

| Qué | Cómo se determina | Mecanismo |
|---|---|---|
| **Dirección** (quién soy) | El usuario, al montar | 5 puentes de soldadura |
| **Posición** (dónde estoy) | **El sistema, solo** | Líneas SENSE |
| **Emparejamiento** (con quién) | El usuario, al montar | 2 puentes de soldadura |

### 4.1 Dirección

Cinco puentes de soldadura a GND (JP0–JP4), leídos con las pull-up internas. **Puente cerrado = bit a 1.** La dirección es la suma de los pesos de los puentes cerrados: 1, 2, 4, 8, 16 → direcciones 1–31.

- **Dirección 0** (todos abiertos) se reserva para broadcast y se interpreta como "sin configurar": el módulo parpadea en ámbar.
- **Direcciones duplicadas:** las respuestas chocan en el bus y el CRC falla. El controlador lo detecta y lo notifica.
- Se lee **una sola vez al arrancar**.

### 4.2 Emparejamiento — puentes direccionales

| Puente | Significado |
|---|---|
| **JP7** | Mi cajón continúa **a la izquierda** |
| **JP8** | Mi cajón continúa **hacia abajo** |

Para un cajón 2×1 horizontal, solo el módulo de la **derecha** cierra JP7.

**Por qué direccionales y no un solo bit "estoy emparejado":** un bit dice que hay pareja, no con quién. Tres módulos en fila con el bit puesto son ambiguos (¿A+B y C, o A y B+C?). Con JP7, el módulo B declara "continúo a la izquierda" → A+B; C no declara nada → C solo.

**Consecuencias útiles:**

- Define el **módulo principal** de cada cajón: el que no declara nada. Es el que da nombre a la entidad en Home Assistant.
- El controlador puede **validarlo contra el mapa**: si un módulo declara pareja donde no hay vecino, es un error de montaje.
- **El 2×2 sale gratis**: cuatro módulos, el de arriba-derecha cierra los dos puentes. No hay "tipos de módulo" que conocer.

---

## 5. Iluminación

**Un LED RGB de 5 mm, de ánodo común, por módulo**, unido a la placa principal con cuatro hilos (5V, R, G, B) a la tira J6. Cada color lo controla un pin del micro a través de un MOSFET.

**Por qué no un LED direccionable (WS2812/SK6812):** necesita en su entrada de datos 0,7 × VDD ≈ 3,5 V alimentado a 5 V, y el micro da 3,3 V. Resolverlo exige un diodo (que lo deja por debajo de su mínimo de 4,5 V) o un buffer. Además, el stock de los LED direccionables de 5 mm con patillas es poco fiable. El RGB normal evita el problema de los niveles.

**Consecuencia:** no hay canal blanco. El "blanco" al abrir es R + G + B, más frío y con menos luz que un RGBW.

### 5.1 Circuito: un MOSFET por color

```
+5V ──────────────────────── ánodo común (J6.1)
                                  │
              ┌───────────────────┼───────────────────┐
         rojo (J6.2)         verde (J6.3)        azul (J6.4)
              │                   │                   │
              R (150 Ω)           R (100 Ω)           R (100 Ω)
              │                   │                   │
            drenador            drenador            drenador
              │                   │                   │
   PC6 ──┤ Q1 (2N7002)   PC7 ──┤ Q2 (2N7002)   PD0 ──┤ Q3 (2N7002)
              │                   │                   │
             GND                 GND                 GND
```

Cada puerta lleva además una resistencia de **100 kΩ a GND**, para que el color quede apagado mientras el micro arranca.

- **Por qué un MOSFET:** el LED va a 5 V y el verde y el azul necesitan ≈ 3 V de caída directa; a 3,3 V casi no darían luz. El MOSFET deja que el pin del micro solo vea 3,3 V y que el LED se alimente desde los 5 V.
- **Lógica directa:** pin a 1 = color encendido. No hay inversión, aunque el LED sea de ánodo común.
- **PWM:** los tres pines se mueven por PWM (hardware o software; ver `04_diseno_software.md`).
- **Valores de las resistencias:** ≈ 150 Ω para el rojo (≈ 2 V de caída) y ≈ 100 Ω para el verde y el azul (≈ 3 V) a 20 mA desde 5 V. Son valores de partida; se ajustan con el LED comprado.
- **Brillo:** el límite de brillo y de LED encendidos a la vez (10) sigue en el controlador.

> **Comprueba que el LED es de ánodo común.** Si compras uno de cátodo común, el circuito no sirve tal cual: habría que invertir la polaridad. Un cambio así no se puede hacer en placas ya montadas.

### 5.2 Caja de luz

El LED va dentro de una caja translúcida en la parte alta del frente del armazón, con dos caras que dejan pasar la luz:

- **La frontal**, visible siempre, también con el cajón cerrado → búsqueda y avisos de estado.
- **La inferior**, que da al hueco del cajón → ilumina el interior al abrir.

El LED de 5 mm (difuso) se orienta a ~45°, mirando hacia abajo y hacia delante, al fondo de la caja (12–15 mm del frente). Se sujeta por sus patillas con los cuatro hilos soldados. El interior de la caja, blanco, para que la luz rebote.

---

## 6. Final de carrera

- **Microinterruptor** con palanca, montado en la pared trasera del armazón. El fondo del cajón lo pulsa al cerrarse.
- **Conexión:** contacto normalmente abierto entre el pin del micro y GND. **Cerrado = pulsado = 0**; abierto = 1.
- **Va por cable a un conector de 2 pines** en la placa (no soldado a la placa), porque la placa es pequeña y flotante y no coincide con la ventana del tabique.
- **Antirrebote por software**, 20–30 ms. El condensador es opcional (se deja en el diseño como *no poblar*).

**En un cajón doble: un cable en Y** desde el microinterruptor a las dos placas. Las dos leen el mismo contacto y cada una enciende su LED por sí sola — así se cumple **R1** en los dos módulos.

---

## 7. Alimentación

### 7.1 Arquitectura

| Parte | Componentes |
|---|---|
| **Entrada (en la base)** | USB-C hembra de 5 V/3 A + polyfuse de 2 A + condensador de 1000 µF + TVS de 5 V. **Alimenta también la SuperMini** por su pin 5V: un solo cable de entrada |
| **Por módulo** | LDO de 3,3 V (XC6206P332, LCSC C5446) solo para el micro, con 100 nF + 10 µF |
| **El LED** | Va a 5 V directamente (ánodo común), no por el LDO. Cada color lo cierra a GND un MOSFET (§5.1) |

### 7.2 La trampa del USB-C

> ⚠️ **Sin las dos resistencias de 5,1 kΩ de CC1 y CC2 a GND, un cargador USB-C moderno no entrega corriente.** Nada. El dispositivo tiene que identificarse como consumidor, y esas dos resistencias son toda la identificación.
>
> **Dos, no una:** con una sola, el cable deja de funcionar al enchufarlo girado. Es un fallo que parece un conector roto.

Elige un conector USB-C de 16 pines con **patas pasantes de anclaje**, no uno solo SMD: en una placa que se enchufa muchas veces, un USB-C solo SMD se arranca y se lleva las pistas.

### 7.3 Consumo

| | |
|---|---|
| Módulo en reposo | 3–5 mA (31 módulos ≈ 0,15 A) |
| Módulo con LED encendido | 20–40 mA con el brillo limitado |
| **Total con 10 LEDs encendidos** | **< 1 A**, frente a 3 A disponibles |

### 7.4 Nada de condensadores grandes por módulo

> ⚠️ **Máximo 10 µF en la entrada de 5 V de cada módulo.** El de 1000 µF va solo en la base.
>
> Si cada módulo llevara un electrolítico grande, enchufar un cajón en marcha produciría un pico de corriente que reiniciaría a los vecinos. Eso incumpliría **R2**, que es criterio de aceptación.

### 7.5 Caída de tensión

Cada salto entre módulos pasa por dos contactos IDC y unos centímetros de cable plano (~20–40 mΩ por línea con los hilos doblados). Con muchos saltos y mucha corriente, los módulos más lejanos reciben menos de 5 V. El micro sigue funcionando (el LDO aguanta hasta ~3,6 V), pero los LED pierden brillo.

**Mitigaciones:**

1. Límite de brillo y de LED encendidos a la vez (10) en el controlador.
2. Colocar la base **en el centro** de la cajonera, no en una esquina.
3. Si la cajonera crece mucho, un segundo módulo de alimentación (como la base, sin ESP32).

---

## 8. Bus DATA

### 8.1 Capa eléctrica

- UART **half-duplex** a 115200 baudios, 3,3 V.
- Salidas **open-drain**, con un **único pull-up de 4,7 kΩ en la base**.
- **Resistencia de 1 kΩ en serie** en cada módulo, que protege el pin si se enchufa un cable con el módulo encendido.
- En el controlador: RX directo a la línea; TX a través de un diodo Schottky (cátodo hacia el TX).

> ⚠️ **Un solo pull-up en todo el sistema.** Con uno por módulo, 31 módulos darían ≈ 150 Ω en paralelo y la línea no podría subir.

### 8.2 Formato de trama

```
[0xA5] [DIR] [CMD] [LEN] [DATOS...] [CRC8]
```

- `DIR = 0`: broadcast. `DIR = 1–31`: un módulo concreto.
- Solo habla el módulo al que se pregunta, así que no hay colisiones (salvo con direcciones duplicadas, que es justo cómo se detectan).

### 8.3 Comandos

| Comando | Qué hace |
|---|---|
| `PING` | Responde con dirección, versión de firmware, emparejamiento y estado del cajón |
| `SET_LED` | Cambia color o efecto (fijo, parpadeo, fundido, apagado) |
| `GET_STATE` | Estado del cajón y avisos pendientes (p. ej. "vecino nuevo") |
| `SENSE_DRIVE n` | Pone a 0 el SENSE número n |
| `SENSE_RELEASE` | Suelta todos los SENSE |
| `SENSE_QUERY` | "¿Ves algún SENSE a 0?" → responde con su dirección y el número de cara |
| `IDENTIFY` | Hace parpadear el módulo para localizarlo |

---

## 9. Descubrimiento de la posición

### 9.1 Capa eléctrica

- En reposo, cada SENSE es una **entrada con pull-up** y lee 1.
- Para avisar, el micro **pone su pin a 0**. Nunca lo fuerza a 1, así que dos módulos activándolo a la vez no provocan cortocircuito.
- **Resistencia de 1 kΩ en serie** en cada SENSE, por la misma razón que en DATA.

### 9.2 Algoritmo (en el controlador)

1. La base está en (0,0).
2. Para cada módulo conocido y cada una de sus cuatro caras:
   1. `SENSE_DRIVE` al módulo, en esa cara.
   2. `SENSE_QUERY` en broadcast → como mucho responde un módulo.
   3. `SENSE_RELEASE`.
3. Con cada par de caras encontrado, calcula la posición del vecino. **Al ser un módulo por unidad de retícula, el cálculo es trivial:**

   | Cara encontrada | Posición del vecino |
   |---|---|
   | arriba | `(A.x, A.y + 1)` |
   | abajo | `(A.x, A.y − 1)` |
   | derecha | `(A.x + 1, A.y)` |
   | izquierda | `(A.x − 1, A.y)` |

4. Recorre toda la cajonera partiendo de los vecinos de la base.
5. Si dos módulos caen en la misma celda, o una conexión no es recíproca, lo marca como error de montaje.

**Coste:** 31 módulos × 4 caras × ~10 ms ≈ **1,2 s** para un mapa completo.

### 9.3 Detección de un módulo nuevo

1. Al recibir corriente, el módulo nuevo **pone todos sus SENSE a 0 durante 50 ms**.
2. Sus vecinos lo detectan por interrupción y guardan el aviso "vecino nuevo en la cara k".
3. El controlador ve el aviso en el siguiente sondeo, descubre la dirección nueva y repite el descubrimiento solo en esa zona.
4. Lo publica en Home Assistant.

---

## 10. Módulo base (controlador)

| Elemento | Detalle |
|---|---|
| Carcasa | Un módulo normal con los mismos cuatro conectores y la misma cola de milano |
| Controlador | **ESP32-C3 SuperMini**. Usa TX/RX para el bus, 4 SENSE propios y un LED de estado |
| Alimentación | Entrada USB-C (§7) |
| Extra | El pull-up de 4,7 kΩ del bus, el diodo Schottky del TX, el polyfuse, el 1000 µF y el TVS |

**Asignación de pines de la SuperMini** (confirmada, la misma en `04_diseno_software.md` §5):

| GPIO | Función | Notas |
|---|---|---|
| GPIO21 (TX) | DATA, a través del Schottky 1N5818 (cátodo hacia el TX) | UART0 TX |
| GPIO20 (RX) | DATA, directo a la línea | UART0 RX |
| GPIO0 | SENSE L (izquierda) | Entrada con pull-up, como en los módulos |
| GPIO1 | SENSE R (derecha) | Idem |
| GPIO3 | SENSE U (arriba) | Idem |
| GPIO10 | SENSE D (abajo) | Idem |
| GPIO8 | LED de estado (en la placa de la SuperMini) | — |

Evitar GPIO2, GPIO5, GPIO6 y GPIO9: son de arranque (strapping) o del botón BOOT.

El pull-up de 4,7 kΩ va de DATA a **3V3 de la SuperMini** (único del sistema). Los cuatro SENSE de la base van a sus cuatro box header, igual que en un módulo.

---

## 11. Mecánica

### 11.1 Parámetros de la retícula

| Parámetro | Valor |
|---|---|
| Unidad de retícula (`U`) | **50 mm** |
| Profundidad (`D`) | 130 mm |
| Pared | 2,0 mm (PETG) |
| Zona trasera | 20 mm (hueco para conectores y cables) |
| Franja de luz | 6 mm en la parte alta del frente |

### 11.2 Unión mecánica

**Cola de milano en las cuatro caras exteriores.** Los módulos se deslizan desde el frente.

- **Derecha y arriba:** la lengüeta sobresale 3 mm, manteniendo el paso de 50 mm exactos.
- **Izquierda y abajo:** la ranura entra 3 mm en la pared, engrosada en una costilla interior.
- **La costilla inferior hace de guía del cajón.**
- **Tope:** la ranura está cerrada en los últimos 3 mm, de modo que el módulo se detiene alineado con sus vecinos.
- **Retén elástico** para que el módulo no salga hacia delante al tirar del cajón.

### 11.3 La regla que hace que todo encaje

> **Las posiciones de la cola de milano, del paso de cable y de la muesca se miden siempre desde el inicio de cada unidad de retícula, en todas las caras.**

Así cualquier módulo encaja con cualquier otro en cualquier posición, y las muescas de dos vecinos siempre coinciden.

### 11.4 Impresión

| Pieza | Material | Orientación |
|---|---|---|
| Armazón | PETG | Apoyado en la cara trasera, boca arriba (colas de milano verticales, sin soportes) |
| Cajón | PLA o PETG | Sobre su base |
| Caja de luz | PETG natural o PLA transparente | Tumbada sobre la cara frontal, paredes de 0,8 mm al 100 % de relleno |

---

## 12. Coste por módulo

| Componente | Coste |
|---|---|
| CH32V003 + LDO + pasivos | 0,50 € |
| LED RGB de 5 mm + 3 MOSFET + resistencias | ≈ 0,15 € (estimación) |
| Microinterruptor | 0,05 € |
| 4 box header + parte del latiguillo | 0,40 € |
| Parte proporcional de la PCB | 0,30 € |
| Filamento (~75 g) | 1,30 € |
| **Total** | **~2,7 €** — cumple **C7** |

Un cajón 2×1 son dos módulos: ~5,4 €, menos el filamento compartido.

---

## 13. Fases y riesgos

### 13.1 Fases del hardware

| Fase | Objetivo | Criterio de éxito |
|---|---|---|
| **H0** | Compras | Conectores y cable recibidos y **medidos** |
| **H1** | Protoboard: controlador + 2 micros, bus, LED, interruptor, puentes y SENSE | `PING`, `SET_LED` y descubrimiento de 2 módulos, verificado con analizador lógico |
| **H2** | Placa perforada dentro de armazones impresos v1 | 5 módulos con mapa correcto en HA; enchufar y desenchufar en marcha sin reinicios |
| **H3** | Diseño de la PCB en KiCad y pedido a JLCPCB | Las placas encajan y funcionan |
| **H4** | Firmware estable + integración en HA | Criterios A1–A11 de la especificación |

### 13.2 Riesgos

| Riesgo | Mitigación |
|---|---|
| El módulo se sale al tirar del cajón | Retén en la cola de milano; holgura correcta del cajón |
| Latiguillo enchufado en la cara equivocada | Serigrafía `L R U D`; detección de conexiones no recíprocas |
| Se rompe un hilo del latiguillo | **Se sustituye el latiguillo** (ya no hay soldaduras que proteger) |
| No se llega a la parte trasera para enchufar | Cajonera desmontable o giratoria, o latiguillos más largos |
| Reinicios al enchufar en marcha | Poca capacidad por módulo, 1000 µF en la base, 1 kΩ en DATA y SENSE |
| El LED RGB comprado no es de ánodo común | Comprobar la polaridad en la ficha y probar uno en protoboard antes de pedir las placas |
| Direcciones duplicadas | Detección por CRC y aviso en HA; casilla de dirección en la serigrafía |
| Caída de tensión en cajoneras grandes | Límite de brillo, base en el centro, segundo módulo de alimentación |
| Medidas de los conectores distintas a las previstas | Diseño paramétrico; **medir antes de modelar** |
| Stock del micro | El CH32V003F4P6 se ha agotado en LCSC alguna vez. Comprar con margen |
