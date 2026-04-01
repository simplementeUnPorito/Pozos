# Matlab

## interfaz_transductor.m — Interfaz de Adquisicion de Datos

Aplicacion grafica en MATLAB para adquirir, visualizar y guardar senales de transductores piezoelectricos conectados a traves de un microcontrolador PSoC (CY8CKIT-059) por puerto serie (UART).

### Requisitos

- MATLAB R2019b o superior (requiere API `serialport`)
- PSoC CY8CKIT-059 con firmware de `Transductor.cydsn` o `Comunicacion.cydsn` cargado

### Flujo de trabajo

1. Conectar el PSoC por USB y seleccionar el puerto COM correspondiente
2. Configurar la velocidad de baudios (por defecto: 115200)
3. Definir el pilote (nombre y *stickup*) mediante el gestor de pilotes
4. Ingresar la profundidad de medicion y la frecuencia de muestreo
5. Seleccionar la carpeta donde se guardaran los archivos `.mat`
6. Hacer clic en **TRIGGER** — la interfaz envia el byte `'T'` al PSoC, que adquiere 2000 muestras y las retorna como floats en ASCII (un valor por linea)
7. Visualizar la senal en el grafico principal e inspeccionar las estadisticas
8. Hacer clic en **GUARDAR** para almacenar el registro en el `.mat` del pilote correspondiente

### Paneles de la interfaz grafica

| Panel | Funcion |
|-------|---------|
| Conexion serie | Seleccion de puerto COM y baudrate; boton CONECTAR/DESCONECTAR |
| Metadata de medicion | Pilote, profundidad, stickup, Fs, cantidad de puntos, carpeta de datos |
| Control de adquisicion | Boton TRIGGER, boton GUARDAR, progreso de muestras recibidas |
| Grafico principal | Senal tiempo (ms) vs. voltaje (V); soporta superposicion de multiples senales |
| Estadisticas | Maximo, minimo, media, RMS y duracion por senal cargada |
| Zoom | Rango de tiempo y voltaje ajustable independientemente |
| Registros guardados | Arbol jerarquico: Pilote → Profundidad → Medicion |
| Acciones de registros | Cargar, descargar, limpiar y eliminar registros |

### Estructura de datos guardados

Cada pilote se guarda en un archivo `<nombre_pilote>.mat` dentro de la carpeta seleccionada:

```matlab
pilote_info.nombre       % Identificador del pilote
pilote_info.stickup      % Distancia de stickup (m)
pilote_info.fecha_reg    % Fecha de registro del pilote

pilote_datos(i).pilote       % ID del pilote
pilote_datos(i).profundidad  % Profundidad de medicion (m)
pilote_datos(i).stickup      % Stickup (m)
pilote_datos(i).senal        % Senal ADC en voltios (Nx1)
pilote_datos(i).tiempo       % Eje temporal (ms) (Nx1)
pilote_datos(i).fs           % Frecuencia de muestreo (Hz)
pilote_datos(i).fecha        % Timestamp de adquisicion (yyyy-MM-dd HH:mm:ss)
pilote_datos(i).n_muestras   % Cantidad de muestras
```

### Persistencia de sesion

Al reabrir la interfaz se restauran automaticamente: carpeta de datos, puerto, baudrate, profundidad, frecuencia de muestreo y rangos de zoom. Estos valores se almacenan en `config_interfaz.mat` (no versionado).

---

## Analisis/ — Procesamiento de Datos

> **[Placeholder — a completar]**
> Esta seccion sera desarrollada una vez que se definan los algoritmos de procesamiento.

Los scripts de analisis de datos se encontraran en la carpeta `Analisis/`. Ver [`Analisis/README.md`](Analisis/README.md) para mas informacion.
