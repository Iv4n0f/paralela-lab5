#!/usr/bin/env python3
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'
plt.rcParams['axes.edgecolor'] = '#cccccc'
plt.rcParams['axes.linewidth'] = 0.8

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA_DIR = os.path.join(BASE_DIR, "data")
IMG_DIR = os.path.join(BASE_DIR, "informe", "img")
os.makedirs(IMG_DIR, exist_ok=True)

THREADS_TICKS = [1, 2, 4, 8, 12, 16, 24]

def add_hardware_lines(ax):
    ax.axvline(x=8, color='#d9534f', linestyle='--', alpha=0.75, linewidth=1.5, label='8 Cores Físicos')
    ax.axvline(x=16, color='#f0ad4e', linestyle=':', alpha=0.85, linewidth=1.5, label='16 Hilos SMT')

# ==============================================================================
# PARTE I: CALCULO DE PI
# ==============================================================================
def graficar_parte1():
    csv_file = os.path.join(DATA_DIR, "pi_pthreads.csv")
    if not os.path.exists(csv_file):
        print(f"No se encontro {csv_file}")
        return

    df = pd.read_csv(csv_file)
    df = df[df['time'] > 0]
    grouped = df.groupby(['n', 'threads'])['time'].agg(['mean', 'std']).reset_index()
    sizes = sorted(df['n'].unique())

    # 1. Tiempo de Ejecución vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n]
        ax.plot(sub['threads'], sub['mean'], marker='o', linewidth=2, label=f'N = {n:,.0f}')
    add_hardware_lines(ax)
    ax.set_title("Cálculo de $\\pi$: Tiempo de Ejecución vs. Número de Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Tiempo de Pared (segundos)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte1_pi_tiempo.png"))
    plt.close()

    # 2. Speedup vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n].copy().sort_values('threads')
        t1_rows = sub[sub['threads'] == 1]['mean'].values
        if len(t1_rows) > 0:
            t1 = t1_rows[0]
            sub['speedup'] = t1 / sub['mean']
            ax.plot(sub['threads'], sub['speedup'], marker='s', linewidth=2, label=f'Speedup N = {n:,.0f}')
    
    threads = sorted(grouped['threads'].unique())
    ax.plot(threads, threads, color='black', linestyle='-.', linewidth=1.5, label='Speedup Ideal ($S=p$)')
    add_hardware_lines(ax)
    ax.set_title("Cálculo de $\\pi$: Speedup vs. Número de Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Speedup ($S = T_1 / T_p$)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte1_pi_speedup.png"))
    plt.close()

    # 3. Eficiencia vs Hilos
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n].copy().sort_values('threads')
        t1_rows = sub[sub['threads'] == 1]['mean'].values
        if len(t1_rows) > 0:
            t1 = t1_rows[0]
            sub['efficiency'] = (t1 / sub['mean']) / sub['threads']
            ax.plot(sub['threads'], sub['efficiency'], marker='^', linewidth=2, label=f'Eficiencia N = {n:,.0f}')
    
    ax.axhline(y=1.0, color='black', linestyle='-.', linewidth=1.5, label='Eficiencia Ideal ($E=1.0$)')
    add_hardware_lines(ax)
    ax.set_title("Cálculo de $\\pi$: Eficiencia vs. Número de Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Eficiencia ($E = S / p$)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.set_ylim(0, 1.15)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte1_pi_eficiencia.png"))
    plt.close()
    print("Graficas de Parte I generadas.")

