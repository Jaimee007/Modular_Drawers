# Modular_Drawers

Cajonera modular inteligente impresa en 3D. Cada unidad de retícula lleva su propio microcontrolador (CH32V003), un LED RGB y un final de carrera. Los módulos se unen con cola de milano y se conectan entre sí con latiguillos de cable plano IDC. Un ESP32-C3 en el módulo base **descubre automáticamente la posición de cada cajón** y lo integra con Home Assistant, para encender el cajón donde está lo que buscas.

## Documentación

La documentación está separada en cuatro niveles, del "qué" al "cómo". **Si solo vas a leer uno, lee el primero.**

| # | Archivo | Contenido | Cambia cuando… |
|---|---|---|---|
| 1 | [docs/01_especificacion_funcional.md](docs/01_especificacion_funcional.md) | **Qué hace el sistema**: requisitos, comportamiento, estados, criterios de aceptación. Sin un número de pin dentro | …cambia el comportamiento del producto |
| 2 | [docs/02_diseno_hardware.md](docs/02_diseno_hardware.md) | **Cómo está construido**: arquitectura, componentes, protocolo del bus, alimentación, mecánica | …se cambia un componente o una decisión técnica |
| 3 | [docs/03_diseno_pcb.md](docs/03_diseno_pcb.md) | **Las placas en KiCad**: esquemáticos, layout, reglas de diseño, pedido a JLCPCB | …se rediseña una placa |
| 4 | [docs/04_diseno_software.md](docs/04_diseno_software.md) | **El código**: firmware del módulo y del controlador, integración en HA, plan de pruebas por fases | …cambia el firmware |

Y aparte:

| Archivo | Contenido |
|---|---|
| [docs/bom_5_cajones.md](docs/bom_5_cajones.md) | Lista de materiales para un prototipo de 5 cajones, con referencias de LCSC y JLCPCB |

## Herramientas

- `tools/gen_esquematico.py`: genera un esquemático de referencia en HTML. Uso: `python tools/gen_esquematico.py` (solo librería estándar de Python).

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
