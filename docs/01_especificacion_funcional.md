# Cajonera modular inteligente — Especificación funcional

Versión 1.0 · Octubre 2026 · **Qué hace el sistema**, no cómo está construido

> **Alcance de este documento.** Define el comportamiento observable del sistema: qué hace, qué ve el usuario y qué debe pasar cuando algo falla. **No contiene pines, valores de componentes ni referencias de fabricante.** Si un cambio de hardware obliga a editar este documento, es que el cambio altera el comportamiento del producto — y eso merece una decisión consciente.
>
> El cómo está en `02_diseno_hardware.md`, `03_diseno_pcb.md` y `04_diseno_software.md`.

---

## 1. El problema que resuelve

Un taller o un escritorio acumulan cientos de componentes pequeños — resistencias, tornillos, conectores — repartidos en cajones indistinguibles. Encontrar algo concreto exige abrir cajones a ciegas o mantener un inventario que nadie actualiza.

**La idea:** una cajonera modular donde cada cajón sabe qué guarda y puede iluminarse. Pides "resistencias de 10k" por texto o por voz, y el cajón correcto se enciende.

---

## 2. Requisitos

### 2.1 Funcionales

| # | Requisito |
|---|---|
| **F1** | Cada cajón puede iluminarse individualmente, en cualquier color, por orden del sistema |
| **F2** | El sistema detecta cuándo un cajón se abre y cuándo se cierra |
| **F3** | Al abrir un cajón, su interior se ilumina en blanco |
| **F4** | Cada cajón tiene asociada una descripción de su contenido, editable por el usuario |
| **F5** | El usuario puede buscar por texto o por voz; los cajones que coinciden se iluminan |
| **F6** | El sistema sabe en qué posición física (x, y) está cada cajón, y lo descubre **sin que el usuario lo configure** |
| **F7** | Al añadir un cajón nuevo, el sistema lo detecta y lo sitúa automáticamente, sin reiniciar nada |
| **F8** | El usuario ve una representación gráfica de la cajonera que refleja su disposición real |
| **F9** | Todo es controlable desde Home Assistant |

### 2.2 De construcción

| # | Requisito |
|---|---|
| **C1** | Modular y ampliable: se añaden cajones sin rediseñar ni reconfigurar lo existente |
| **C2** | Los cajones se unen mecánica y eléctricamente entre sí, sin cableado central |
| **C3** | Retícula de 50 mm. Cajones simples (1×1) y dobles (2×1) |
| **C4** | Capacidad de al menos 31 cajones |
| **C5** | Una sola entrada de alimentación para toda la cajonera |
| **C6** | Imprimible en 3D con una impresora doméstica, sin soportes en las piezas estructurales |
| **C7** | Coste objetivo por cajón por debajo de 5 € |

### 2.3 De fiabilidad

| # | Requisito |
|---|---|
| **R1** | **Un cajón que se abre se ilumina aunque el controlador central no responda.** La luz al abrir no depende de la red ni del Wi-Fi |
| **R2** | Enchufar o desenchufar un cajón con el sistema en marcha no debe reiniciar ni perturbar a los demás |
| **R3** | Un cajón mal conectado o mal configurado se señaliza al usuario; no deja el sistema inservible |
| **R4** | Es físicamente imposible conectar un cable entre cajones en una orientación incorrecta |
| **R5** | El sistema se recupera solo tras un corte de luz o un reinicio del controlador |
| **R6** | La descripción del contenido de los cajones sobrevive a reinicios |

---

## 3. Modelo conceptual

```
   Usuario ──► Home Assistant ──► Controlador ──► Cajones
   (voz/texto)   (interfaz)       (descubre y     (iluminan y
                                   coordina)       detectan)
```

Tres conceptos que conviene no confundir:

| Concepto | Qué es | Quién lo determina |
|---|---|---|
| **Dirección** | *Quién* es un módulo. Un número único, 1–31 | El usuario, al montar el módulo |
| **Posición** | *Dónde* está, en coordenadas (x, y) de la retícula | **El sistema, automáticamente** |
| **Emparejamiento** | Si un módulo forma parte de un cajón más grande junto a un vecino | El usuario, al montar el módulo |

**Un módulo es una unidad de retícula, no un cajón.** Un cajón 1×1 es un módulo; un cajón 2×1 son dos módulos emparejados que se comportan como uno. Esta distinción es la que permite que exista un solo tipo de módulo para cualquier forma de cajón.

