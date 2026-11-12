# Plan de acción: programar y probar el firmware, del primer parpadeo a Home Assistant

Versión 1.0 · Octubre 2026 · Entorno: **VS Code + PlatformIO** · Micro de los cajones: **CH32V003F4P6** · Base: **ESP32-C3 SuperMini**

Cada fase termina con una **prueba de aceptación**. No pases a la siguiente hasta que la anterior funcione: así, cuando algo falle, sabrás que el problema está en lo último que has añadido.

---

## Resumen de fases

| Fase | Objetivo | Hardware necesario | Duración orientativa |
|---|---|---|---|
| 0 | Preparar el entorno y el repositorio | PC, WCH-LinkE | 1–2 h |
| 1 | Primer programa: parpadeo y `printf` | 1 placa CH32V003 + LED | 1 h |
| 2 | Periféricos del cajón, uno a uno | + LED RGBW, final de carrera, jumpers | 3–4 h |
| 3 | Bus DATA: el ESP32 habla con un cajón | + ESP32-C3, diodo, resistencias | 4–6 h |
| 4 | Dos cajones en el mismo bus | 2.ª placa CH32V003 | 1–2 h |
| 5 | SENSE: descubrimiento de la posición | Cables entre placas | 3–4 h |
| 6 | Robustez y lógica completa del cajón | — | 3–4 h |
| 7 | Integración con Home Assistant (ESPHome) | Home Assistant funcionando | 4–6 h |
| 8 | Búsqueda, panel y voz | — | 2–3 h |
| 9 | Prueba final con los cajones impresos | Armazones v1 + coletas | 2 h |

---

## Fase 0 · Preparar el entorno

### 0.1 Instalar el software

1. **VS Code** y la extensión **PlatformIO IDE**.
2. **La plataforma CH32V** para PlatformIO. En el terminal de PlatformIO:
   ```
   pio pkg install -g -p https://github.com/Community-PIO-CH32V/platform-ch32v.git
   ```
3. **WCH-LinkUtility** (Windows, en la web de WCH). Lo vas a usar para dos cosas:
   - **Actualizar el firmware del WCH-LinkE.** La plataforma avisa de que, con un firmware antiguo, la grabación falla con "Programming Failed".
   - **Ponerlo en modo RISC-V.** El WCH-LinkE tiene dos modos (ARM y RISC-V); en modo ARM no reconoce el CH32V003. El modo se cambia desde WCH-LinkUtility o con el botón "ModeS" del propio programador.

### 0.2 Estructura del repositorio

```
Modular_Drawers/
├── docs/                     ← documentación (ya existe)
├── tools/                    ← scripts (ya existe)
├── firmware/
│   ├── cajon/                ← proyecto PlatformIO del CH32V003
│   │   ├── platformio.ini
│   │   ├── include/          ← protocolo compartido (bus.h), funconfig.h
│   │   └── src/main.c
│   └── base_test/            ← proyecto PlatformIO del ESP32 (pruebas, fases 3–6)
│       ├── platformio.ini
│       └── src/main.cpp
├── esphome/                  ← fase 7
│   ├── components/cajonera/  ← componente externo de ESPHome
│   └── cajonera.yaml
└── README.md
```

### 0.3 Configuración de PlatformIO

**`firmware/cajon/platformio.ini`**

```ini
[env:cajon]
platform = https://github.com/Community-PIO-CH32V/platform-ch32v.git
board = genericCH32V003F4P6
framework = ch32v003fun
upload_protocol = minichlink
```

**Por qué ch32v003fun y no Arduino:**

- **Memoria:** ocupa muy poco, y el micro solo tiene 16 KB de flash y 2 KB de RAM.
- **Control:** da acceso directo al UART en modo half-duplex, que es lo que necesita el bus.
- **Ejemplos:** trae ejemplos de WS2812 por SPI+DMA.

La placa también admite el framework Arduino (`framework = arduino`) si en algún momento lo prefieres. Como subidor, `minichlink` es el propio de ch32v003fun; `wch-link` (OpenOCD) también funciona.

**`firmware/base_test/platformio.ini`**

```ini
[env:base]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino
monitor_speed = 115200
build_flags =
  -DARDUINO_USB_MODE=1
  -DARDUINO_USB_CDC_ON_BOOT=1
```

Las dos `build_flags` hacen que el `Serial` del ESP32-C3 SuperMini salga por su USB-C.

**✅ Prueba de aceptación:** los dos proyectos compilan (botón ✓ de PlatformIO) y se ha hecho un commit "Estructura de firmware".

