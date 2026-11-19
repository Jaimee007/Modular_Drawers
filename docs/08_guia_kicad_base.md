# Guía: esquemático de la base en KiCad

Versión 1.0 · Noviembre 2026 · **Paso a paso para dibujar el esquemático de la placa perforada**

> **Alcance.** Cómo dibujar en KiCad el esquemático de la base (ESP32-C3 SuperMini en placa perforada), desde crear los símbolos de la SuperMini y del breakout USB-C hasta cada conexión. El circuito y los valores están en `05_montaje_base.md` §3; los pines del ESP32, en `02_diseno_hardware.md` §10. **Esta guía no repite los valores: los referencia.**
>
> **Versión de KiCad.** El proyecto `modulo_cajon` está guardado con KiCad 10. Los nombres de menús y atajos son los de esa versión; si algo no coincide, dime qué ves.
>
> **Sin PCB.** Solo hay esquemático: la base va en placa perforada. No se crea `.kicad_pcb`.

---

## 0. Antes de empezar: el orden de pines de la SuperMini

El orden físico de los 16 pines **ya está comprobado contra tu placa** (el pinout del símbolo de protoflow, C9900147397, coincide con la serigrafía):

| Nº | Nombre | Nº | Nombre |
|---|---|---|---|
| 1 | GPIO5 | 9 | GPIO0 |
| 2 | GPIO6 | 10 | GPIO1 |
| 3 | GPIO7 | 11 | GPIO2 |
| 4 | GPIO8 | 12 | GPIO3 |
| 5 | GPIO9 | 13 | GPIO4 |
| 6 | GPIO10 | 14 | 3V3 |
| 7 | GPIO20 | 15 | GND |
| 8 | GPIO21 | 16 | 5V |

Lo que **sigue sin comprobar** es el orden de los pines del **breakout USB-C**: míralo en su serigrafía. Y si dudas de cuál es el 5V o el GND de cualquier placa, **no lo adivines por posición**: con el multímetro en continuidad, el GND es la carcasa metálica del USB-C; con la placa conectada al USB y en voltios DC, el pin de 5 V mide ≈ 5 V y el de 3V3, ≈ 3,3 V.

---

## 1. Crear el proyecto

1. Abre KiCad. **Archivo → Nuevo proyecto…**
2. Ruta: `Hardware/base_perforada/`. Nombre: `base_perforada`. Si KiCad ofrece crear una carpeta para el proyecto, **desmárcalo**, para no anidar.
3. En el gestor verás `base_perforada.kicad_pro` y `base_perforada.kicad_sch`. Haz doble clic en el `.kicad_sch` para abrir el **editor de esquemas**.
4. Descarta o borra el `.kicad_pcb` que KiCad pueda crear: no se usa.

> Si prefieres no crear un proyecto nuevo, puedes abrir una hoja en el proyecto `modulo_cajon`. Lo desaconsejo: la base y el módulo son placas distintas y deben tener proyectos distintos.

---

## 2. Dónde guardar los símbolos nuevos

Ya tienes una librería propia: `Hardware/lib/Modular_Drawers.kicad_sym`, con el símbolo `CH32V003F4P6`. **Añade ahí los dos símbolos nuevos**, así quedan junto al del micro y los puedes reutilizar.

### 2.1 Conectar la librería al proyecto nuevo

1. Editor de esquemas → **Preferencias → Gestionar bibliotecas de símbolos…**
2. Pestaña **Bibliotecas del proyecto** → icono de carpeta (**Añadir biblioteca existente**).
3. Elige `Hardware/lib/Modular_Drawers.kicad_sym`.
4. En **Nickname** pon `Modular_Drawers`. Acepta.

Si el proyecto ya tiene una tabla `sym-lib-table` con esa entrada, no hace falta repetirla.

---

## 3. El símbolo de la SuperMini

Hay dos caminos. **Recomendado: importar el símbolo de protoflow** y revisarlo (§3.A). Si no lo consigues, créalo a mano (§3.B).

