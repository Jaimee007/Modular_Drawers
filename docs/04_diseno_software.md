# Diseño de software

Versión 2.0 · Octubre 2026 · **Firmware del módulo, del controlador e integración en Home Assistant**

> **Alcance.** Qué software corre en cada sitio, cómo se organiza y en qué orden se construye y prueba. Implementa `01_especificacion_funcional.md` sobre el hardware de `02_diseno_hardware.md`.
>
> Entorno: **VS Code + PlatformIO**.

---

## 1. Las tres piezas de software

| Pieza | Dónde corre | Lenguaje / framework |
|---|---|---|
| **Firmware del módulo** | CH32V003 (uno por unidad de retícula) | C sobre `ch32v003fun` |
| **Firmware del controlador** | ESP32-C3 (módulo base) | C++ como componente externo de ESPHome |
| **Integración** | Home Assistant | YAML, plantillas y scripts |

### 1.1 Reparto de responsabilidades

El principio que lo gobierna todo viene del requisito **R1**:

> **Un cajón que se abre se ilumina aunque el controlador no responda.**

De ahí sale la línea divisoria:

| Lo decide **el módulo**, solo | Lo decide **el controlador** |
|---|---|
| Encender en blanco al abrir | Qué cajones están siendo buscados |
| El verde de "encontrado" y cancelar la búsqueda | El mapa de posiciones |
| Parpadear en ámbar si no tiene dirección | Detectar direcciones duplicadas y errores de montaje |
| Filtrar los rebotes del interruptor | Agrupar módulos emparejados en un cajón |
| Reiniciarse si se cuelga | Límites de brillo y de LED simultáneos |

**El módulo tiene prioridad.** Si el controlador pide azul y el cajón se abre, gana el blanco.

---

## 2. Estructura del repositorio

```
Modular_Drawers/
├── firmware/
│   ├── common/bus.h             ← protocolo y CRC, COMPARTIDO
│   ├── module/                  ← proyecto PlatformIO del CH32V003
│   │   ├── platformio.ini       ← un entorno por programa (ver 07_pruebas_banco.md)
│   │   ├── include/funconfig.h
│   │   └── src/phaseN_<función>.c
│   └── base_test/               ← proyecto del ESP32 (pruebas, fases 3-6)
│       ├── platformio.ini
│       └── src/base_<función>.cpp
├── esphome/
│   ├── components/cajonera/     ← componente externo (fase 7)
│   └── cajonera.yaml
└── homeassistant/
    ├── scripts.yaml
    └── dashboard.yaml
```

> **`bus.h` tiene que ser un único archivo compartido** por los dos proyectos. El formato de trama y el CRC duplicados en dos sitios es una fuente garantizada de fallos difíciles de diagnosticar.

### 2.1 Configuración de PlatformIO

**`firmware/module/platformio.ini`** (un entorno por programa; ver `07_pruebas_banco.md`)

```ini
[env:phase3_bus_tx]
platform = https://github.com/Community-PIO-CH32V/platform-ch32v.git
board = genericCH32V003F4P6
framework = ch32v003fun
upload_protocol = minichlink
```

**`firmware/base_test/platformio.ini`**

```ini
[env:base_bus_rx]   ; un entorno por programa: ver 07_pruebas_banco.md
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
build_flags =
  -DARDUINO_USB_MODE=1
  -DARDUINO_USB_CDC_ON_BOOT=1
```

Las dos `build_flags` hacen que el `Serial` del ESP32-C3 SuperMini salga por su USB-C.

**Por qué `ch32v003fun` y no Arduino:** ocupa muy poco (el micro tiene 16 kB de flash y 2 kB de RAM), da acceso directo al UART en modo half-duplex —que es lo que necesita el bus— y permite manejar los tres canales del LED por PWM.

---

## 3. Firmware del módulo

### 3.1 Al arrancar

1. Leer los **5 puentes de dirección** (una sola vez). Si la dirección es 0 → estado "sin configurar".
2. Leer los **2 puentes de emparejamiento** (JP7 izquierda, JP8 abajo).
3. Configurar los 4 SENSE como entradas con pull-up.
4. **Anunciarse:** poner los 4 SENSE a 0 durante 50 ms, para que los vecinos detecten el módulo nuevo.
5. Configurar la USART1 en half-duplex y los tres canales PWM del LED (PC6 = rojo, PC7 = verde, PD0 = azul).
6. Activar el watchdog.

### 3.2 Bus: lo que hay que acertar