---

## Fase 1 · Primer programa en el CH32V003

### 1.1 Conexión del WCH-LinkE a la placa

| WCH-LinkE | Placa CH32V003 |
|---|---|
| 3V3 | 3V3 / VCC |
| GND | GND |
| SWDIO | PD1 (SWIO) |

> ⚠️ El 3V3 del WCH-LinkE solo se conecta cuando la placa no tiene otra alimentación. Si la placa ya recibe corriente (por ejemplo, del bus de 5 V a través del LDO), conecta solo GND y SWIO.

### 1.2 Parpadeo

1. Pon un LED con su resistencia de 330 Ω en **PD4**. Es un pin libre en el diseño final; si tu placa ya trae un LED, usa ese.
2. Copia el ejemplo *blink* de ch32v003fun en `src/main.c`, cambiando el pin.
3. Pulsa **Upload** (→) en PlatformIO.

### 1.3 `printf` por el cable de programación

ch32v003fun puede enviar `printf` a través del mismo hilo SWIO que se usa para programar. Esto es muy útil porque el UART del micro estará ocupado con el bus.

1. Activa la salida de depuración en `funconfig.h`.
2. Abre el terminal de `minichlink` en modo terminal (`-T`).
3. Imprime un contador cada segundo.

**✅ Prueba de aceptación:** el LED parpadea, ves el contador en el terminal y puedes grabar el micro 5 veces seguidas sin errores.

---

## Fase 2 · Periféricos del cajón, uno a uno

Monta en la protoboard los componentes según la **hoja 2 del esquemático**. Cada prueba es un pequeño programa (o una opción de un menú) que se puede volver a usar más adelante para revisar una placa.

### 2.1 LED RGBW (PC6)

- **Conexión:** LED con su diodo (1N4148) en la alimentación, condensador de 100 nF y resistencia de 330 Ω en DIN.
- **Para empezar,** usa el ejemplo de WS2812 de ch32v003fun (SPI+DMA en PC6).
- **RGBW:** el SK6812**RGBW** necesita **4 bytes por LED** (G, R, B, W) en vez de 3. Hay que adaptar el ejemplo para enviar 32 bits por LED.
- **Prueba:** ciclo rojo → verde → azul → blanco (canal W) → apagado.

**✅ Prueba de aceptación:** los cuatro colores salen limpios, sin parpadeos raros, y el blanco del canal W es claramente distinto del blanco RGB.

### 2.2 Final de carrera (PD6)

- **Conexión:** entrada con pull-up interna. COM del interruptor a GND y NO a PD6.
- **Rebotes:** filtro por software de 25 ms; el estado solo cambia si se mantiene estable ese tiempo.
- **Prueba:** imprime "ABIERTO" o "CERRADO" en cada cambio y pon el LED en blanco al abrir.

**✅ Prueba de aceptación:** 50 aperturas y cierres seguidos generan exactamente 100 eventos (no aparecen eventos duplicados por rebotes).

### 2.3 Jumpers de dirección y de tamaño

- **Pines:** PC0–PC4 y PD0 para la dirección (bits 0–5) y PC5 para el tamaño, todos con pull-up interna. En la protoboard, un jumper a GND equivale a un puente cerrado.
- **Lectura:** se leen una sola vez al arrancar: `dirección = suma de los bits a 0`.
- **Prueba:** al arrancar se imprimen la dirección y el tamaño. Si la dirección es 0, el LED parpadea en ámbar.

**✅ Prueba de aceptación:** las direcciones 1, 2, 21 y 63 y el tamaño 1×1/2×1 se leen bien. Con la dirección 0 el LED parpadea en ámbar.

### 2.4 Pines SENSE (lectura básica)

- **Pines:** PA1, PA2, PD2 y PD3 como entradas con pull-up, cada una con su resistencia de 1 kΩ.
- **Prueba:** imprime el estado de los 4 pines cada 200 ms y lleva cada pin a GND con un cable.

**✅ Prueba de aceptación:** cada pin cambia a 0 solo cuando lo llevas a GND, sin afectar a los otros. Commit "Periféricos del cajón".

---

## Fase 3 · Bus DATA: el ESP32 habla con un cajón

### 3.1 Montaje (hojas 1 y 3 del esquemático)