### 3.A Importar el símbolo de protoflow

El símbolo `ESP32C3-SUPERMINI` (LCSC C9900147397, `protoflow.ai/library/parts/C9900147397`) viene de una librería de usuarios de EasyEDA convertida a KiCad. **La página no indica la licencia**: para uso personal no suele importar, pero tenlo en cuenta si publicas el repositorio.

1. Descarga `ESP32C3-SUPERMINI.kicad_sym` desde esa página a una carpeta **aparte** (no la metas en el repositorio todavía).
2. En el editor de símbolos: **Archivo → Importar → Símbolo…** y elige el archivo.
3. Copia el símbolo a tu librería: arrástralo a `Modular_Drawers`, o clic derecho → **Copiar** y **Pegar** sobre `Modular_Drawers`. Renómbralo a `ESP32-C3_SuperMini`.
4. **Revisa los tipos eléctricos** de cada pin (tabla de §3.B.2): los símbolos de EasyEDA suelen traer todo como pasivo o bidireccional, y entonces el ERC no avisa de alimentación sin fuente.
5. **Renombra** `GPIO20` a `GPIO20_RX` y `GPIO21` a `GPIO21_TX`, para que el esquema se lea mejor.
6. Elimina la huella asociada (campo *Huella* vacío): la base no lleva PCB y el símbolo de EasyEDA trae una huella SMD que no sirve para la placa perforada.
7. Guarda y salta a §4.

### 3.B Crearlo a mano

Solo si no puedes importar el de §3.A. La SuperMini va enchufada en dos zócalos de 8 pines, así que se modela como un componente de 16 pines con sus nombres reales.

#### 3.B.1 Abrir el editor de símbolos

1. En el editor de esquemas: **Herramientas → Editor de símbolos**.
2. En el panel izquierdo, haz clic derecho sobre `Modular_Drawers` → **Nuevo símbolo…**
3. Rellena:

| Campo | Valor |
|---|---|
| Nombre | `ESP32-C3_SuperMini` |
| Designador de referencia | `U` |
| Huella | déjala vacía (no hay PCB) |
| Tipo | símbolo normal |

#### 3.B.2 Añadir los 16 pines

Para cada pin: **Colocar → Pin** (o la tecla `P`), haz clic donde quieras ponerlo, y en la ventana de propiedades rellena:

| Campo | Qué poner |
|---|---|
| **Nombre del pin** | El nombre de la serigrafía, por ejemplo `GPIO4`, `5V`, `GND`, `3V3`, `GPIO21_TX`, `GPIO20_RX` |
| **Número del pin** | La posición física: `1` a `16`, **en el orden de la tabla del §0** |
| **Tipo eléctrico** | Ver la tabla de abajo |
| **Orientación / lado** | Los pines del lado izquierdo del componente con la orientación que mire hacia la izquierda, y los del derecho hacia la derecha |

**Tipos eléctricos recomendados** (sirven para que el ERC detecte errores):

| Pin | Tipo eléctrico |
|---|---|
| `5V` | Entrada de alimentación (*Power input*) |
| `3V3` | Salida de alimentación (*Power output*) |
| `GND` | Entrada de alimentación (*Power input*) |
| `GPIO…` (el resto) | Bidireccional (*Bidirectional*) |

> **Atención con el ERC:** KiCad se queja si una red de alimentación no tiene una «fuente» marcada. Para `5V` se resuelve con el símbolo `PWR_FLAG` (§6.2); para `3V3`, con el tipo *Power output* de arriba.

Haz los 16 pines. Coloca los del lado izquierdo en columna y los del derecho en otra, separados por una caja.

#### 3.B.3 Dibujar el cuerpo

1. **Colocar → Rectángulo** (o la herramienta de rectángulo): dibuja una caja que encierre las dos columnas de pines, con los pines tocando el borde.
2. Pon el nombre `ESP32-C3 SuperMini` como valor del símbolo (propiedad *Value*).

#### 3.B.4 Comprobar y guardar

