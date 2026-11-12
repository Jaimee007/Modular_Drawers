# Diseño de PCB

Versión 2.0 · Octubre 2026 · **Las placas en KiCad: esquemático, layout y fabricación**

> **Alcance.** Cómo se materializa el hardware en circuito impreso: esquemáticos, huellas, layout, reglas de diseño y pedido a JLCPCB. Implementa `02_diseno_hardware.md`.
>
> Entorno: **KiCad 10**. Fabricante: **JLCPCB**, Economic PCBA.

---

## 1. Las placas del proyecto

| Placa | Tamaño | Cantidad | Estado |
|---|---|---|---|
| **Módulo** | ~30 × 30 mm | 10 en el primer pedido (diseño para hasta 31) | Fase H3 — este documento |
| **Base (controlador)** | por definir | 1–2 | Después de H3 |
| **LED** | — | — | **No lleva placa propia**: un LED RGB de 5 mm (ánodo común) con 4 hilos a la tira J6 |
| *Devboard CH32V003* | 36 × 20,32 mm | 0 | Ejercicio de aprendizaje; **no se pide** |

**Una sola placa de módulo para todos los cajones.** No existe variante 2×1: un cajón doble son dos placas idénticas.

### 1.1 Por qué la placa es pequeña y flotante

~30 × 30 mm, no 44 × 44 del tamaño del cajón. Para llegar a los sitios están los cables.

**Esto desbloquea el diseño:** con una placa que se atornilla con dos tornillos donde convenga, el PCB **deja de depender de la geometría del armazón impreso**. Se puede cerrar el diseño sin esperar al armazón v2.

Consecuencia asumida: el **final de carrera sale de la placa**, a un conector de 2 pines con cables, igual que el LED. La placa tiene tres salidas de cable: LED, interruptor y los cuatro latiguillos del bus.

---

## 2. Organización en el repositorio

```
Modular_Drawers/
├── Hardware/
│   ├── lib/
│   │   ├── Modular_Drawers.kicad_sym     (símbolos propios, si hiciera falta)
│   │   └── Modular_Drawers.pretty/       (huellas propias, si hiciera falta)
│   ├── devboard_ch32v003/                (ejercicio)
│   ├── modulo_cajon/                     ← la placa de este documento
│   └── modulo_base/                      (después)
```

**En `.gitignore`:**

```
*-backups/
*.kicad_prl
fp-info-cache
*.net
*.rpt
sym-lib-table.bak
Hardware/*/fab/
```

Los `.kicad_sch`, `.kicad_pcb`, `.kicad_pro` y `.kicad_sym` **sí** van a git: son texto plano y versionan bien.

---

## 3. Bibliotecas: no hay que dibujar nada

| Elemento | De dónde sale |
|---|---|
| **CH32V003F4P6** | **`MCU_WCH_RiscV` → `CH32V003FxPx`**, biblioteca oficial de KiCad 10. Verificado pin a pin contra el datasheet |
| Huella TSSOP-20 | `Package_SO:TSSOP-20_4.4x6.5mm_P0.65mm` — ya asignada en el símbolo |
| Box header 2×3 | `Connector_IDC:IDC-Header_2x03_P2.54mm_Vertical` |
| Puentes de soldadura | `Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm` |
| Puntos de test | `TestPoint:TestPoint_Pad_D1.5mm` |
| Todo lo demás | Bibliotecas estándar |

> **Toda la placa se hace con huellas de biblioteca.** Esto elimina el error más caro posible: una huella mal dibujada no la detecta el ERC, ni el DRC, ni la previsualización de JLCPCB. Aparece cuando las placas están soldadas.

**El símbolo oficial no trae el código de LCSC** (las bibliotecas de KiCad son neutrales respecto a proveedores). Añade un campo `LCSC` **sobre la instancia en el esquemático**, no editando la biblioteca oficial (se sobrescribe al actualizar KiCad).

---

## 4. Lista de materiales de la placa de módulo