| Conexión | Detalle |
|---|---|
| GND común | ESP32, CH32V003 y WCH-LinkE |
| Pull-up del bus | 4,7 kΩ entre DATA y 3V3 (solo en la base) |
| ESP32 GPIO6 (RX1) | Directo a DATA |
| ESP32 GPIO5 (TX1) | A DATA a través de un diodo B5819W (cátodo hacia GPIO5) |
| CH32V003 PD5 | A DATA a través de 1 kΩ |

Alimentación en la protoboard: el ESP32 por USB y el CH32V003 desde el 3V3 del ESP32. Más adelante, el LDO del cajón.

### 3.2 Lado del cajón (CH32V003)

- **USART1 en modo half-duplex** (bit HDSEL), con PD5 como salida de función alternativa *open-drain* a 115200 baudios, 8N1.
- **Eco:** en half-duplex el receptor también "oye" lo que transmite el propio micro. Hay que descartar ese eco, o desactivar la recepción mientras se transmite.
- **Recepción:** por interrupción, con un búfer y una máquina de estados que reconoce la trama.
- **Formato de trama** (definido en el plan, §2.8):
  ```
  [0xA5] [DIR] [CMD] [LEN] [DATOS…] [CRC8]
  ```
- **Comandos de esta fase:** `PING`, `SET_LED` y `GET_STATE`.
- **Respuesta:** el cajón responde solo si `DIR` coincide con su dirección. Antes de contestar espera ~100 µs, para dar tiempo a que el ESP32 libere la línea.

### 3.3 Lado del ESP32 (`base_test`)

- `Serial1.begin(115200, SERIAL_8N1, 6, 5)`, es decir, RX en GPIO6 y TX en GPIO5.
- También tiene que descartar su propio eco (lo que envía lo recibe por RX).
- Un pequeño menú por el monitor serie para lanzar las pruebas:
  ```
  ping 1          → PING al cajón 1
  led 1 0 0 255 0 → cajón 1 en azul (R G B W)
  state 1         → estado (abierto/cerrado)
  stress 1 1000   → 1000 PING seguidos y recuento de errores
  ```
- **Código compartido:** el formato de trama y el CRC deben estar en un único archivo (`bus.h`) que usan los dos proyectos, para que no haya diferencias.

### 3.4 Depuración con el analizador lógico

| Canal | Señal |
|---|---|
| 0 | DATA |
| 1 | LED (PC6) |
| 2 | SENSE_L |
| 3 | SENSE_R |

En PulseView, añade el decodificador UART (115200, 8N1) al canal 0 para ver las tramas.

**✅ Prueba de aceptación:** `ping 1` responde siempre, `led` cambia el color al momento y `stress 1 1000` da **0 errores**. Commit "Bus DATA funcionando".

---

## Fase 4 · Dos cajones en el mismo bus

1. Monta la segunda placa CH32V003 igual que la primera, con la **dirección 2**.
2. Conecta el PD5 de las dos placas a la misma línea DATA, cada una con su 1 kΩ.
3. Pruebas:
   - `ping 1` y `ping 2` responden por separado.
   - `led 1 …` solo cambia el cajón 1.
   - **Prueba de error:** pon las dos placas en la **dirección 1**. El ESP32 debe recibir una respuesta corrupta (CRC incorrecto) y avisar de "dirección duplicada".

**✅ Prueba de aceptación:** los dos cajones responden de forma independiente, `stress` a los dos da 0 errores y la dirección duplicada se detecta.

---

## Fase 5 · SENSE: descubrimiento de la posición

### 5.0 Prueba previa: solo la capa eléctrica, con dos placas de desarrollo

Se puede validar la parte física de SENSE **antes** de tener el bus DATA y el ESP32, con dos placas CH32V003 de desarrollo (A y B). No cubre `SENSE_DRIVE`, `SENSE_QUERY` ni el mapa: eso va por DATA y se prueba en 5.1–5.3.

Pines (como en 2.4): PA1 (izquierda), PA2 (derecha), PD2 (arriba) y PD3 (abajo), entradas con pull-up, cada una con 1 kΩ en serie. GND común entre las dos placas.

| # | Prueba | Montaje | Resultado esperado |
|---|---|---|---|
| 1 | Lectura de un pin | Una placa. Imprime el estado de PA1 cada 200 ms y llévalo a GND con un cable | Lee 0 al llevarlo a GND y 1 al soltarlo |
| 2 | Dos placas conectadas | PA2 de A a PA1 de B, con 1 kΩ en cada lado. A tira PA2 a 0 (open-drain) o lo suelta; B lee PA1. Repite en sentido contrario | A tira: B lee 0. A suelta: B lee 1. En ningún caso se fuerza un 1 (dos placas tirando a la vez no hacen cortocircuito) |
| 3 | «Hola» al encender | B arranca y pone sus 4 SENSE a 0 durante 50 ms. A tiene una interrupción en PA2 | A registra el evento al alimentar B con A ya encendida |
| 4 | Independencia de caras | Solo PA2(A)–PA1(B) conectados. B actúa sobre ese SENSE | PD2 y PD3 de A no cambian (requisito **F6**: los SENSE no se unen entre caras) |