1. **Inspeccionar → Comprobador de símbolos** (o el icono de ERC del editor de símbolos). Corrige lo que marque: pines con el mismo número, o sin tipo.
2. **Archivo → Guardar**.

> **Un solo símbolo para ESP32.** No hace falta una huella: en la base no hay PCB.

---

## 4. Crear el símbolo del breakout USB-C

Repite el procedimiento de §3 con estos cambios.

| Campo | Valor |
|---|---|
| Nombre | `USB-C_Breakout` |
| Designador de referencia | `J` |
| Pines | Solo los que cablees. **VBUS**, **GND**, **CC1** y **CC2** son los que usa la base; D+ y D− no |

| Pin | Tipo eléctrico |
|---|---|
| `VBUS` | Salida de alimentación (*Power output*) |
| `GND` | Entrada de alimentación (*Power input*) |
| `CC1`, `CC2` | Pasivo (*Passive*) |

**Número de cada pin:** el que corresponda a la serigrafía de tu breakout (§0). Si el breakout tiene más pines (D+, D−, SBU…), puedes dejarlos fuera del símbolo.

Los **5,1 kΩ** de CC1 y CC2 los lleva el breakout. **No los dibujes** como componentes: déjalos como nota en el esquema («el breakout lleva 5,1 kΩ en CC1 y CC2; medir antes de enchufar»).

---

## 5. Los demás símbolos: ya existen en KiCad

Todos salen de las librerías estándar de KiCad. En el editor de esquemas, `A` abre el selector de símbolos y se busca por nombre.

| Componente | Símbolo | Librería | Valor y referencia |
|---|---|---|---|
| Resistencias | `R` | `Device` | Ver `05` §3.2 |
| Condensador cerámico | `C` | `Device` | `100nF` |
| Condensador electrolítico | `C_Polarized` | `Device` | `1000uF` |
| Schottky | `D_Schottky` | `Device` | `1N5819` (o `1N5818`) |
| TVS | `D_TVS` | `Device` | `P6KE6V8A` |
| Fusible reiniciable | `Polyfuse` | `Device` | `30R185UU` |
| MOSFET N | `Q_NMOS_GSD` o el de tu pieza | `Device` | `2N7002` o el que uses |
| LED RGB ánodo común | `LED_RGB_CommonAnode` si existe; si no, `LED_RCBG` | `Device` | `MP017043` |
| Box header 2×3 (×4) | `Conn_02x03_Odd_Even` | `Connector_Generic` | `L`, `R`, `U`, `D` |
| Microinterruptor | `SW_Push` o `SW_SPST` | `Switch` | final de carrera |
| Masa | `GND` | `power` | |
| +5V | `+5V` | `power` | |
| +3V3 | `+3V3` | `power` | |
| Marca de alimentación | `PWR_FLAG` | `power` | §6.2 |

> No sé con certeza si KiCad 10 trae el nombre exacto `LED_RGB_CommonAnode`. Si el buscador no lo encuentra, usa el símbolo RGB de 4 pines que aparezca y comprueba que su pin común es el que va a +5V.

> **Pinout del MOSFET:** el símbolo `Q_NMOS_GSD` tiene el orden **G, S, D**, pero las patas físicas de tu transistor pueden estar en otro orden (el BS170 en TO-92, por ejemplo). Es un símbolo, no una huella: en la placa **manda el pinout real de la pieza**.

---

## 6. Dibujar el esquemático, paso a paso

Atajos útiles en el editor de esquemas: `A` añadir símbolo, `W` dibujar cable, `L` etiqueta de red, `R` rotar, `E` editar propiedades, `M` mover, `Esc` cancelar.

**Idea general:** en lugar de cablear todo con líneas largas, usa **etiquetas de red** (`L`): dos puntos con la misma etiqueta están conectados. Es lo que hace legible un esquemático de 40 conexiones.

### 6.1 Colocar las piezas

