# Arquitectura

## Propósito

DriftDTL separa validación, dominio, contabilidad y presentación para que cada transición pueda
reproducirse con la misma entrada. El proceso es deliberadamente monolítico: reduce dependencias de
ejecución y deja la orquestación distribuida en manos de la plataforma integradora.

## Mapa de componentes

```mermaid
flowchart TB
    CLI["CLI"] --> Loader["ScenarioLoader"]
    Loader --> Schema["ScenarioSchema"]
    Schema --> Runner["ScenarioRunner"]
    Runner --> Engine["SettlementEngine"]
    Engine --> Policy["PolicyEvaluator"]
    Engine --> Ledger["AccountLedger"]
    Engine --> Timeline["Timeline"]
    Engine --> Events["Event stream"]
    Ledger --> Reconcile["LedgerReconciler"]
    Engine --> Queue["SettlementQueue"]
    Reconcile --> Capital["CapitalModel"]
    Queue --> Capital
    Capital --> Report["JsonReport"]
    Events --> Report
```

| Capa       | Entrada           | Salida            | Garantía principal              |
| ---------- | ----------------- | ----------------- | ------------------------------- |
| Core       | escalares y texto | tipos comprobados | sin desbordamientos silenciosos |
| Domain     | configuración     | entidades tipadas | identificadores no vacíos       |
| Scenario   | JSON              | secuencia válida  | referencias y epochs coherentes |
| Settlement | acciones          | estados y eventos | orden determinista              |
| Ledger     | movimientos       | saldos por plano  | reservas no negativas           |
| Economics  | estado completo   | ratios por activo | métricas comparables            |
| Report     | agregados         | JSON estable      | claves y unidades explícitas    |

## Flujo de datos

```mermaid
sequenceDiagram
    autonumber
    participant F as Archivo
    participant S as Schema
    participant R as Runner
    participant E as Engine
    participant A as AccountLedger
    participant J as JsonReport
    F->>S: parse + inspect
    S-->>R: Scenario
    loop acción por epoch
        R->>E: submit/confirm/cancel/retry
        E->>A: reserva o liquidación
        A-->>E: nuevo saldo
    end
    E->>J: snapshot inmutable
    J-->>F: informe
```

El loader no modifica el estado. El runner es el único adaptador entre acciones y comandos del
motor. El informe consulta interfaces de solo lectura, de modo que observar no cambia el resultado.

## Dependencias

```mermaid
graph LR
    Core --> Domain
    Domain --> Ledger
    Domain --> Settlement
    Ledger --> Settlement
    Settlement --> Audit
    Settlement --> Economics
    Ledger --> Economics
    Audit --> Report
    Economics --> Report
    Report --> CLI
    CLI -. JSON .-> SDK
```

Las flechas no regresan hacia capas inferiores. Esta regla evita que la serialización o el SDK
condicionen la semántica contable.

## Decisiones técnicas

- C++20 ofrece tipos de valor y control explícito del ciclo de vida.
- El JSON interno evita una dependencia nativa adicional.
- `std::map` mantiene orden estable en cuentas, canales y paquetes.
- Los importes usan `int64_t`; los porcentajes usan puntos básicos.
- El SDK invoca el binario sin shell y conserva el contrato de proceso.
- CMake y el compilador directo son caminos de compilación equivalentes.

## Extensión segura

Una nueva acción requiere actualizar, en este orden, el enum de dominio, el parser, el esquema, el
runner, el motor, los eventos, el informe y las pruebas. Una nueva métrica debe derivarse desde
interfaces de solo lectura y no debe alterar el orden de eventos.