**✅ Prueba de aceptación:** las cuatro pruebas dan el resultado esperado.

### 5.1 Montaje

Une los SENSE como en la hoja 1 del esquemático. Cada unión lleva una resistencia de 1 kΩ en cada lado.

| Desde | Hasta |
|---|---|
| ESP32 GPIO1 (SENSE derecho de la base) | Cajón 1 PA1 (SENSE izquierdo) |
| Cajón 1 PA2 (SENSE derecho) | Cajón 2 PA1 (SENSE izquierdo) |

### 5.2 Firmware

- **Cajón:** comandos `SENSE_DRIVE n`, `SENSE_RELEASE` y `SENSE_QUERY`. Al ver un SENSE a 0 que él no está tirando, responde con su dirección y el número de ese SENSE. Si no ve ninguno, no responde.
- **ESP32:**
  - El algoritmo del plan (§2.9): parte de la base en (0,0), recorre los vecinos y calcula la posición de cada uno con la tabla de conectores.
  - El comando `map` imprime la cuadrícula:
    ```
    (0,0) BASE   (1,0) #1 1×1   (2,0) #2 1×1
    ```
- **"Hola" al encender:** el cajón nuevo pone todos sus SENSE a 0 durante 50 ms. El vecino lo detecta por interrupción y lo marca en `GET_STATE`. El ESP32 ve el aviso y repite el descubrimiento en esa zona.

### 5.3 Pruebas

1. Con todo encendido, `map` debe mostrar la posición correcta.
2. Cambia el orden (cajón 2 junto a la base y cajón 1 al final): `map` debe reflejarlo.
3. Desconecta la alimentación del cajón 2 y vuelve a conectarla: el ESP32 debe avisar de que hay un cajón nuevo y volver a colocarlo en (2,0) **sin reiniciar nada**.
4. Quita el cable SENSE entre los dos cajones: el cajón 2 debe aparecer como "sin posición", pero seguir respondiendo por el bus.

**✅ Prueba de aceptación:** las 4 pruebas dan el resultado esperado. Commit "Descubrimiento de posición".

---

## Fase 6 · Robustez y lógica completa del cajón

### 6.1 Lógica del LED en el cajón

Implementa la tabla de estados del plan (§2.5):

| Situación | LED |
|---|---|
| Al abrir | Blanco |
| Al abrir un cajón buscado | Verde 2 s y después blanco (la búsqueda se cancela) |
| Sin dirección | Ámbar |
| Error | Rojo |
| `IDENTIFY` | Parpadeo rápido |

**Prioridad:** lo que hace el propio cajón al abrirse manda sobre lo que pida el ESP32.

### 6.2 Seguridad del cajón

- **Watchdog (IWDG):** si el firmware se cuelga, el micro se reinicia solo.
- **Tiempo sin órdenes:** si pasa más de 30 s sin recibir nada del ESP32, el cajón apaga la búsqueda pero sigue encendiendo la luz al abrirse.
- **Brillo:** límite del brillo por defecto, guardado en el firmware.

### 6.3 Seguridad del ESP32

- **Sondeo:** cada ~100 ms, con 2 reintentos. Tras 3 fallos seguidos, el cajón se marca como "desconectado".
- **LEDs encendidos a la vez:** como máximo 10 cajones.
- **Cajones nuevos:** cada 5 s pregunta a las direcciones que no conoce.

### 6.4 Prueba de resistencia

Deja el sistema funcionando **24 horas**:

- El ESP32 enciende un cajón al azar cada minuto.
- Abre y cierra los cajones de vez en cuando.
- Al terminar, revisa en el monitor el recuento de errores y de reinicios.

**✅ Prueba de aceptación:** 24 h sin reinicios inesperados y con menos de un 0,1 % de tramas con error. Commit "Firmware robusto v0.1".

---

## Fase 7 · Integración con Home Assistant (ESPHome)

### 7.1 Componente externo

