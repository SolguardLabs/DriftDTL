# Seguridad De DriftDTL

## Modelo De Seguridad

DriftDTL asume un entorno de ejecucion local y determinista. Las entradas
externas son fixtures JSON revisables, y el motor no realiza llamadas de red ni
accede a servicios externos.

Los operadores de lane pueden configurar:

- activo liquidado;
- cuenta operativa de fees;
- timeout por epoch;
- fee maxima dentro de rango soportado;
- limites minimo y maximo por paquete;
- numero maximo de reintentos;
- politica de ACK unico.

## Invariantes Esperadas

- Ninguna cuenta puede reservar mas liquidez visible de la disponible.
- Un paquete no puede superar los limites de su lane.
- Un ACK duplicado no debe acreditar dos veces el mismo paquete.
- Un paquete cerrado no debe reabrirse sin pasar por una accion de retry
  valida.
- Las cantidades son enteros no negativos y las operaciones usan aritmetica
  comprobada.

## Validaciones Automatizadas

La suite TypeScript cubre:

- settlement normal;
- paquetes confirmados fuera de orden;
- confirmaciones duplicadas;
- cancelaciones por timeout;
- retry posterior a timeout;
- contrato JSON de la CLI.

Ejecutar:

```bash
npm test
```

## Dependencias

El motor C++ no usa dependencias externas. La capa TypeScript usa dependencias
de desarrollo para ejecutar tests y formateo.

Dependabot revisa:

- npm;
- GitHub Actions.

## Alcance De Revision

Revisar especialmente:

- `src/settlement`;
- `src/ledger`;
- `src/scenario`;
- fixtures usados por integraciones internas;
- cambios de politica de lane.

## Reporte Interno

Los hallazgos deben incluir:

- fixture minimo de reproduccion;
- estado esperado y observado;
- impacto contable;
- propuesta de mitigacion;
- test de regresion recomendado.