1. `A` → `ESP32-C3_SuperMini` (tu librería) → colócala a la izquierda de la hoja.
2. `A` → `USB-C_Breakout` → colócala arriba a la izquierda.
3. `A` → los cuatro `Conn_02x03_Odd_Even` → colócalos a la derecha. En cada uno, edita (`E`) el **Valor** a `L`, `R`, `U` y `D`.
4. El resto (resistencias, condensadores, diodo, fusible, TVS, MOSFET, LED, interruptor), cerca de lo que conectan.
5. Para cada pieza: `E` → pon **Referencia** (`R1`, `C1`, `Q1`…) y **Valor**. Los valores, en `05_montaje_base.md` §3.2.

### 6.2 Entrada de 5 V

Redes: `VBUS_RAW` entre el breakout y el fusible; `+5V` después del fusible.

1. **Breakout `VBUS`** → `Polyfuse` → red `+5V`. (Usa `W` entre los pines y pon el símbolo `+5V` o la etiqueta `+5V` en el lado de salida del fusible.)
2. **`TVS` entre `+5V` y `GND`**, con el **cátodo (la banda) hacia `+5V`**.
3. **`C_Polarized` de 1000 µF entre `+5V` y `GND`**, con el `+` hacia `+5V`.
4. **Breakout `GND`** → símbolo `GND`.
5. **Pon un `PWR_FLAG` en la red `+5V` y otro en la red `GND`.** Sin ellos, el ERC dice «pin alimentado por ninguna fuente»: el breakout es solo un conector y KiCad no lo sabe.

### 6.3 Alimentación de la SuperMini

1. **Pin `5V` de la SuperMini** → red `+5V`.
2. **Pin `GND` de la SuperMini** → `GND`.
3. **Pin `3V3` de la SuperMini** → red `+3V3`. Este pin es una **salida**, no se alimenta desde fuera.
4. Dos condensadores `C` de 100 nF entre `+3V3` y `GND`, cerca de la SuperMini (se dibuja en el esquema, y en la placa van junto a sus pines).

> ⚠️ **No conectes el USB-C de la SuperMini y el pin `5V` a la vez en la placa real.** Son dos fuentes en la misma red (`05_montaje_base.md` §5). En el esquemático, añade una **nota de texto** (`T`) que lo diga junto al pin `5V`.

### 6.4 Bus DATA

Red `DATA`. Un único pull-up en todo el sistema.

1. **`R` de 4,7 kΩ** entre `+3V3` y la red `DATA`.
2. **Pin `GPIO20_RX`** → red `DATA`, directo.
3. **`D_Schottky`**: el **ánodo a `DATA`** y el **cátodo (banda) hacia `GPIO21_TX`**. Es decir, el diodo apunta de DATA hacia el pin TX.
4. **Pin `GPIO21_TX`** → cátodo del diodo.

### 6.5 Los cuatro box header

Cada `Conn_02x03_Odd_Even` tiene 6 pines. El pinout del cable (`02_diseno_hardware.md` §3.2):

| Pin | Señal |
|---|---|
| 1 | `GND` |
| 2 | `+5V` |
| 3 | `DATA` |
| 4 | `SENSE_x` |
| 5 | `GND` |
| 6 | `+5V` |

Para **cada uno de los cuatro**:
1. Pines 1 y 5 → `GND`.
2. Pines 2 y 6 → `+5V`.
3. Pin 3 → `DATA` (la **misma red** en los cuatro).
4. Pin 4 → `SENSE_L`, `SENSE_R`, `SENSE_U` o `SENSE_D` (**una red distinta en cada uno**). **No unas dos SENSE entre sí.**

> ⚠️ **Orden de los pines en el símbolo:** `Conn_02x03_Odd_Even` numera los pines impares en una columna y los pares en otra. Comprueba en tu conector físico que el pin 1 es el del triángulo y que la numeración coincide con la del cable. Si no, el latiguillo cruzaría GND con 5V.

### 6.6 SENSE

Cuatro resistencias de 1 kΩ, una por cara:

