# Transductor.cydsn

> **[Placeholder — para completar por el profesor]**
> *El rol especifico de este proyecto dentro del sistema sera explicado por el docente a cargo.*

---

## Descripcion del firmware (estado actual del codigo)

Proyecto de PSoC Creator para el kit **CY8CKIT-059** (PSoC 5LP). Implementa la adquisicion de senal desde un transductor piezoelectrico mediante ADC con DMA y comunicacion UART con la PC.

### Hardware utilizado

| Componente | Funcion |
|------------|---------|
| `TIA_1`, `TIA_2` | Amplificadores de transimpedancia para acondicionamiento de la senal del sensor |
| `Opamp_1`, `Opamp_2` | Amplificadores operacionales de soporte |
| `ADC_SAR_1` | ADC de aproximaciones sucesivas — activo (12 bits) |
| `ADC_DelSig_1` | ADC delta-sigma — disponible pero no utilizado actualmente |
| `DMA_1` | Transferencia directa de memoria: ADC → buffer RAM sin intervencion del CPU |
| `UART_1` | Comunicacion serie con la PC a 115200 baudios |
| `Clock_ADC`, `Clock_Trig` | Relojes de temporización para ADC y trigger |

### Flujo de operacion (`main.c`)

El firmware implementa una maquina de estados controlada por la variable `flag`:

```
Estado '0'  Espera trigger
            Habilita interrupciones (Fin_Adq, RxIsr)
            Espera byte 'T' por UART
                |
                | (llega 'T' → RxIsr escribe Reg_Received = 1)
                v
Estado '1'  Adquisicion en curso
            DMA transfiere 2000 muestras del ADC_SAR_1 al arreglo datos[] (int16)
                |
                | (DMA completa → interrupcion Fin_Adq)
                v
Estado '2'  Conversion y envio
            Convierte datos[i] de cuentas ADC a voltios → y[i] (float)
            Envia y[i] por UART a la PC (un valor por linea, formato ASCII)
                |
                v
Estado '0'  Ciclo reiniciado
```

### Parametros de adquisicion

| Parametro | Valor |
|-----------|-------|
| Muestras por adquisicion | 2000 (`#define LONGITUD 2000`) |
| Clock del sistema | 24 MHz (`clock_config.h`) |
| Tipo de ADC | SAR 12 bits |
| Protocolo UART | ASCII, un `float` por linea |
| Trigger | Byte `'T'` enviado desde MATLAB |

### Comunicacion serie (`er.c`)

`er.c` provee funciones de comunicacion serie con protocolo de integridad:

- `envBytesChecksum()` — Envia datos con encabezado de longitud (16 bits little-endian) y checksum por suma
- `recBytesChecksum()` — Recibe datos y verifica integridad; timeout ~2 s a 24 MHz
- `envBytes()` / `recBytes()` — Versiones sin checksum
- Timeout configurado en base a `FRECUENCIA_CLOCK_CONFIG` (24 MHz)

> Estas funciones estan disponibles en el proyecto pero no son utilizadas en el flujo principal de `main.c` actualmente.
