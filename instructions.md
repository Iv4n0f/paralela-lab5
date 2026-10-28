INSTRUCCIONES DE EJECUCIÓN PARA EL AGENTE DE IAROL DEL AGENTE:
Actúa como un asistente técnico de desarrollo e investigación para la asignatura de Computación Paralela y Distribuida. Tu tarea es guiar, estructurar, automatizar y documentar el desarrollo del "Laboratorio 5: Programación con Pthreads: Concurrencia y Sincronización" siguiendo las directivas del documento y la metodología técnica descrita a continuación. No resuelvas el laboratorio de golpe; debes procesar cada fase de manera ordenada según las pautas de este prompt.   ==================================================ENTORNO DE HARDWARE Y PARÁMETROS EXPERIMENTALES
==================================================Entorno de ejecución: Laptop local.Arquitectura de CPU: 8 núcleos físicos, 16 núcleos lógicos mediante Hyper-Threading (SMT).Vector de hilos a evaluar en todas las pruebas de escalabilidad:
threads = [1, 2, 4, 8, 12, 16, 24]De 1 a 8 hilos: análisis de paralelismo puro sobre núcleos físicos.   De 8 a 16 hilos: análisis de impacto de SMT / Hyper-Threading.De 16 a 32 hilos: análisis de sobrecarga por cambio de contexto y contención.   Repeticiones: Cada ejecución debe correrse un mínimo de 3 a 5 veces para promediar los tiempos y descartar variaciones de frecuencia dinámica.==================================================
2. ESTRUCTURA DEL PROYECTOOrganiza el proyecto estrictamente en la siguiente estructura de carpetas:proyecto_lab5/
├── parte1_pi/
│   ├── pi_secuencial.c
│   ├── pi_pthreads.c
│   └── Makefile
├── parte2_matriz_vector/
│   ├── matvec_secuencial.c
│   ├── matvec_pthreads.c
│   └── Makefile
├── parte3_lista_enlazada/
│   ├── lista_sin_sync.c
│   ├── lista_mutex.c
│   ├── lista_rwlock.c
│   └── Makefile
├── parte4_barreras/
│   ├── barrera_busywait.c
│   ├── barrera_semaforos.c
│   ├── barrera_condvar.c
│   └── Makefile
├── data/
│   └── (almacén de archivos .csv generados por los ejecutables)
├── scripts/
│   ├── benchmark.sh        (script de automatización de corridas)
│   └── graficar.py         (procesamiento con pandas/matplotlib)
└── informe/
├── img/                (destino de las figuras generadas)
├── assets/
│   └── cs_logo.png     (logotipo institucional)
├── lib.typ             (definiciones de la plantilla)
└── main.typ            (informe técnico redactado en Typst), puedes modificar esta estructura para facilitar algunas cosas, de ser necesesario. pero se que se entienda la separacion por subactividades==================================================
3. PROTOCOLO DE MEDICIÓN Y GENERACIÓN DE CSVNo utilizar la función clock() de C (mide tiempo de CPU agregado, no tiempo de reloj de pared).Medir exclusivamente la sección concurrente/paralela usando:
clock_gettime(CLOCK_MONOTONIC, &inicio);
...
clock_gettime(CLOCK_MONOTONIC, &fin);Metodología de salida hacia CSV desde los archivos .c:El código en C debe recibir los argumentos por línea de comandos (ej. número de hilos, tamaño del problema).   Debe imprimir únicamente la tupla de resultados por stdout en formato delimitado por comas:
printf("%d,%ld,%.9f\n", num_threads, n_datos, tiempo_segundos);El script scripts/benchmark.sh debe inicializar el archivo en data/.csv con su cabecera correspondiente (ej. "threads,n,time") y anexar las ejecuciones redirigiendo la salida estándar (>>).==================================================
4. ACTIVIDADES SOLICITADAS POR EL LABORATORIOPARTE I: CÁLCULO DE PI   Implementar una versión secuencial base.   Implementar una versión paralela en Pthreads distribuyendo las iteraciones en bloques continuos por hilo.   Evitar condiciones de carrera: cada hilo acumula en una variable local privada; la reducción a la variable compartida se realiza una única vez por hilo dentro de una sección crítica protegida por pthread_mutex.   Validar la precisión comparando el valor obtenido frente a un valor de referencia.   Variar tamaños de problema (N) y cantidad de hilos.   Calcular Speedup (S = T_1 / T_p) y Eficiencia (E = S / p).   Preguntas a responder en el informe:¿Cómo se distribuyó el trabajo?   ¿Qué variables son compartidas y cuáles privadas?   ¿Cómo se resolvió la sección crítica?   ¿Qué comportamiento exhibe el tiempo al aumentar hilos y por qué deja de mejorar el rendimiento?   PARTE II: MULTIPLICACIÓN MATRIZ-VECTOR   Implementar versión secuencial y versión Pthreads.   Estrategia: Descomposición unidimensional por filas (cada hilo calcula un subconjunto de filas del vector resultado y).   Verificar que la versión paralela produzca exactamente el mismo resultado que la secuencial.   Probar diferentes tamaños de matriz (N x N) y vector de hilos.   Preguntas a responder en el informe:¿Por qué las filas son una unidad de trabajo adecuada?   ¿Es necesario un mutex para escribir en el vector resultado? (Justificar la respuesta técnica demostrando accesos disjuntos).   ¿Cómo cambia el rendimiento al aumentar la dimensión de la matriz?   ¿Qué ocurre cuando los hilos superan a los núcleos disponibles (16 y 32 hilos)?   ¿El speedup crece linealmente? Explicar las razones.   PARTE III: LISTA ENLAZADA MULTITHREADING   Desarrollar una estructura de lista enlazada compartida con funciones: Member(val), Insert(val), Delete(val).   Carga de trabajo recomendada: Lista precargada con datos iniciales; ejecución de un volumen alto de operaciones concurrentes distribuidas (ej. 80% lectura/búsqueda, 10% inserción, 10% eliminación).Versiones obligatorias:lista_sin_sync.c: Sin mecanismos de sincronización para evidenciar condiciones de carrera, inconsistencias de memoria o fallos de segmentación.   lista_mutex.c: Sincronización gruesa mediante un único pthread_mutex_t para toda la lista.lista_rwlock.c: Candados de lectura/escritura (pthread_rwlock_t) permitiendo múltiples lectores simultáneos para Member y exclusión para Insert/Delete.Preguntas a responder en el informe:¿Qué condiciones de carrera ocurren y qué estructuras son compartidas?   ¿Qué secciones requieren protección?   ¿Qué limitaciones introduce un único mutex para toda la lista?   ¿Qué relación existe entre la granularidad del bloqueo y el paralelismo?   Comparación de rendimiento entre sin sincronización, mutex y rwlock.   PARTE IV: IMPLEMENTACIÓN DE BARRERAS   Implementar un escenario multifase donde todos los hilos deben completar la fase actual antes de pasar a la siguiente.   Desarrollar las tres variantes clásicas del Capítulo 4:   barrera_busywait.c: Mutex para conteo de hilos llegados + bucle de espera activa (busy-waiting) evaluando la bandera de liberación.   barrera_semaforos.c: Implementación con semáforos POSIX (sem_t).   barrera_condvar.c: Implementación con variable de condición (pthread_cond_t) y pthread_mutex_t.   Verificar que ningún hilo avance de fase prematuramente.   Preguntas a responder en el informe:¿Qué mecanismo usa cada implementación?   ¿Cuál es la diferencia conceptual entre las tres?   Ventajas, desventajas y costo de sincronización conforme escala la cantidad de hilos.   ==================================================
5. GENERACIÓN DE GRÁFICAS Y CARPETA DE IMÁGENESEl script scripts/graficar.py debe leer los archivos en data/*.csv.Calcular automáticamente el promedio por cantidad de hilos, el Speedup relativo a 1 hilo y la Eficiencia.Generar curvas de:Tiempo de ejecución vs. Número de hilos.Speedup vs. Número de hilos (incluyendo línea de Speedup ideal y marcas verticales en 8 y 16 hilos).Eficiencia vs. Número de hilos.Guardar automáticamente todas las figuras generadas en formato PNG de alta resolución o SVG dentro de la carpeta:
informe/img/==================================================
6. REDACCIÓN Y COMPILACIÓN DEL INFORME EN TYPSTTodo el informe final se redacta en informe/main.typ y se compila utilizando el CLI local de Typst:
typst compile informe/main.typ informe/laboratorio5.pdfLa cabecera del informe debe basarse estrictamente en la plantilla del usuario, adaptando el título a la práctica correspondiente:Fragmento de código#import "lib.typ": proyecto, footnote_ref

#show: proyecto.with(
  title: "LABORATORIO 5: PROGRAMACIÓN CON PTHREADS: CONCURRENCIA Y SINCRONIZACIÓN",
  course: "COMPUTACION PARALELA Y DISTRIBUIDA",
  logo: "assets/cs_logo.png",
  teacher: "ALVARO HENRY MAMANI ALIAGA",
  members: (
    "Lopez Zegarra, Ivan Alexander",
  ),
)
Estructura obligatoria de secciones del informe:   Introducción   Entorno de ejecución y características del equipo (Documentar los 8 núcleos físicos y 16 con SMT)   Cálculo de π   Multiplicación matriz-vector   Lista enlazada multithreading   Implementaciones de barreras   Resultados y análisis (incluir figuras desde img/ y tablas de datos)   Problemas encontrados y soluciones   Conclusiones   ==================================================