| Pin SuperMini | Resistencia | Red |
|---|---|---|
| `GPIO0` | R 1 kΩ | `SENSE_L` |
| `GPIO1` | R 1 kΩ | `SENSE_R` |
| `GPIO3` | R 1 kΩ | `SENSE_U` |
| `GPIO10` | R 1 kΩ | `SENSE_D` |

Cada `SENSE_x` va solo al pin 4 de su box header.

### 6.7 LED RGB y MOSFET

Repite esto **tres veces** (rojo, verde, azul). Valores en `05_montaje_base.md` §3.1.

1. **Ánodo común del LED** → `+5V`.
2. **Cátodo del color** → `R` (150 Ω rojo; 100 Ω verde y azul) → **drenador** del MOSFET.
3. **Fuente** del MOSFET → `GND`.
4. **Puerta** del MOSFET → pin de la SuperMini: `GPIO4` (rojo), `GPIO5` (verde) o `GPIO7` (azul).
5. **`R` de 100 kΩ** entre la puerta y `GND`.

### 6.8 Final de carrera

1. **Un lado del interruptor** → `GPIO6`.
2. **El otro lado** → `GND`.

En la placa real, COM va a GND y NO a GPIO6 (`07_pruebas_banco.md#base_rgb_led`). En el esquemático el interruptor es un contacto simple.

---

## 7. Comprobar con el ERC

1. **Inspeccionar → Comprobador de reglas eléctricas (ERC)** → **Ejecutar**.
2. Los avisos esperables, y qué hacer:

| Aviso | Qué hacer |
|---|---|
| «Pin no conectado» en D+, D−, SBU… del breakout | Normal si no los has puesto en el símbolo; si los pusiste, márcalos como «no conectado» (`Q`) |
| «Power pin not driven» en `+5V` o `GND` | Falta el `PWR_FLAG` (§6.2) |
| «Pin no conectado» en un GPIO que no usas (`GPIO2`, `GPIO8`, `GPIO9`) | Normal: marca la X de «no conectado» (`Q`) en cada uno |
| Dos salidas conectadas entre sí | Revisa `3V3` y `VBUS`: nunca deben unirse |

3. **No ignores** los avisos de «pin sin conectar» en pines que **sí** deberían estar conectados: suelen indicar una etiqueta con el nombre mal escrito (`DATA` frente a `Data`).

---

## 8. Cómo comprobar que las conexiones son correctas

Con el esquema cerrado, antes de soldar:

1. **Lista de redes:** **Archivo → Fabricación → Netlist** (o **Herramientas → Generar lista de redes**). En la lista, comprueba que:
   - `DATA` tiene: pull-up, GPIO20, el diodo y los pines 3 de los 4 box header.
   - Cada `SENSE_x` tiene exactamente: un GPIO, una resistencia y un pin 4.
   - `+5V` conecta: fusible, TVS, electrolítico, pin `5V` de la SuperMini, ánodo del LED y pines 2 y 6 de los box header.
2. **Contra el doc:** compara con `05_montaje_base.md` §3 y con la tabla de pines de `02_diseno_hardware.md` §10. Si algo no cuadra, **manda `02`**.
3. **En la placa:** cuando suelden, las comprobaciones de `05` §6 con el multímetro (continuidad de cada SENSE, DATA común, 5,1 kΩ en CC1/CC2, sin cortocircuito entre 3V3 y GND).

---

## 9. Exportar

- **PDF del esquema:** **Archivo → Trazar → PDF**. Es lo que llevas al banco.
- No hay gerbers ni BOM: la base no se fabrica.

---

## 10. Antes de dar por bueno el esquema

- Confirmar el orden de pines del breakout USB-C con tu placa (§0). El de la SuperMini ya está comprobado.
- Confirmar la polaridad del TVS y del electrolítico en las hojas de datos.
- Comprobar el pinout real del MOSFET que uses.
- Subir `Hardware/base_perforada/` y los dos símbolos nuevos de `Hardware/lib/` al repositorio **solo cuando el ERC esté limpio**.
