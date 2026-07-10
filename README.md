# DriftDTL

![banner](./assets/banner.png)

DriftDTL es un motor C++ de settlement asincronico para paquetes DTL con
confirmaciones tardias, timeouts, reintentos y accounting por planos de
observacion y confirmacion.

El repositorio esta pensado para auditoria local: no abre puertos, no requiere
servicios externos y todos los escenarios se ejecutan desde fixtures JSON.

## Componentes

- `src/core`: cantidades enteras, errores, utilidades de texto y JSON
  autocontenido.
- `src/domain`: entidades del protocolo, IDs, lanes, politicas y acciones.
- `src/ledger`: saldos disponibles, reservas observadas, reservas confirmadas,
  creditos recibidos y fees.
- `src/settlement`: politicas de admision y motor de estado asincronico.
- `src/scenario`: carga y ejecucion de fixtures JSON.
- `src/audit`: metricas de consistencia e invariantes operativas.
- `src/report`: salida JSON estable para integraciones y tests.
- `tests/fixtures`: escenarios reproducibles de settlement.
- `tests/node`: tests TypeScript con `node:test`.

## Requisitos

- Node.js 20 o superior.
- Un compilador C++20 disponible como `g++`, `clang++`, `c++` o MSVC `cl`.

En Windows, el script de build intenta localizar `vcvars64.bat` de Visual
Studio si `cl` no esta ya en `PATH`.

## Uso

Instalar dependencias:

```bash
npm install
```

Compilar:

```bash
npm run build
```

Validar un fixture:

```bash
out/driftdtl validate tests/fixtures/balanced_settlement.json
```

Ejecutar un escenario:

```bash
out/driftdtl run tests/fixtures/out_of_order_delivery.json --json --events
```

Ejecutar tests:

```bash
npm test
```

Validacion completa:

```bash
npm run ci
```

## Modelo De Fixture

Un fixture declara:

- cuentas iniciales y balances por activo;
- lanes de settlement con activo, operador y politica;
- acciones ordenadas por epoch (`submit`, `confirm`, `cancel`, `retry`,
  `advance`, `snapshot`).

Los importes son enteros no negativos. No se aceptan decimales ni notacion
exponencial.

## Salida JSON

La CLI devuelve:

- resumen agregado de epoch, settled, fees, locks y paquetes;
- cuentas y balances por activo;
- lanes cargadas;
- settlements con estado observado y confirmado;
- metricas de auditoria;
- eventos opcionales cuando se usa `--events`.

## Estado Del Lab

DriftDTL es un laboratorio autocontenido de revision de settlement. El contrato
principal para herramientas externas es la salida JSON generada por la CLI.
