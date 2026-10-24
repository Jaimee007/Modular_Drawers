# Modular_Drawers

Cajonera modular inteligente impresa en 3D. Cada cajón lleva su propio microcontrolador (CH32V003), un LED RGBW y un final de carrera. Los cajones se unen con cola de milano y se conectan entre sí con cable plano IDC. Un ESP32-C3 en el módulo base detecta la posición de cada cajón y lo integra con Home Assistant, para encender el cajón donde está lo que buscas.

## Documentación

| Archivo | Contenido |
|---|---|
| [docs/plan_cajonera_modular.md](docs/plan_cajonera_modular.md) | Plan de diseño: requisitos, arquitectura, hardware, diseño 3D, firmware y riesgos |
| [docs/bom_5_cajones.md](docs/bom_5_cajones.md) | Lista de materiales para un prototipo de 5 cajones + referencias de JLCPCB |
| [docs/esquematico_2_cajones.html](docs/esquematico_2_cajones.html) | Esquemático de referencia (base + 2 cajones). Abrir en el navegador |

## Herramientas

- `tools/gen_esquematico.py`: genera `docs/esquematico_2_cajones.html`. Uso: `python tools/gen_esquematico.py` (solo librería estándar de Python).

## Resumen técnico

- **Retícula:** 50 mm (cajones 1×1 de 50×50 y 2×1 de 100×50), profundidad 130 mm.
- **Micro por cajón:** CH32V003F4P6 (TSSOP-20). Dirección 1–63 con 6 puentes de soldadura.
- **Bus entre cajones:** cable plano de 6 hilos (GND, 5V, DATA, SENSE, GND, 5V). Coleta soldada en las caras derecha y superior; conector macho 2×3 con llave en las izquierda e inferior.
- **Posición automática:** línea SENSE punto a punto entre vecinos.
- **Base:** ESP32-C3 SuperMini + entrada USB-C de 5 V.