1. Pasa la lógica de `base_test` (bus, sondeo y mapa) a un **componente externo de ESPHome** en C++: `esphome/components/cajonera/`.
2. Usa el UART de ESPHome en GPIO5/6, sin cambios en el hardware.
3. Por cada cajón detectado, el componente publica:

| Entidad | Uso |
|---|---|
| `light.cajon_NN` | RGBW, con efectos de parpadeo |
| `binary_sensor.cajon_NN_abierto` | Estado del cajón |
| `text.cajon_NN_contenido` | Lo que guarda; se queda guardado en el ESP32 |
| `sensor.cajon_NN_x`, `sensor.cajon_NN_y`, `sensor.cajon_NN_tamano` | Posición y tamaño |

Además, un botón "Redescubrir cajonera".

### 7.2 Instalación

1. En `esphome/cajonera.yaml` declara la placa (`esp32-c3-devkitm-1`), la Wi-Fi, la API de Home Assistant y `external_components` apuntando a la carpeta local.
2. Graba la primera vez por USB; las siguientes, por OTA.
3. En Home Assistant: **Ajustes → Dispositivos → ESPHome → Añadir**.

**Si el componente externo se complica,** hay un plan B: mantener el firmware Arduino de `base_test` y publicar las entidades por **MQTT con descubrimiento automático**. Funciona igual en Home Assistant.

**✅ Prueba de aceptación:**

- Los dos cajones aparecen como dispositivos en Home Assistant.
- Encender `light.cajon_01` en azul lo enciende en menos de 0,5 s.
- Abrir el cajón 2 cambia su `binary_sensor` en menos de 0,5 s.
- Tras reiniciar el ESP32, todo vuelve a aparecer solo.

Commit "Integración ESPHome".

---

## Fase 8 · Búsqueda, panel y voz

1. **Script `buscar_componente`:**
   1. Recibe un texto, por ejemplo "10k".
   2. Busca ese texto en el contenido (`text`) de cada cajón.
   3. Hace parpadear en azul los cajones que coinciden.
   4. Los apaga al abrirlos o a los 30 s.
2. **Panel:** una tarjeta con la cuadrícula dibujada a partir de las posiciones X/Y, el color de cada cajón y su contenido. Al tocar un cajón, se identifica (parpadea).
3. **Assist:** un intent personalizado del tipo "¿dónde están las {cosa}?" que llama al script.

**✅ Prueba de aceptación:** escribes o dices "resistencias de 10k" y se ilumina el cajón correcto. Al abrirlo, se pone en verde y después en blanco.

---

## Fase 9 · Prueba final con los cajones impresos

1. Pasa los dos cajones y la base de la protoboard a placa perforada, dentro de los armazones v1, con sus coletas de cable plano.
2. Repite las pruebas clave:
   - **Fase 3:** `stress` sin errores.
   - **Fase 5:** mapa correcto y cajón nuevo detectado.
   - **Fase 8:** búsqueda desde Home Assistant.
3. Prueba mecánica: abre y cierra cada cajón 100 veces y comprueba que el final de carrera no falla nunca.

**✅ Prueba de aceptación final:** el sistema completo funciona con los cajones reales. Commit y etiqueta `v0.1-prototipo`.

---

## Problemas típicos y cómo resolverlos

| Síntoma | Causa probable | Solución |
|---|---|---|
| "Programming Failed" al grabar | Firmware del WCH-LinkE antiguo, o en modo ARM | Actualizarlo y ponerlo en modo RISC-V con WCH-LinkUtility |
| No detecta el chip | SWIO o GND mal conectados, o placa sin alimentación | Comprobar las 3 conexiones; probar con el 3V3 del WCH-LinkE |
| El ESP32 recibe basura | Falta GND común, o no se descarta el eco | Unir las masas; descartar los bytes propios |
| El cajón no responde nunca | PD5 no está en open-drain, o falta HDSEL | Revisar la configuración del USART; mirar la línea con el analizador |
| Colores mal o el LED no hace caso | Trama de 24 bits en un LED RGBW, o problema de nivel de la señal | Enviar 32 bits por LED; comprobar el diodo en la alimentación del LED |
| Eventos dobles al abrir | Rebotes del interruptor | Subir el filtro a 30–40 ms |
| SENSE siempre a 0 | Pull-up sin activar, o pin configurado como salida | Revisar la configuración del pin al arrancar y tras `SENSE_RELEASE` |
| El mapa sale incoherente | Cable SENSE cruzado o tabla de conectores incorrecta | Comprobar las conexiones contra la hoja 1; revisar la tabla del plan (§2.3) |