---

## 4. Comportamiento de un cajón

### 4.1 Estados del indicador luminoso

Cada cajón tiene un indicador visible por delante (con el cajón cerrado) y que ilumina el interior (con el cajón abierto).

| Estado | Indicación | Quién lo decide |
|---|---|---|
| Reposo | Apagado | — |
| **Buscado** | Azul parpadeante | El controlador |
| **Abierto** | Blanco | **El propio cajón** |
| **Abierto y buscado** | Verde 2 s, luego blanco; la búsqueda se cancela | **El propio cajón** |
| **Sin dirección asignada** | Ámbar parpadeante | **El propio cajón** |
| **Error** (dirección duplicada, mal conectado) | Rojo parpadeante | El controlador |
| **Identificar** | Parpadeo rápido o arcoíris | El controlador |

**Regla de prioridad:** lo que decide el propio cajón manda sobre lo que pida el controlador. Es lo que garantiza **R1**.

### 4.2 Qué pasa al abrir un cajón

1. El cajón detecta la apertura y **enciende su luz en blanco inmediatamente**, sin consultar a nadie.
2. Si el cajón estaba siendo buscado, muestra verde 2 segundos ("encontrado") y luego blanco, y la búsqueda se cancela.
3. El controlador se entera en menos de medio segundo y lo refleja en Home Assistant.

### 4.3 Qué pasa al añadir un cajón

1. Se monta el módulo: se le asigna una dirección libre y se conecta por cable a algún vecino.
2. **Al recibir corriente, el módulo se anuncia a sus vecinos.**
3. El controlador detecta el módulo nuevo, averigua su posición y lo publica en Home Assistant: *"Cajón #21 añadido en (3,1)"*.
4. **Nada se reinicia y nada hay que configurar.** (**F7**, **C1**)

### 4.4 Comportamiento ante fallos

| Situación | Qué debe ocurrir |
|---|---|
| Un módulo sin dirección asignada | Parpadea en ámbar y el controlador lo avisa en HA |
| Dos módulos con la misma dirección | El controlador lo detecta y lo avisa: *"dirección 12 duplicada"* |
| Un cable suelto o un contacto defectuoso | El cajón sigue respondiendo por el bus si le llega corriente, pero aparece "sin posición" |
| Un cable conectado a la cara equivocada | El controlador detecta una conexión incoherente (no recíproca) y la marca como error de montaje |
| Un módulo declara pareja donde no tiene vecino | El controlador lo avisa como error de montaje |
| El controlador no responde | Los cajones siguen iluminándose al abrirse (**R1**). Se pierde la búsqueda |
| Corte de alimentación | Al volver, el sistema se redescubre solo y recupera las descripciones de contenido (**R5**, **R6**) |
| Un módulo se cuelga | Se reinicia solo y vuelve a responder |

---

## 5. Comportamiento del sistema

### 5.1 Búsqueda

1. El usuario escribe o dice qué busca ("condensadores de 100 nanos").
2. El sistema busca ese texto en las descripciones de todos los cajones.
3. Los que coinciden **parpadean en azul**.
4. La búsqueda termina cuando se abre uno de ellos, o a los **30 segundos**.

**Decisiones de comportamiento:**

| Pregunta | Respuesta |
|---|---|
| ¿Qué pasa si coinciden varios cajones? | Se iluminan todos a la vez |
| ¿Y si coinciden muchos? | Se limita el número de cajones encendidos simultáneamente (ver §5.4) |
| ¿Si no coincide ninguno? | El sistema lo indica en la interfaz; no se ilumina nada |
| ¿Si se lanza una búsqueda con otra en curso? | La nueva sustituye a la anterior |
| ¿Si se abre un cajón que no era el buscado? | Se ilumina en blanco (es un cajón abierto) y la búsqueda sigue activa |

### 5.2 Descubrimiento de la posición

El sistema **nunca pregunta al usuario dónde está un cajón.** Lo deduce:

1. El módulo controlador está en el origen (0,0).
2. El sistema interroga a cada módulo conocido por cada una de sus cuatro caras, preguntando qué vecino hay al otro lado.
3. Con cada par de caras encontrado, calcula la posición del vecino.
4. Recorre así toda la cajonera, partiendo del controlador.

