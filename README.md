# Modular_Drawers

Cajonera modular inteligente impresa en 3D. Cada unidad de retícula lleva su propio microcontrolador (CH32V003), un LED RGB y un final de carrera. Los módulos se unen con cola de milano y se conectan entre sí con latiguillos de cable plano IDC. Un ESP32-C3 en el módulo base **descubre automáticamente la posición de cada cajón** y lo integra con Home Assistant, para encender el cajón donde está lo que buscas.

## Documentación

**¿Por dónde empiezo?** Para entender el proyecto, lee el **01**. Para ver cómo está construido, el **02**. Si vas a montar o programar algo en el banco, ve directo a la sección «Montaje y pruebas».

Cada documento responde a una sola pregunta. Si dos documentos parecen decir cosas distintas, **manda el 02** (el diseño de hardware), y el resto se corrige para coincidir con él.

### Qué es y cómo está diseñado

| # | Archivo | Responde a… | Cambia cuando… |
|---|---|---|---|
| 01 | [especificación funcional](docs/01_especificacion_funcional.md) | **¿Qué hace el sistema?** Requisitos, estados, criterios de aceptación. Sin pines | …cambia el comportamiento del producto |
| 02 | [diseño de hardware](docs/02_diseno_hardware.md) | **¿Cómo está construido?** Arquitectura, bus, LED, alimentación, mecánica, **pines del ESP32** | …se cambia un componente o una decisión técnica |
| 03 | [diseño de PCB](docs/03_diseno_pcb.md) | **¿Cómo son las placas?** Esquemático, layout, pedido a JLCPCB | …se rediseña una placa |
| 04 | [diseño de software](docs/04_diseno_software.md) | **¿Cómo es el código?** Firmware del módulo y de la base, Home Assistant | …cambia el firmware |

### Montaje y pruebas (lo que se hace en el banco)

| # | Archivo | Responde a… |
|---|---|---|
| 05 | [montaje de la base](docs/05_montaje_base.md) | **¿Cómo monto la base en placa perforada?** |
| 06 | [montaje en protoboard](docs/06_montaje_protoboard.md) | **¿Cómo cableo la base y las dos TENSTAR para probar el bus y SENSE?** |
| 07 | [pruebas de banco](docs/07_pruebas_banco.md) | **¿Qué programa pruebo, con qué entorno, y cuándo está bien?** Índice de `firmware/` |
| — | [plan de firmware](docs/plan_firmware.md) | **¿En qué orden programo?** Fases 0 a 9, cada una con su prueba de aceptación |

### Compras

| Archivo | Responde a… |
|---|---|
| [BOM del prototipo](docs/bom_5_cajones.md) | **¿Qué compro y dónde?** Lista de materiales, referencias LCSC y JLCPCB |

### Histórico

| Archivo | Nota |
|---|---|
| [plan de diseño original](docs/archivo/plan_cajonera_modular.md) | **No vigente.** Contradice al diseño actual (LED direccionable, 6 bits de dirección); se conserva por las decisiones que recoge |
| [esquemático de referencia](docs/esquematico_2_cajones.html) | HTML generado por `tools/gen_esquematico.py` |

## Código y herramientas

| Carpeta | Contenido |
|---|---|
| `firmware/module/` | Firmware del CH32V003. Un entorno de PlatformIO por programa (`phaseN_<función>`) |
| `firmware/base_test/` | Firmware del ESP32-C3. Un entorno por programa (`base_<función>`) |
| `firmware/common/bus.h` | Protocolo y CRC, **compartido** por los dos proyectos |
| `Hardware/` | Proyectos de KiCad: `modulo_cajon`, `devboard_ch32v003` y las hojas de simulación |
| `tools/gen_esquematico.py` | Genera el esquemático de referencia en HTML. Solo librería estándar de Python |

## Resumen técnico

| | |
|---|---|
| **Retícula** | 50 mm. Profundidad 130 mm |
| **Un módulo = una unidad de retícula** | Un cajón 1×1 es un módulo; un 2×1 son dos módulos emparejados. **Una sola placa para todo** |
| **Micro por módulo** | CH32V003F4P6 (TSSOP-20), ~0,15 $ |
| **Dirección** | 1–31, por 5 puentes de soldadura |
| **Emparejamiento** | 2 puentes direccionales (pareja a la izquierda / abajo) |
| **Bus entre módulos** | Cable plano de 6 hilos: GND, 5V, DATA, SENSE, GND, 5V. **Box header 2×3 en las cuatro caras**; el cable nunca se suelda a la placa |
| **Posición automática** | Línea SENSE punto a punto entre vecinos. El usuario nunca configura posiciones |
| **Base** | ESP32-C3 SuperMini + una única entrada USB-C de 5 V para toda la cajonera |
| **Coste por módulo** | ~2,7 € |

## Estado del proyecto

| Fase | Estado |
|---|---|
| H0 · Compras | En curso |
| H1 · Protoboard | Pendiente |
| H2 · Placa perforada en armazones v1 | Pendiente |
| H3 · PCB en KiCad | **En curso** |
| H4 · Firmware e integración | Pendiente |

## Principios de diseño

Cuatro ideas que explican la mayoría de las decisiones:

1. **La posición se descubre, no se configura.** El usuario monta un cajón y el sistema averigua dónde está.
2. **Un cajón que se abre se ilumina aunque el controlador no responda.** La función más básica no depende de la red.
3. **En la PCB solo se suelda el conector; el cable nunca toca la placa.** Los cables son consumibles reemplazables.
4. **Las placas son pequeñas y flotantes.** No cumplen funciones mecánicas: para llegar a los sitios están los cables. Esto desacopla el diseño electrónico del diseño 3D.