| Asunto | Cómo |
|---|---|
| **Modo half-duplex** | Bit HDSEL de la USART1; el pin de DATA como salida de función alternativa **open-drain** |
| **El eco** | En half-duplex el receptor oye lo que transmite el propio micro. Hay que **descartar ese eco** o desactivar la recepción mientras se transmite |
| **Recepción** | Por interrupción, con búfer y máquina de estados que reconoce la trama |
| **Responder** | Solo si `DIR` coincide. Esperar ~100 µs antes de contestar, para que el controlador libere la línea |
| **Trama** | `[0xA5] [DIR] [CMD] [LEN] [DATOS...] [CRC8]` |

### 3.3 Máquina de estados del LED

Implementa la tabla de la especificación §4.1, con esta prioridad:

```
1. Sin dirección (ámbar parpadeante)      ← máxima
2. Cajón abierto (blanco, o verde 2 s si estaba buscado)
3. Lo que pida el controlador (búsqueda, error, identificar)
4. Reposo (apagado)                        ← mínima
```

**Detalle del LED RGB:** son **tres canales PWM** (R, G, B), uno por pin, a través de un MOSFET: pin a 1 = color encendido. No hay canal blanco: el "blanco" es R + G + B a la vez. Hay que comprobar que PC6, PC7 y PD0 admiten PWM por hardware (temporizadores TIM1/TIM2, con remapeo); si no, se hace por software. Los MOSFET dejan los colores apagados mientras el micro arranca, por la resistencia de 100 kΩ de cada puerta.

### 3.4 Final de carrera

- Entrada con pull-up. **Cerrado = pulsado = 0.**
- **Antirrebote por software, 25 ms:** el estado solo cambia si se mantiene estable ese tiempo.
- Al detectar apertura, **encender el LED en blanco inmediatamente**, sin consultar al bus.

### 3.5 SENSE

| Comando | Comportamiento |
|---|---|
| `SENSE_DRIVE n` | Poner a 0 el SENSE de la cara n |
| `SENSE_RELEASE` | Volver todos a entrada con pull-up |
| `SENSE_QUERY` | Si ve algún SENSE a 0 que él no está tirando, responde con su dirección y el número de cara. Si no ve ninguno, **no responde** |

**Interrupción en los SENSE:** al detectar un flanco de bajada que él no ha provocado, guarda el aviso "vecino nuevo en la cara k" para el siguiente `GET_STATE`.

### 3.6 Robustez

| Medida | Detalle |
|---|---|
| **Watchdog (IWDG)** | Si el firmware se cuelga, el micro se reinicia solo |
| **Tiempo sin órdenes** | Si pasan más de 30 s sin recibir nada del controlador, apaga la búsqueda pero **sigue encendiendo la luz al abrirse** |
| **Límite de brillo** | Por defecto en el firmware |

---

## 4. Firmware del controlador

### 4.1 Sondeo

| | |
|---|---|
| Cada ~100 ms | pregunta a todas las direcciones conocidas |
| Reintentos | 2 por sondeo; tras 3 fallos seguidos, marca el módulo como "desconectado" |
| Cada ~5 s | pregunta a direcciones desconocidas, para descubrir módulos nuevos |

Esto da una latencia de detección de apertura de ~0,1 s, holgadamente dentro del medio segundo que pide la especificación.

### 4.2 Descubrimiento de la topología

Implementa el algoritmo de `02_diseno_hardware.md` §9.2. Con un módulo por unidad de retícula, el cálculo de posición es directo:

| Cara | Posición del vecino |
|---|---|
| arriba | `(x, y+1)` |
| abajo | `(x, y−1)` |
| derecha | `(x+1, y)` |
| izquierda | `(x−1, y)` |

**Validaciones que debe hacer:**

| Comprobación | Si falla |
|---|---|
| Dos módulos en la misma celda | error de montaje |
| Conexión recíproca (si A ve a B por la derecha, B debe ver a A por la izquierda) | error de montaje |
| Un módulo declara pareja donde no tiene vecino | error de montaje |
| Un módulo sin camino hasta la base | "sin posición", pero sigue funcionando |

### 4.3 Agrupación de cajones

A partir de los puentes JP7/JP8:

1. Cada módulo que declara "pareja a la izquierda" se une al de `(x−1, y)`.
2. Cada módulo que declara "pareja abajo" se une al de `(x, y−1)`.
3. El **módulo principal** de un grupo es el que no declara nada. Da nombre a la entidad.
4. Un grupo expone **una sola entidad de cajón** en Home Assistant.
5. Una orden de iluminación al grupo se envía a **todos** sus módulos.

### 4.4 Límites

| Límite | Valor |
|---|---|
| LED encendidos a la vez | 10 |
| Brillo máximo | limitado |
| Duración de una búsqueda | 30 s |

