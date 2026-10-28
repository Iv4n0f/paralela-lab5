# Laboratorio 5: Programación con Pthreads: Concurrencia y Sincronización

Este repositorio contiene las implementaciones, análisis de rendimiento y automatización experimental correspondientes al **Laboratorio 5 de Computación Paralela y Distribuida** (Escuela Profesional de Ciencia de la Computación, Universidad Nacional de San Agustín de Arequipa).

## Estructura del Proyecto

```text
paralela-lab5/
├── parte1_pi/               # Cálculo de Pi mediante series de Leibniz (Secuencial y Pthreads)
│   ├── pi_secuencial.c
│   ├── pi_pthreads.c
│   └── Makefile
├── parte2_matriz_vector/    # Multiplicación Matriz-Vector con partición por filas
│   ├── matvec_secuencial.c
│   ├── matvec_pthreads.c
│   └── Makefile
├── parte3_lista_enlazada/   # Lista enlazada concurrente (Sin Sync, Mutex y RWLock)
│   ├── lista_sin_sync.c
│   ├── lista_mutex.c
│   ├── lista_rwlock.c
│   └── Makefile
├── parte4_barreras/         # Barreras multifase (Busy-Waiting, Semáforos y CondVar)
│   ├── barrera_busywait.c
│   ├── barrera_semaforos.c
│   ├── barrera_condvar.c
│   └── Makefile
├── data/                    # Archivos CSV generados por las corridas de benchmark
├── scripts/
│   ├── benchmark.sh         # Automatización de benchmarks con timeout y aislamiento de fallos
│   └── graficar.py          # Generador de curvas (Tiempo, Speedup, Eficiencia)
├── informe/
│   ├── assets/              # Logotipo institucional
│   ├── img/                 # Gráficas generadas en alta resolución
│   ├── lib.typ              # Plantilla Typst
│   ├── main.typ             # Código fuente del informe técnico
│   └── laboratorio5.pdf     # Documento final compilado (17 páginas)
├── instructions.md          # Directivas del laboratorio
└── README.md
```

## Requisitos

- **Compilador:** GCC con soporte C99/POSIX (`-pthread -O3`).
- **Python 3:** Bibliotecas `pandas`, `matplotlib` y `numpy` para el procesamiento de datos.
- **Typst:** CLI de Typst (versión 0.15+) para compilar el informe técnico.
- **Entorno de ejecución:** Linux (probado en AMD Ryzen 7 5700U de 8 núcleos físicos y 16 hilos SMT).

## Compilación y Ejecución

### 1. Compilación manual de módulos C
Cada subdirectorio incluye un `Makefile` independiente:

```bash
make -C parte1_pi
make -C parte2_matriz_vector
make -C parte3_lista_enlazada
make -C parte4_barreras
```

Para limpiar los ejecutables:
```bash
make -C parte1_pi clean
make -C parte2_matriz_vector clean
make -C parte3_lista_enlazada clean
make -C parte4_barreras clean
```

### 2. Suite de Benchmarks Automatizada
El script `scripts/benchmark.sh` recrea todos los archivos en `data/*.csv`, compila los programas con optimización `-O3`, ejecuta 4 repeticiones independientes para cada vector de hilos ($p in [1, 2, 4, 8, 12, 16, 24]$) con control de `timeout` por prueba, y genera las gráficas en `informe/img/`:

```bash
./scripts/benchmark.sh
```

*Tiempo total estimado de ejecución:* Menos de 3 minutos.

### 3. Generación de Gráficas
Para procesar los datos de `data/` y actualizar las curvas en `informe/img/`:

```bash
python3 scripts/graficar.py
```

### 4. Compilación del Informe en Typst
El informe técnico incluye el marco conceptual, justificaciones matemáticas, tablas empíricas y respuestas a todas las preguntas teóricas:

```bash
typst compile informe/main.typ informe/laboratorio5.pdf
```

## Autor

- **Ivan Alexander Lopez Zegarra** - *Universidad Nacional de San Agustín de Arequipa*
