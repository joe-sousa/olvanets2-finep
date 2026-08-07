#!/usr/bin/env python3
"""
execution_time_ci95.py

Calcula o tempo médio de execução e o intervalo de confiança de 95%
para cada cenário.

A coluna 2 do arquivo .tr representa:

- localTime       -> quando não houve offloading (linhas N)
- tempoResultante -> quando houve offloading (linhas T)

Após linha.split(";"):

    índice 2  -> executionTime
    índice 14 -> run

Para cada run utiliza-se o último tempo registrado.

São calculados:

- média
- desvio padrão amostral
- erro padrão
- IC95%

Além da tabela no terminal, é gerado um gráfico de barras
com barras de erro (95% CI).
"""

from math import sqrt
from pathlib import Path
from statistics import mean, stdev

import matplotlib.pyplot as plt

ARQUIVOS = {
    "10/90 W1": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload1-w1-gtt.tr"),
    "10/90 W2": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload2-w2-gtt.tr"),
    "10/90 W4": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload4-w4-gtt.tr"),
    "90/10 W1": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload1-w1-gtt.tr"),
    "90/10 W2": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload2-w2-gtt.tr"),
    "90/10 W4": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload4-w4-gtt.tr"),
}

COL_EXECUTION_TIME = 2
COL_RUN = 14

RUNS_ESPERADOS = 30
T_STUDENT_95_GL29 = 2.045


def ler_tempos_por_run(arquivo: Path):

    if not arquivo.exists():
        raise FileNotFoundError(f"Arquivo não encontrado: {arquivo}")

    runs = {}

    with arquivo.open("r", encoding="utf-8", errors="replace") as f:

        for numero_linha, linha in enumerate(f, start=1):

            if not (linha.startswith("N;") or linha.startswith("T;")):
                continue

            colunas = linha.strip().split(";")

            if len(colunas) <= COL_RUN:
                print(
                    f"Aviso: linha {numero_linha} ignorada "
                    f"em {arquivo.name}"
                )
                continue

            try:
                execution_time = float(colunas[COL_EXECUTION_TIME])
                run = int(colunas[COL_RUN])

            except ValueError:
                continue

            # mantém o último tempo daquele run
            runs[run] = execution_time

    if not runs:
        raise ValueError(
            f"Nenhum tempo encontrado em {arquivo}"
        )

    return runs


def calcular_ic95(valores):

    n = len(valores)

    media = mean(valores)

    if n == 1:
        return media, 0.0, 0.0, 0.0

    desvio = stdev(valores)

    erro = desvio / sqrt(n)

    margem = T_STUDENT_95_GL29 * erro

    return media, desvio, erro, margem


def gerar_grafico(resultados):

    labels = [r["scenario"] for r in resultados]
    medias = [r["mean"] for r in resultados]
    ic95 = [r["ci"] for r in resultados]

    plt.figure(figsize=(10,6))

    barras = plt.bar(
        labels,
        medias,
        yerr=ic95,
        capsize=6,
        color="steelblue",
        edgecolor="black",
        ecolor="black",
        linewidth=1
    )

    plt.ylabel("Execution time (s)", fontsize=15)
    plt.xlabel("Scenario / Workload", fontsize=15)

    plt.grid(
        axis="y",
        linestyle="--",
        alpha=0.4
    )

    ymax = max(medias) + max(ic95) + 3

    plt.ylim(0, ymax)

    for barra, media in zip(barras, medias):

        plt.text(
            barra.get_x() + barra.get_width()/2,
            barra.get_height() + 0.35,
            f"{media:.2f}",
            ha="center",
            va="bottom",
            fontsize=11
        )

    plt.tight_layout()

    plt.savefig(
        "execution_time_ci95.png",
        dpi=300,
        bbox_inches="tight"
    )

    print("\nGráfico salvo como:")
    print("execution_time_ci95.png")

    plt.show()


def main():

    resultados = []

    print()

    print(
        f'{"Scenario":<14}'
        f'{"Runs":>6}'
        f'{"Mean (s)":>14}'
        f'{"StdDev":>14}'
        f'{"95% CI":>14}'
        f'{"Interval":>24}'
    )

    print("-"*86)

    for nome, arquivo in ARQUIVOS.items():

        runs = ler_tempos_por_run(arquivo)

        tempos = [runs[r] for r in sorted(runs)]

        media, desvio, erro, margem = calcular_ic95(tempos)

        inferior = media - margem
        superior = media + margem

        intervalo = f"[{inferior:.3f}, {superior:.3f}]"

        resultados.append(
            {
                "scenario": nome,
                "mean": media,
                "ci": margem
            }
        )

        print(
            f"{nome:<14}"
            f"{len(tempos):>6}"
            f"{media:>14.3f}"
            f"{desvio:>14.3f}"
            f"{margem:>14.3f}"
            f"{intervalo:>24}"
        )

        if len(tempos) != RUNS_ESPERADOS:
            print(
                f"  Aviso: esperados {RUNS_ESPERADOS} runs."
            )

    print()
    print("Formato para o artigo:")
    print("Mean ± 95% CI")

    gerar_grafico(resultados)


if __name__ == "__main__":
    main()