| Ref | Componente | Huella | LCSC | Tipo JLC |
|---|---|---|---|---|
| U1 | CH32V003F4P6 | `Package_SO:TSSOP-20_4.4x6.5mm_P0.65mm` | C5187096 | **Extended** |
| U2 | XC6206P332MR | `Package_TO_SOT_SMD:SOT-23` | C5446 | Basic |
| C1 | 100 nF, desacoplo en +3V3 | `Capacitor_SMD:C_0603_1608Metric` | genérico | Basic |
| C2 | 10 µF, salida del LDO (CL) | `Capacitor_SMD:C_0805_2012Metric` | genérico | Basic |
| C3 | 1 µF, entrada del LDO (Cin) | `C_0603_1608Metric` | genérico | Basic |
| C4 | 100 nF, desacoplo en +3V3 | `C_0603_1608Metric` | genérico | Basic |
| C5 | 100 nF antirrebote (**DNP**) | `C_0603_1608Metric` | genérico | Basic |
| R1 | 1 kΩ serie en DATA | `Resistor_SMD:R_0603_1608Metric` | genérico | Basic |
| R2–R5 | 1 kΩ serie en SENSE L/R/U/D | `R_0603_1608Metric` | genérico | Basic |
| Q1–Q3 | MOSFET N 2N7002 (uno por color del LED) | `Package_TO_SOT_SMD:SOT-23` | C8545 (JSCJ 2N7002, 60 V, 115 mA) | Basic |
| R6 | 150 Ω, rojo del LED | `R_0603_1608Metric` | C22808 (Uni-Royal 0603WAF1500T5E) | Basic |
| R7, R8 | 100 Ω, verde y azul del LED | `R_0603_1608Metric` | C22775 (Uni-Royal 0603WAF1000T5E) | Basic |
| R9–R11 | 100 kΩ, puerta a GND de Q1–Q3 | `R_0603_1608Metric` | C25803 (Uni-Royal 0603WAF1003T5E) | Basic |
| JP0–JP4 | Dirección: pesos 1,2,4,8,16 | `Jumper:SolderJumper-2_P1.3mm_Open_Pad1.0x1.5mm` | — | — |
| JP7 | Pareja a la izquierda | igual | — | — |
| JP8 | Pareja abajo | igual | — | — |
| J1–J4 | Box header 2×3 (L, R, U, D) | `Connector_IDC:IDC-Header_2x03_P2.54mm_Vertical` | — | THT |
| J5 | Tira 1×3 — programación | `Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical` | — | THT |
| J6 | Tira 1×4 — LED RGB (5V, R, G, B) | `PinHeader_1x04_P2.54mm_Vertical` | — | THT |
| J7 | Tira 1×2 — final de carrera | `PinHeader_1x02_P2.54mm_Vertical` | — | THT |
| TP1–TP7 | Puntos de test | `TestPoint:TestPoint_Pad_D1.5mm` | — | — |

**Una sola pieza Extended** (el micro) → un feeder de ~3 $. El 2N7002 (C8545) es Basic. El LED no va en esta placa, así que no tiene feeder. **Los códigos de las resistencias salen del catálogo de JLCPCB; falta confirmar que las de 100 Ω, 150 Ω y 100 kΩ siguen en stock.**

---

## 5. Esquemático, bloque a bloque

### 5.1 Alimentación

```
pin 2 de J1–J4  ──┬── +5V   (los 4 conectores en paralelo)
pin 6 de J1–J4  ──┘
                   ├── C3 (1 µF) ── GND
                   └── U2 VIN
                        U2 VOUT ── +3V3 ──┬── C4 (100 nF) ── GND
                                           ├── C1 (100 nF) ── GND
                                           ├── C2 (10 µF)  ── GND
                                           ├── U1 pin 9 (VDD)
                                           └── J5.1
pin 1 de J1–J4  ──┬── GND
pin 5 de J1–J4  ──┘
```

El XC6206 pide (datasheet de Torex, circuito típico) **1 µF cerámico en la entrada y 1 µF cerámico en la salida**. C3 (1 µF) cumple la entrada; el C2 (10 µF) cumple la salida con margen. Todos cerámicos X5R/X7R, nunca Y5V.