# ==============================================================================
# PARTE II: MULTIPLICACION MATRIZ-VECTOR
# ==============================================================================
def graficar_parte2():
    csv_file = os.path.join(DATA_DIR, "matvec_pthreads.csv")
    if not os.path.exists(csv_file):
        print(f"No se encontro {csv_file}")
        return

    df = pd.read_csv(csv_file)
    df = df[df['time'] > 0]
    grouped = df.groupby(['n', 'threads'])['time'].agg(['mean', 'std']).reset_index()
    sizes = sorted(df['n'].unique())

    # 1. Tiempo
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n]
        ax.plot(sub['threads'], sub['mean'], marker='o', linewidth=2, label=f'Matriz {n}x{n}')
    add_hardware_lines(ax)
    ax.set_title("Multiplicación Matriz-Vector: Tiempo vs. Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Tiempo de Pared (segundos)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte2_matvec_tiempo.png"))
    plt.close()

    # 2. Speedup
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n].copy().sort_values('threads')
        t1_rows = sub[sub['threads'] == 1]['mean'].values
        if len(t1_rows) > 0:
            t1 = t1_rows[0]
            sub['speedup'] = t1 / sub['mean']
            ax.plot(sub['threads'], sub['speedup'], marker='s', linewidth=2, label=f'Speedup {n}x{n}')
    threads = sorted(grouped['threads'].unique())
    ax.plot(threads, threads, color='black', linestyle='-.', linewidth=1.5, label='Speedup Ideal')
    add_hardware_lines(ax)
    ax.set_title("Multiplicación Matriz-Vector: Speedup vs. Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Speedup ($S = T_1 / T_p$)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte2_matvec_speedup.png"))
    plt.close()

    # 3. Eficiencia
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    for n in sizes:
        sub = grouped[grouped['n'] == n].copy().sort_values('threads')
        t1_rows = sub[sub['threads'] == 1]['mean'].values
        if len(t1_rows) > 0:
            t1 = t1_rows[0]
            sub['efficiency'] = (t1 / sub['mean']) / sub['threads']
            ax.plot(sub['threads'], sub['efficiency'], marker='^', linewidth=2, label=f'Eficiencia {n}x{n}')
    ax.axhline(y=1.0, color='black', linestyle='-.', linewidth=1.5, label='Eficiencia Ideal')
    add_hardware_lines(ax)
    ax.set_title("Multiplicación Matriz-Vector: Eficiencia vs. Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Eficiencia ($E = S / p$)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.set_ylim(0, 1.15)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte2_matvec_eficiencia.png"))
    plt.close()
    print("Graficas de Parte II generadas.")

