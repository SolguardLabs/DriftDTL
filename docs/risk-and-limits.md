# Riesgo y límites

## Marco operativo

El control se aplica en tres momentos: configuración del canal, admisión del paquete y seguimiento
del capital. Ningún indicador sustituye las reglas duras del motor.

```mermaid
flowchart LR
    C["Configuración"] --> P["Política del canal"]
    P --> A["Admisión por paquete"]
    A --> E["Ejecución"]
    E --> M["Métricas de capital"]
    M --> D{"Banda operativa"}
    D -->|amplia| A
    D -->|vigilada| R["Reducir capacidad"]
    D -->|restringida| H["Pausar nuevos envíos"]
```

## Límites duros

| Parámetro         | Efecto                           | Recomendación inicial   |
| ----------------- | -------------------------------- | ----------------------- |
| `minPacketAmount` | evita operaciones antieconómicas | coste operativo × 10    |
| `maxPacketAmount` | limita concentración unitaria    | ≤ 5 % de liquidez       |
| `timeoutEpochs`   | acota permanencia en cola        | p99 de confirmación × 2 |
| `maxRetries`      | limita reaperturas               | 1–2                     |
| `feeBps`          | remunera al operador             | gobernanza por activo   |

Los valores dependen de la frecuencia de epoch y del perfil del activo. Un cambio exige simulación
con volumen alto, latencia extrema y secuencias fuera de orden.

## Bandas de capital

```mermaid
stateDiagram-v2
    [*] --> Ample
    Ample --> Monitored: cobertura < 15000 o utilización > 6000
    Monitored --> Constrained: cobertura < 10000 o utilización > 8000
    Constrained --> Monitored: rebalanceo confirmado
    Monitored --> Ample: ventana estable
```

El SDK clasifica:

- `ample`: cobertura ≥ 15.000 bps y utilización ≤ 6.000 bps;
- `monitored`: cobertura ≥ 10.000 bps y utilización ≤ 8.000 bps;
- `constrained`: cualquier otra combinación.

## Escenarios de tensión

```mermaid
flowchart TB
    B["Snapshot base"] --> T1["Confirmaciones +50 %"]
    B --> T2["Latencia p99 ×2"]
    B --> T3["Concentración de canal"]
    B --> T4["Ráfaga de reintentos"]
    T1 --> K["Cobertura mínima"]
    T2 --> X["Exposición expirada"]
    T3 --> U["Utilización máxima"]
    T4 --> Q["Profundidad de cola"]
    K --> Decision["Decisión de capacidad"]
    X --> Decision
    U --> Decision
    Q --> Decision
```

Una revisión de capacidad debe incluir al menos:

1. volumen nominal y p99 por paquete;
2. participación del mayor origen y destinatario;
3. cobertura mínima por activo;
4. exposición expirada máxima;
5. tiempo de recuperación tras una pausa.

## Respuesta por umbral

| Señal                         | Acción                          |
| ----------------------------- | ------------------------------- |
| Utilización > 8.000 bps       | congelar ampliaciones de límite |
| Cobertura < 10.000 bps        | pausar admisión del activo      |
| Exposición expirada creciente | revisar latencia y cola         |
| ACK repetidos en ráfaga       | aislar integración emisora      |
| Epoch regresivo               | rechazar escenario completo     |

La reactivación requiere un snapshot reconciliado y dos ventanas estables consecutivas.