Colocación: C3 junto a VIN/GND de U2; C2 junto a VOUT/GND de U2; C1 y C4 junto a VDD/VSS de U1. Un cerámico de 1 µF en 0603 pierde capacidad con la tensión aplicada: C3 de 10 V o más.

Los pines 2 y 6 son la misma red, igual que el 1 y el 5. La duplicación es para repartir corriente y lo que importa es el layout (§7).

### 5.2 Bus DATA

```
U1 pin 2 (PD5) ── R1 (1 kΩ) ── DATA ──┬── pin 3 de J1
                                        ├── pin 3 de J2
                                        ├── pin 3 de J3
                                        ├── pin 3 de J4
                                        └── TP1
```

**Sin pull-up en el módulo.** Va uno solo en la base.

### 5.3 SENSE — cuatro redes independientes

```
U1 pin 5  (PA1) ── R2 ── SENSE_L ── pin 4 de J1  (izquierda) + TP2
U1 pin 6  (PA2) ── R3 ── SENSE_R ── pin 4 de J2  (derecha)   + TP3
U1 pin 19 (PD2) ── R4 ── SENSE_U ── pin 4 de J3  (arriba)    + TP4
U1 pin 20 (PD3) ── R5 ── SENSE_D ── pin 4 de J4  (abajo)     + TP5
```

### 5.4 Final de carrera

```
U1 pin 3 (PD6) ──┬── J7.1
                  └── C5 (100 nF, DNP) ── GND
J7.2 ── GND
```

### 5.5 Dirección y emparejamiento

```
U1 pin 10 (PC0) ── JP0 ── GND     peso 1
U1 pin 11 (PC1) ── JP1 ── GND     peso 2
U1 pin 12 (PC2) ── JP2 ── GND     peso 4
U1 pin 13 (PC3) ── JP3 ── GND     peso 8
U1 pin 14 (PC4) ── JP4 ── GND     peso 16
U1 pin  8 (PD0) ── azul del LED (§5.6). Sin puente de dirección de peso 32: con 31 módulos la dirección usa solo JP0–JP4
U1 pin 15 (PC5) ── JP7 ── GND     pareja a la IZQUIERDA
U1 pin  1 (PD4) ── JP8 ── GND     pareja ABAJO
```

Sin resistencias: pull-up internas.

### 5.6 Salida al LED

```
+5V ── J6.1   (ánodo común del LED)

PC6 ──┬── R9 (100 kΩ) ── GND
      └── puerta de Q1       drenador ── R6 (150 Ω) ── J6.2 (rojo)     fuente ── GND
PC7 ──┬── R10 (100 kΩ) ── GND
      └── puerta de Q2       drenador ── R7 (100 Ω) ── J6.3 (verde)    fuente ── GND
PD0 ──┬── R11 (100 kΩ) ── GND
      └── puerta de Q3       drenador ── R8 (100 Ω) ── J6.4 (azul)     fuente ── GND
```

### 5.7 Programación y puntos de test

| Ref | Qué | Pines |
|---|---|---|
| J5 | Programación | 1 = 3V3, 2 = GND, 3 = SWIO |
| TP1 | DATA | — |
| TP2–TP5 | SENSE L / R / U / D | — |
| TP6 | **GND** (pinza de masa del analizador) | — |
| TP7 | +3V3 (punto de test de la alimentación del micro) | — |

---

## 6. Verificación del esquemático

### 6.1 ERC: dónde van los PWR_FLAG

El ERC comprueba que cada red de alimentación esté "alimentada por una salida de potencia". Los conectores son pasivos, así que hay que indicárselo — **pero solo donde el ERC no puede saberlo**:

| Red | PWR_FLAG | Por qué |
|---|---|---|
| `+5V` | **Sí** | Viene de un conector, que es pasivo |
| `GND` | **Sí** | Solo toca pines pasivos y entradas |
| `+3V3` | **No** | La salida del LDO ya es *power output*: alimenta la red |

> Si pones un PWR_FLAG en `+3V3`, el ERC avisa de **dos salidas de potencia en la misma red** — que en un circuito real es un cortocircuito entre fuentes. Los dos errores son simétricos: uno dice "nadie alimenta esta red" y el otro "hay dos cosas alimentándola".