# ==============================================================================
# PARTE III: LISTA ENLAZADA
# ==============================================================================
def graficar_parte3():
    csv_mutex = os.path.join(DATA_DIR, "lista_mutex.csv")
    csv_rwlock = os.path.join(DATA_DIR, "lista_rwlock.csv")
    csv_nosync = os.path.join(DATA_DIR, "lista_sin_sync.csv")

    if not (os.path.exists(csv_mutex) and os.path.exists(csv_rwlock)):
        print("Archivos de listas enlazadas faltantes.")
        return

    df_mutex = pd.read_csv(csv_mutex)
    df_rwlock = pd.read_csv(csv_rwlock)
    df_mutex = df_mutex[df_mutex['time'] > 0]
    df_rwlock = df_rwlock[df_rwlock['time'] > 0]

    grp_m = df_mutex.groupby('threads')['time'].mean().reset_index()
    grp_rw = df_rwlock.groupby('threads')['time'].mean().reset_index()

    # 1. Comparativa de Tiempo Mutex vs RWLock
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    ax.plot(grp_m['threads'], grp_m['time'], marker='o', color='#d9534f', linewidth=2, label='Mutex Único (Coarse-Grained)')
    ax.plot(grp_rw['threads'], grp_rw['time'], marker='s', color='#0275d8', linewidth=2, label='RWLock (80% Readers)')

    if os.path.exists(csv_nosync):
        df_ns = pd.read_csv(csv_nosync)
        df_ns = df_ns[df_ns['time'] > 0]
        if not df_ns.empty:
            grp_ns = df_ns.groupby('threads')['time'].mean().reset_index()
            # Escalar tiempo a 50k ops proporcionalmente si ops fueron 10k para referencia
            ops_ns = df_ns['n'].iloc[0] if 'n' in df_ns.columns else 10000
            scale_factor = 50000.0 / ops_ns
            ax.plot(grp_ns['threads'], grp_ns['time'] * scale_factor, marker='^', color='#5cb85c', linestyle=':', linewidth=2, label=f'Sin Sync (Escalado {int(ops_ns)}k->50k ops, Inseguro)')

    add_hardware_lines(ax)
    ax.set_title("Lista Enlazada Concurrente: Tiempo de Ejecución (50k ops)", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Tiempo de Pared (segundos)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte3_lista_comparativa.png"))
    plt.close()

    # 2. Speedup Mutex vs RWLock
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    t1_m = grp_m[grp_m['threads'] == 1]['time'].values[0]
    t1_rw = grp_rw[grp_rw['threads'] == 1]['time'].values[0]
    ax.plot(grp_m['threads'], t1_m / grp_m['time'], marker='o', color='#d9534f', linewidth=2, label='Speedup Mutex')
    ax.plot(grp_rw['threads'], t1_rw / grp_rw['time'], marker='s', color='#0275d8', linewidth=2, label='Speedup RWLock')
    threads = sorted(grp_m['threads'].unique())
    ax.plot(threads, threads, color='black', linestyle='-.', linewidth=1.5, label='Speedup Ideal')
    add_hardware_lines(ax)
    ax.set_title("Lista Enlazada Concurrente: Speedup vs. Hilos", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Speedup ($S = T_1 / T_p$)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte3_lista_speedup.png"))
    plt.close()
    print("Graficas de Parte III generadas.")

# ==============================================================================
# PARTE IV: BARRERAS
# ==============================================================================
def graficar_parte4():
    f_busy = os.path.join(DATA_DIR, "barrera_busywait.csv")
    f_sem = os.path.join(DATA_DIR, "barrera_semaforos.csv")
    f_cond = os.path.join(DATA_DIR, "barrera_condvar.csv")

    if not (os.path.exists(f_busy) and os.path.exists(f_sem) and os.path.exists(f_cond)):
        print("Archivos de barreras faltantes.")
        return

    df_b = pd.read_csv(f_busy)
    df_s = pd.read_csv(f_sem)
    df_c = pd.read_csv(f_cond)

    df_busy = df_b[df_b['time'] > 0].groupby('threads')['time'].mean().reset_index()
    df_sem = df_s[df_s['time'] > 0].groupby('threads')['time'].mean().reset_index()
    df_cond = df_c[df_c['time'] > 0].groupby('threads')['time'].mean().reset_index()

    phases = df_b['n'].iloc[0] if 'n' in df_b.columns and not df_b.empty else 1000

    # 1. Escala Logarítmica
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    ax.plot(df_busy['threads'], df_busy['time'], marker='o', color='#d9534f', linewidth=2, label='Barrera Busy-Waiting')
    ax.plot(df_sem['threads'], df_sem['time'], marker='s', color='#5cb85c', linewidth=2, label='Barrera Semáforos POSIX')
    ax.plot(df_cond['threads'], df_cond['time'], marker='^', color='#0275d8', linewidth=2, label='Barrera CondVar (Pthreads)')

    add_hardware_lines(ax)
    ax.set_title(f"Comparativa de Barreras: Tiempo vs. Número de Hilos ({phases} fases)", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Tiempo de Pared (segundos, escala logarítmica)", fontsize=11)
    ax.set_yscale('log')
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6, which="both")
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte4_barreras_tiempo.png"))
    plt.close()

    # 2. Escala Lineal (detalle de saturación)
    fig, ax = plt.subplots(figsize=(8, 5), dpi=300)
    ax.plot(df_busy['threads'], df_busy['time'], marker='o', color='#d9534f', linewidth=2, label='Busy-Waiting')
    ax.plot(df_sem['threads'], df_sem['time'], marker='s', color='#5cb85c', linewidth=2, label='Semáforos')
    ax.plot(df_cond['threads'], df_cond['time'], marker='^', color='#0275d8', linewidth=2, label='CondVar')

    add_hardware_lines(ax)
    ax.set_title(f"Barreras en Escala Lineal: Sobrecarga por Contención ({phases} fases)", fontsize=12, fontweight='bold')
    ax.set_xlabel("Número de Hilos ($p$)", fontsize=11)
    ax.set_ylabel("Tiempo de Pared (segundos)", fontsize=11)
    ax.set_xticks(THREADS_TICKS)
    ax.legend(frameon=True)
    ax.grid(True, linestyle='--', alpha=0.6)
    plt.tight_layout()
    plt.savefig(os.path.join(IMG_DIR, "parte4_barreras_lineal.png"))
    plt.close()

    print("Graficas de Parte IV generadas.")

if __name__ == '__main__':
    graficar_parte1()
    graficar_parte2()
    graficar_parte3()
    graficar_parte4()
    print("Todas las graficas fueron guardadas exitosamente en informe/img/")