---

### 4.5 Pines de la SuperMini

La asignación de pines está **solo** en `02_diseno_hardware.md` §10; esa tabla es la única fuente y el firmware la usa tal cual. No se repite aquí para que no pueda divergir.

Resumen: el bus va por UART0 (GPIO20 RX, GPIO21 TX), los cuatro SENSE por GPIO0, 1, 3 y 10, y el LED de estado es el de la propia SuperMini.

## 5. Integración en Home Assistant

### 5.1 Entidades que publica el componente

Por cada **cajón** (no por módulo):

| Entidad | Uso |
|---|---|
| `light.cajon_NN` | RGB con efectos de parpadeo |
| `binary_sensor.cajon_NN_abierto` | Estado del cajón |
| `text.cajon_NN_contenido` | Descripción; **persistida en el ESP32** (requisito R6) |
| `sensor.cajon_NN_x`, `_y`, `_tamano` | Para dibujar la cajonera |

Y a nivel de sistema: un botón **"Redescubrir cajonera"** y entidades de aviso para cajones nuevos, direcciones duplicadas y errores de montaje.

### 5.2 Búsqueda

Script `buscar_componente`:

1. Recibe un texto ("10k").
2. Lo busca en el `text` de contenido de cada cajón.
3. Hace parpadear en azul los que coinciden.
4. Los apaga al abrirse uno, o a los 30 s.

**Assist (voz):** un intent personalizado del tipo *"¿dónde están las {cosa}?"* que llama al script.

**Panel:** una tarjeta con la cuadrícula dibujada a partir de las posiciones X/Y, el color de cada cajón y su contenido. Al tocar un cajón, se identifica parpadeando.

### 5.3 Plan B

Si el componente externo de ESPHome se complica, se mantiene el firmware Arduino de `base_test` y se publican las entidades por **MQTT con descubrimiento automático**. Funciona igual en Home Assistant.

---

## 6. Plan de construcción y pruebas

El software se construye en **10 fases (0 a 9)**, cada una con su prueba de aceptación: no se pasa a la siguiente hasta que la anterior funcione. El detalle (montaje, pasos y criterios) está **solo** en dos documentos, para que no haya dos versiones:

- **`plan_firmware.md`**: las fases, en orden, con sus pruebas de aceptación.
- **`07_pruebas_banco.md`**: qué programa de `firmware/` prueba cada cosa y con qué entorno de PlatformIO.

| Fase | Objetivo | Hardware |
|---|---|---|
| 0 | Entorno y repositorio | PC, WCH-LinkE |
| 1 | Primer programa: parpadeo y `printf` | 1 micro |
| 2 | Periféricos, uno a uno | + LED, interruptor, puentes |
| 3 | Bus DATA: el controlador habla con un módulo | + ESP32, diodo Schottky, resistencias |
| 4 | Dos módulos en el mismo bus | 2.º micro |
| 5 | SENSE: descubrimiento de posición | Cables entre placas |
| 6 | Emparejamiento, autonomía y robustez | — |
| 7 | Integración con Home Assistant | HA funcionando |
| 8 | Búsqueda, panel y voz | — |
| 9 | Prueba final con cajones impresos | Armazones v1 |

---

## 7. Problemas típicos

| Síntoma | Causa probable | Solución |
|---|---|---|
| "Programming Failed" al grabar | Firmware del WCH-LinkE antiguo, o en modo ARM | Actualizarlo y ponerlo en modo RISC-V |
| No detecta el chip | SWIO o GND mal, o placa sin alimentación | Comprobar las 3 conexiones |
| El controlador recibe basura | Falta GND común, o no se descarta el eco | Unir masas; descartar los bytes propios |
| El módulo no responde nunca | El pin de DATA no está en open-drain, o falta HDSEL | Revisar la USART; mirar la línea con el analizador |
| Colores mal o el LED no hace caso | LED de cátodo común en vez de ánodo común, o MOSFET/resistencia de puerta mal puestos | Comprobar la polaridad del LED y el circuito de `02_diseno_hardware.md` §5.1 |
| Eventos dobles al abrir | Rebotes del interruptor | Subir el filtro a 30–40 ms |
| SENSE siempre a 0 | Pull-up sin activar, o pin como salida | Revisar la configuración al arrancar y tras `SENSE_RELEASE` |
| El mapa sale incoherente | **Latiguillo en la cara equivocada** | Comprobar la serigrafía `L R U D` de cada conector |
| Un cajón emparejado se ilumina solo a medias | El cable en Y del interruptor, o JP7/JP8 mal puestos | Revisar los puentes y la continuidad del cable en Y |