**Y no pongas etiquetas de texto con el mismo nombre que un símbolo de alimentación.** El símbolo `+3V3` ya *es* la etiqueta; añadir una etiqueta `3V3` sobre el mismo cable produce dos nombres para una red.

### 6.2 La verificación que ninguna herramienta hace

> ⚠️ **Comprueba en el net inspector que cada red SENSE tiene exactamente 3 elementos:** el pin del micro, la resistencia y un pin de conector.
>
> Si los cuatro SENSE acabaran unidos, el circuito sería eléctricamente válido —el ERC no diría nada— pero el sistema perdería el descubrimiento de posición, que es su función más distintiva. Es el error más grave posible en esta placa.

---

## 7. Layout

### 7.1 Reglas de diseño

Importables de la devboard (*File → Import Settings from Another Board*):

| Parámetro | Valor |
|---|---|
| Capas | 2 (F.Cu, B.Cu) |
| Espesor | 1,6 mm |
| Clearance mínimo | 0,2 mm |
| Ancho de pista mínimo | 0,2 mm |
| Taladro mínimo | 0,3 mm |
| Diámetro de via | 0,6 mm |

**Net classes:**

| Clase | Redes | Ancho |
|---|---|---|
| `Default` | señales | 0,25 mm |
| `Power` | `+5V`, `+3V3`, `GND` | **0,8 mm** |

> El 5V es **bus de paso**: la corriente de todos los módulos que cuelgan de este lo atraviesa. Con 10 LED a 40 mA más el reposo, pueden pasar ~500 mA por una placa intermedia. De ahí los 0,8 mm.

### 7.2 Contorno y mecánica

- **~30 × 30 mm**, esquinas redondeadas 1 mm. Ajústalo a lo que pidan los cuatro box header, que son lo que más ocupa.
- **2 taladros M2** (2,2 mm) en esquinas opuestas.
- Los cuatro box header **hacia los cuatro bordes**, uno por lado, con la **muesca de la carcasa hacia el centro de la placa**. Si no, la llave mecánica no protege de nada.

### 7.3 Reparto por caras

| Cara | Qué va |
|---|---|
| **F.Cu** | **Todo el SMD**: micro, LDO, MOSFET, pasivos y los puentes |
| **B.Cu** | **El plano de masa** y los puntos de test |

> **Todo el SMD en F.Cu, sin excepción.** Economic PCBA de JLCPCB solo puebla una cara; un componente en la inferior obliga a pasar a Standard, que cuesta bastante más.
>
> Los THT (J1–J7) van donde convenga mecánicamente: Economic no los monta en ningún caso, los sueldas tú.

### 7.4 Orden de enrutado

| # | Qué | Capa | Ancho |
|---|---|---|---|
| 1 | GND y 5V entre los 4 conectores | F.Cu + plano | 0,8 mm |
| 2 | LDO y desacoplo, pegados al micro | F.Cu | 0,8 / 0,5 |
| 3 | DATA a los 4 conectores | F.Cu | 0,25 |
| 4 | Los 4 SENSE, **cada uno a su conector** | F.Cu | 0,25 |
| 5 | Los 8 puentes al micro y a GND | F.Cu | 0,25 |
| 6 | J5, J6, J7 y puntos de test | F.Cu | 0,25 |
| 7 | **Plano de GND en B.Cu** | B.Cu | — |

**El 5V merece atención:** al ser bus de paso, une los pines 2 y 6 de los cuatro conectores por el camino más directo — pista ancha o zona rellena. El LDO **se cuelga** de esa red, no la interrumpe. Enrutarlo "en serie" pasando por cada componente acumula resistencia y empeora la caída de tensión.

### 7.5 El plano de masa

1. Capa **B.Cu**, herramienta *Add Filled Zone* (`Ctrl+Shift+Z`), red `GND`.
2. Rectángulo cubriendo toda la placa.
3. Tecla `B` para rellenar.
4. Una via junto a cada pad de GND de la cara superior.

