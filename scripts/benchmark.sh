#!/bin/bash
set -e

# Directorio raíz del proyecto
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "=========================================================="
echo "Iniciando suite de benchmarks automatizados - Lab 5"
echo "Hardware detectado: AMD Ryzen 7 5700U (8C / 16T)"
echo "Límite de hilos: 24 (Evaluación de 1, 2, 4, 8, 12, 16, 24)"
echo "Tiempo máximo esperado de suite: < 5 minutos (límite: 30 min)"
echo "=========================================================="

mkdir -p "$ROOT_DIR/data" "$ROOT_DIR/informe/img"

# Crear todos los CSV desde cero
rm -f "$ROOT_DIR"/data/*.csv

# Compilar todos los componentes C
echo "[1/5] Compilando todos los modulos C con optimizacion -O3..."
make -C parte1_pi clean && make -C parte1_pi
make -C parte2_matriz_vector clean && make -C parte2_matriz_vector
make -C parte3_lista_enlazada clean && make -C parte3_lista_enlazada
make -C parte4_barreras clean && make -C parte4_barreras

# Vector de hilos acotado a máximo 24 hilos
THREADS=(1 2 4 8 12 16 24)
REPS=4
DEFAULT_TIMEOUT=15

# Función de ejecución robusta con límite de tiempo por prueba (timeout)
# Evita que un benchmark bloqueado o fallido detenga la suite completa.
run_bench() {
    local timeout_sec="$1"
    local csv_file="$2"
    local fallback_t="$3"
    local fallback_n="$4"
    shift 4
    local cmd=("$@")

    local output=""
    local exit_code=0

    set +e
    output=$(timeout "$timeout_sec" "${cmd[@]}" 2>/dev/null)
    exit_code=$?
    set -e

    if [ $exit_code -eq 0 ] && [ -n "$output" ]; then
        echo "$output" >> "$csv_file"
    elif [ $exit_code -eq 124 ]; then
        echo "    [TIMEOUT] Excedió límite de ${timeout_sec}s: ${cmd[*]}"
        echo "${fallback_t},${fallback_n},-1.000000000" >> "$csv_file"
    else
        echo "    [FALLO] Código de salida ${exit_code}: ${cmd[*]}"
        echo "${fallback_t},${fallback_n},-1.000000000" >> "$csv_file"
    fi
}

# ----------------------------------------------------
# PARTE I: CALCULO DE PI
# ----------------------------------------------------
echo "[2/5] Ejecutando benchmarks de Parte I (Calculo de Pi)..."
PI_CSV="$ROOT_DIR/data/pi_pthreads.csv"
PI_SEQ_CSV="$ROOT_DIR/data/pi_secuencial.csv"
echo "threads,n,time" > "$PI_CSV"
echo "threads,n,time" > "$PI_SEQ_CSV"

PI_SIZES=(100000000 500000000)

for n in "${PI_SIZES[@]}"; do
    echo "  -> Pi Secuencial n=$n"
    for r in $(seq 1 $REPS); do
        run_bench "$DEFAULT_TIMEOUT" "$PI_SEQ_CSV" 1 "$n" ./parte1_pi/pi_secuencial "$n"
    done

    for t in "${THREADS[@]}"; do
        echo "  -> Pi Pthreads threads=$t n=$n"
        for r in $(seq 1 $REPS); do
            run_bench "$DEFAULT_TIMEOUT" "$PI_CSV" "$t" "$n" ./parte1_pi/pi_pthreads "$t" "$n"
        done
    done
done

# ----------------------------------------------------
# PARTE II: MULTIPLICACION MATRIZ-VECTOR
# ----------------------------------------------------
echo "[3/5] Ejecutando benchmarks de Parte II (Matriz-Vector)..."
MATVEC_CSV="$ROOT_DIR/data/matvec_pthreads.csv"
MATVEC_SEQ_CSV="$ROOT_DIR/data/matvec_secuencial.csv"
echo "threads,n,time" > "$MATVEC_CSV"
echo "threads,n,time" > "$MATVEC_SEQ_CSV"

MATVEC_SIZES=(4000 8000 12000)

for n in "${MATVEC_SIZES[@]}"; do
    echo "  -> MatVec Secuencial dim=$n"
    for r in $(seq 1 $REPS); do
        run_bench "$DEFAULT_TIMEOUT" "$MATVEC_SEQ_CSV" 1 "$n" ./parte2_matriz_vector/matvec_secuencial "$n"
    done

    for t in "${THREADS[@]}"; do
        echo "  -> MatVec Pthreads threads=$t dim=$n"
        for r in $(seq 1 $REPS); do
            run_bench "$DEFAULT_TIMEOUT" "$MATVEC_CSV" "$t" "$n" ./parte2_matriz_vector/matvec_pthreads "$t" "$n"
        done
    done
done

# ----------------------------------------------------
# PARTE III: LISTA ENLAZADA MULTITHREADING
# ----------------------------------------------------
echo "[4/5] Ejecutando benchmarks de Parte III (Lista Enlazada)..."
LISTA_MUTEX_CSV="$ROOT_DIR/data/lista_mutex.csv"
LISTA_RWLOCK_CSV="$ROOT_DIR/data/lista_rwlock.csv"
LISTA_NOSYNC_CSV="$ROOT_DIR/data/lista_sin_sync.csv"
echo "threads,n,time" > "$LISTA_MUTEX_CSV"
echo "threads,n,time" > "$LISTA_RWLOCK_CSV"
echo "threads,n,time" > "$LISTA_NOSYNC_CSV"

LIST_OPS=50000
NOSYNC_OPS=10000

for t in "${THREADS[@]}"; do
    echo "  -> Lista Mutex threads=$t ops=$LIST_OPS"
    for r in $(seq 1 $REPS); do
        run_bench "$DEFAULT_TIMEOUT" "$LISTA_MUTEX_CSV" "$t" "$LIST_OPS" ./parte3_lista_enlazada/lista_mutex "$t" "$LIST_OPS"
    done

    echo "  -> Lista RWLock threads=$t ops=$LIST_OPS"
    for r in $(seq 1 $REPS); do
        run_bench "$DEFAULT_TIMEOUT" "$LISTA_RWLOCK_CSV" "$t" "$LIST_OPS" ./parte3_lista_enlazada/lista_rwlock "$t" "$LIST_OPS"
    done

    echo "  -> Lista Sin Sync threads=$t ops=$NOSYNC_OPS"
    for r in $(seq 1 $REPS); do
        run_bench "$DEFAULT_TIMEOUT" "$LISTA_NOSYNC_CSV" "$t" "$NOSYNC_OPS" ./parte3_lista_enlazada/lista_sin_sync "$t" "$NOSYNC_OPS"
    done
done

# ----------------------------------------------------
# PARTE IV: IMPLEMENTACION DE BARRERAS
# ----------------------------------------------------
echo "[5/5] Ejecutando benchmarks de Parte IV (Barreras)..."
BAR_BUSY_CSV="$ROOT_DIR/data/barrera_busywait.csv"
BAR_SEM_CSV="$ROOT_DIR/data/barrera_semaforos.csv"
BAR_COND_CSV="$ROOT_DIR/data/barrera_condvar.csv"
echo "threads,n,time" > "$BAR_BUSY_CSV"
echo "threads,n,time" > "$BAR_SEM_CSV"
echo "threads,n,time" > "$BAR_COND_CSV"

BARRIER_PHASES=1000

for t in "${THREADS[@]}"; do
    echo "  -> Barreras threads=$t phases=$BARRIER_PHASES"
    for r in $(seq 1 $REPS); do
        # 20s de timeout para busywait en alta sobrecarga
        run_bench 20 "$BAR_BUSY_CSV" "$t" "$BARRIER_PHASES" ./parte4_barreras/barrera_busywait "$t" "$BARRIER_PHASES"
        run_bench "$DEFAULT_TIMEOUT" "$BAR_SEM_CSV" "$t" "$BARRIER_PHASES" ./parte4_barreras/barrera_semaforos "$t" "$BARRIER_PHASES"
        run_bench "$DEFAULT_TIMEOUT" "$BAR_COND_CSV" "$t" "$BARRIER_PHASES" ./parte4_barreras/barrera_condvar "$t" "$BARRIER_PHASES"
    done
done

echo "=========================================================="
echo "Generando graficas comparativas..."
python3 "$ROOT_DIR/scripts/graficar.py"

echo "=========================================================="
echo "Todos los benchmarks concluidos exitosamente."
echo "Archivos generados en data/*.csv e imagenes en informe/img/"
echo "=========================================================="