**Requisitos de comportamiento:**

| | |
|---|---|
| Un mapa completo de 31 módulos | menos de 10 segundos |
| Cuándo se ejecuta | al arrancar, y cuando se detecta un cambio |
| Dos módulos en la misma celda | error de montaje, se avisa |
| Una conexión no recíproca | error de montaje, se avisa |
| Un módulo sin camino hasta el controlador | aparece como "sin posición", pero no rompe nada |

> **No hace falta conectar todas las caras.** Basta con que cada módulo tenga alguna cadena de cables hasta el controlador. Solo se pierde la detección de vecinos en las caras que se dejen sin cable.

### 5.3 Cajones dobles

Un cajón 2×1 son dos módulos emparejados. El comportamiento esperado:

| | |
|---|---|
| En Home Assistant | aparece como **un solo cajón** |
| Al buscarlo | se iluminan los dos módulos, con el mismo color |
| Al abrirlo | los dos se iluminan en blanco, cada uno por su cuenta (**R1**) |
| Su descripción de contenido | una sola, compartida |
| Si el emparejamiento no es coherente con el mapa | error de montaje, se avisa |

### 5.4 Límites operativos

| Límite | Valor | Por qué |
|---|---|---|
| Cajones iluminados a la vez | 10 | Consumo y caída de tensión en la cadena de cables |
| Brillo máximo | limitado por diseño | Idem |
| Latencia de una orden de iluminación | < 0,5 s | Percepción de inmediatez |
| Latencia de detección de apertura | < 0,5 s | Idem |
| Duración de una búsqueda sin respuesta | 30 s | Evitar luces encendidas indefinidamente |

---

## 6. Lo que el usuario ve en Home Assistant

Por cada cajón:

| Entidad | Para qué |
|---|---|
| Luz | Encender en cualquier color o efecto |
| Sensor de apertura | Saber si está abierto |
| Texto de contenido | Describir lo que guarda |
| Posición y tamaño | Dibujar la cajonera |

Y a nivel de sistema:

- **Buscar contenido**: la acción principal, por texto o por voz.
- **Panel gráfico**: una cuadrícula que refleja la disposición real. Al tocar un cajón, se identifica parpadeando.
- **Redescubrir**: forzar un nuevo mapeo.
- **Avisos**: cajones nuevos, direcciones duplicadas, errores de montaje, módulos desconectados.

---

## 7. Fuera de alcance

Decisiones explícitas sobre lo que el sistema **no** hace, para evitar que crezca sin control:

| No hace | Por qué |
|---|---|
| Detectar automáticamente qué hay dentro de un cajón | Requeriría sensores o visión; el contenido lo describe el usuario |
| Contar unidades o avisar de existencias bajas | Misma razón |
| Funcionar sin el controlador para búsquedas | La búsqueda necesita coordinación central. Solo la luz al abrir es autónoma (**R1**) |
| Cerrar o abrir cajones por sí mismo | No hay actuadores; es una cajonera, no un robot |
| Identificar quién abre un cajón | Sin interés para el caso de uso |

---

## 8. Criterios de aceptación del sistema completo

El proyecto está terminado cuando todo esto se cumple con hardware real:

| # | Criterio |
|---|---|
| **A1** | Escribir o decir "resistencias de 10k" ilumina el cajón correcto en menos de 1 s |
| **A2** | Abrir ese cajón lo pone en verde 2 s y luego en blanco, y cancela la búsqueda |
| **A3** | Abrir cualquier cajón ilumina su interior en blanco **con el controlador desconectado** |
| **A4** | Añadir un cajón nuevo en marcha lo hace aparecer en HA, en su posición correcta, sin reiniciar nada |
| **A5** | Desenchufar y volver a enchufar un cajón no perturba a los demás |
| **A6** | Reordenar físicamente los cajones y redescubrir da el mapa correcto |
| **A7** | Montar dos cajones con la misma dirección produce un aviso claro en HA |
| **A8** | Un cajón 2×1 aparece como un solo cajón y sus dos módulos se iluminan juntos |
| **A9** | Tras un corte de luz, el sistema se recupera solo con las descripciones intactas |
| **A10** | 24 horas de funcionamiento continuo sin reinicios inesperados |
| **A11** | 100 aperturas y cierres de un cajón sin un solo fallo de detección |