> **GND no se enruta: se rellena.** Y **no cortes el plano con pistas largas por B.Cu.** Una pista por la cara inferior no pasa "por debajo" del plano: lo atraviesa abriendo una ranura, y las corrientes de retorno de todas las demás señales tienen que rodearla. Baja a B.Cu solo con dos vias y un tramo corto, para saltar por encima de otra pista.
>
> A 48 MHz, una pista de 26 mm por arriba es eléctricamente irrelevante. Una ranura en el plano, no. Si una pista te sale larga, **mueve el componente**, no la cambies de capa.

### 7.6 Serigrafía

Esta placa se monta 10 veces en el primer pedido (hasta 31 en el máximo) y se configura a mano cada una. La serigrafía es la diferencia entre montar de memoria o consultar el esquemático veinticinco veces.

| Qué | Dónde |
|---|---|
| **Los pesos `1 2 4 8 16`** | Junto a JP0–JP4. Sin esto, poner la dirección es adivinar |
| **Casilla en blanco `DIR:`** | Para rotular la dirección con rotulador |
| **`PAREJA ←` y `PAREJA ↓`** | Junto a JP7 y JP8 |
| **`L` `R` `U` `D` en letra grande** | Junto a cada box header — ver el aviso |
| `GND 5V DATA SENSE GND 5V` | Junto al pin 1 de un conector |
| Un `▲` en el pin 1 de cada conector | — |
| `3V3 GND SWIO` + `(!) 3V3 solo sin 5V en bus` | En J5 |
| `5V R G B` | En J6 |
| `SW` | En J7 |
| Nombre de cada punto de test | — |
| `Modular_Drawers v2` + fecha | — |

Tamaño mínimo legible que imprime JLCPCB: **0,8 mm de alto, 0,15 mm de grosor**.

> ⚠️ **Rotula las caras, no los números de conector.** Con una placa pequeña y flotante, los cuatro conectores están juntos y ya no caen junto a su borde: **es el cable el que decide a qué cara va cada uno**. Si el latiguillo de la izquierda se enchufa en J3, el mapa de Home Assistant sale mal, y lo único que puede detectarlo es que el controlador vea una conexión no recíproca. `L R U D` grandes y legibles: al montar se lee la cara, nunca el número.

> ⚠️ **El aviso de J5 importa.** Si el módulo recibe 5 V por el bus y además se conecta el 3V3 del WCH-LinkE, el LDO pelea con el programador. Aquí es más fácil de olvidar que en una placa de desarrollo, porque el módulo se alimenta solo al enchufarlo al bus.

### 7.7 Comprobación final

| | Qué | Cómo |
|---|---|---|
| ☐ | DRC a 0 errores | *Inspect → Design Rules Checker* |
| ☐ | Ninguna línea de ratsnest sin resolver | No deben quedar líneas finas |
| ☐ | Las 4 redes SENSE separadas | Net inspector (§6.2) |
| ☐ | Todo el SMD en F.Cu | Vista 3D |
| ☐ | Serigrafía legible y sin pisar pads | Vista 3D; el DRC avisa |
| ☐ | Los conectores enchufados caben en los 20 mm de zona trasera | **Medir** el conjunto macho+hembra |

---

## 8. Fabricación

### 8.1 Archivos

Instala el plugin **Fabrication Toolkit** (*Plugin and Content Manager*). Genera de un botón el ZIP de gerbers, el BOM y el CPL con el formato que espera JLCPCB, respetando los DNP.

| Archivo | Qué es |
|---|---|
| Gerbers (`.gbr`) | Una imagen por capa |
| Taladros (`.drl`) | Dónde perforar y con qué diámetro |
| **BOM** (`.csv`) | Qué pieza va en cada referencia — necesita el campo `LCSC` |
| **CPL** (`.csv`) | Dónde y con qué rotación va cada pieza |

**Antes de generar:**

1. Campo `LCSC` rellenado en todos los componentes SMD (U1, U2, Q1–Q3, R1–R11, C1–C4).
2. **J1–J7 marcados como "excluir del BOM"**: son THT y se sueldan a mano.
3. C5 en DNP.
4. **Las resistencias del LED confirmadas en JLCPCB** (tipo Basic/Extended y stock) antes de generar el BOM. Q1–Q3 (C8545) ya están confirmados como Basic.

### 8.2 Verificar antes de pagar

Sube el ZIP a `gerber-viewer.easyeda.com` y revisa capa por capa: contorno cerrado del tamaño correcto, taladros en su sitio, serigrafía legible y sin caer sobre pads, plano de masa sin islas desconectadas.

**Y revisa la previsualización del montaje que muestra JLCPCB**, pieza a pieza.

> ⚠️ **La trampa de las rotaciones.** KiCad y las máquinas de JLCPCB no siempre coinciden en qué es la rotación 0° de un encapsulado. El caso clásico es el **SOT-23 del LDO**, que puede salir girado 180° dejando VIN y VOUT cruzados. La previsualización es la última oportunidad de cazarlo.

### 8.3 El pedido

| Opción | Valor |
|---|---|
| Capas / espesor | 2 / 1,6 mm |
| Acabado | HASL con plomo (más barato y mejor para soldar a mano) |
| Montaje | **Economic PCBA**, una cara |
| Cantidad | **10 unidades** (primer pedido; el diseño admite hasta 31 módulos) |
| Coste estimado | ~45 $ + IVA del 21 % a la entrada |

**Desglose de los costes fijos:** ~8 $ de setup + ~3 $ de feeder por el único componente Extended.

### 8.4 Montaje después de recibir

JLCPCB entrega los SMD soldados. Quedan los THT, ~10 minutos por placa: los cuatro box header, J5, J6 y J7. Pincha las tiras en una protoboard y apoya la placa encima para que queden perpendiculares.

Y los puentes: cierra con una gota de estaño los de la dirección que toque, más JP7/JP8 si el módulo va emparejado.

---

## 9. Lo que falta antes de pedir

| # | Bloqueo | Se resuelve con |
|---|---|---|
| 1 | ¿Caben los conectores enchufados en los 20 mm de zona trasera? | **Medir** macho+hembra (fase H0) |
| 2 | ¿Funciona el bus UART half-duplex open-drain? | Fases 3–4 del firmware (H1) |
| 3 | ¿El LED RGB es de ánodo común y se enciende bien con los 2N7002 y las resistencias de §4? | Fase 2 del firmware, con un LED en protoboard |
| 4 | ¿Los SENSE y el algoritmo de topología funcionan? | Fase 5 del firmware |
| 5 | ¿1 kΩ basta en DATA y SENSE? | Fase H2 con 5 módulos |

**Lo que ya NO bloquea:** la posición de los taladros y del interruptor. Con la placa pequeña y flotante (§1.1), el armazón v2 ha dejado de ser un requisito previo. Los cinco puntos restantes se validan **gratis en protoboard**.

---

## 10. Exportaciones para el diseño 3D

El paso que conecta las dos mitades del proyecto:

| Exportar | Para qué |
|---|---|
| **DXF** del contorno (*File → Export → DXF*, solo Edge.Cuts) | Referencia en OpenSCAD para los soportes de la placa |
| **STEP** de la placa (*File → Export → STEP*) | Comprobar en FreeCAD que los conectores enchufados no se salen del contorno del módulo |

---

## 11. Los tres errores que no perdonan

Los tres tienen algo en común: **la placa funciona, pasa ERC y DRC, y el sistema no hace lo que debe.** Hay que cazarlos leyendo.

**1 · Unir los SENSE.** Rompe el descubrimiento de posición. Verifica las cuatro redes en el net inspector: 3 elementos cada una.

**2 · Montar un LED de cátodo común.** El circuito de §5.6 supone **ánodo común** (+5 V en la pata común). Con cátodo común, el LED no enciende o enciende al revés. Comprueba la polaridad en la ficha y con una pila de 3 V y 330 Ω antes de pedir las placas.

**3 · Un condensador grande en la entrada de 5 V.** Enchufar un cajón en marcha reiniciaría a los vecinos, incumpliendo el requisito **R2**. Máximo 10 µF.
