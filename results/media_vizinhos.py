#!/usr/bin/env python3
"""
neighbors_ci95.py

Calcula o número médio de vizinhos para cada cenário de mobilidade.

Como o workload (W1, W2 e W4) NÃO altera a mobilidade, o número de
vizinhos é o mesmo para os três workloads. Assim, utiliza-se apenas
um arquivo representativo (W1) de cada cenário.

A última coluna do arquivo .tr corresponde a:

    numberOfNeighbors

Para cada run utiliza-se o último valor registrado.

São calculados:

- média
- desvio padrão
- erro padrão
- IC95%

Também é gerado um gráfico de barras com IC95%.
"""

from math import sqrt
from pathlib import Path
from statistics import mean, stdev

import matplotlib.pyplot as plt


ARQUIVOS = {
    "10/90": Path(
        "urban-medium-100-urban-medium-10moto90car-no-infra-workload1-w1-gtt.tr"
    ),
    "90/10": Path(
        "urban-medium-100-urban-medium-90moto10car-no-infra-workload1-w1-gtt.tr"
    ),
}

COL_NEIGHBORS = 18
COL_RUN = 14

RUNS_ESPERADOS = 30
T_STUDENT_95_GL29 = 2.045


def ler_vizinhos_por_run(arquivo: Path):

    if not arquivo.exists():
        raise FileNotFoundError(f"Arquivo não encontrado: {arquivo}")

    runs = {}

    with arquivo.open("r", encoding="utf-8", errors="replace") as f:

        for numero_linha, linha in enumerate(f, start=1):

            if not (linha.startswith("N;") or linha.startswith("T;")):
                continue

            colunas = linha.strip().split(";")

            if len(colunas) <= COL_NEIGHBORS:
                continue

            try:
                vizinhos = int(colunas[COL_NEIGHBORS])
                run = int(colunas[COL_RUN])

            except ValueError:
                continue

            # mantém o último valor daquele run
            runs[run] = vizinhos

    if not runs:
        raise ValueError(
            f"Nenhum valor válido encontrado em {arquivo.name}"
        )

    return runs


def calcular_ic95(valores):

    n = len(valores)

    media = mean(valores)

    if n == 1:
        return media, 0.0, 0.0, 0.0

    dp = stdev(valores)

    erro = dp / sqrt(n)

    margem = T_STUDENT_95_GL29 * erro

    return media, dp, erro, margem


def gerar_grafico(resultados):

    labels = [r["scenario"] for r in resultados]
    medias = [r["mean"] for r in resultados]
    ic95 = [r["ci"] for r in resultados]

    plt.figure(figsize=(6,5))

    barras = plt.bar(
        labels,
        medias,
        yerr=ic95,
        capsize=6,
        color="darkorange",
        edgecolor="black",
        ecolor="black",
        linewidth=1
    )

    plt.ylabel("Average number of neighbors", fontsize=13)
    plt.xlabel("Mobility scenario", fontsize=13)

    plt.grid(axis="y", linestyle="--", alpha=0.35)

    plt.ylim(0, max(medias) + max(ic95) + 2)

    for barra, media in zip(barras, medias):

        plt.text(
            barra.get_x() + barra.get_width()/2,
            barra.get_height() + 0.2,
            f"{media:.2f}",
            ha="center",
            va="bottom",
            fontsize=11,
            fontweight="bold"
        )

    plt.tight_layout()

    plt.savefig(
        "average_neighbors_ci95.png",
        dpi=300,
        bbox_inches="tight"
    )

    print("\nGráfico salvo como average_neighbors_ci95.png")

    plt.show()


def main():

    resultados = []

    print()

    print(
        f'{"Scenario":<10}'
        f'{"Runs":>8}'
        f'{"Mean":>12}'
        f'{"StdDev":>12}'
        f'{"95% CI":>12}'
        f'{"Interval":>22}'
    )

    print("-" * 76)

    for nome, arquivo in ARQUIVOS.items():

        runs = ler_vizinhos_por_run(arquivo)

        valores = [runs[r] for r in sorted(runs)]

        media, dp, erro, margem = calcular_ic95(valores)

        inferior = media - margem
        superior = media + margem

        resultados.append(
            {
                "scenario": nome,
                "mean": media,
                "ci": margem
            }
        )

        print(
            f"{nome:<10}"
            f"{len(valores):>8}"
            f"{media:>12.2f}"
            f"{dp:>12.2f}"
            f"{margem:>12.2f}"
            f"{f'[{inferior:.2f}, {superior:.2f}]':>22}"
        )

        if len(valores) != RUNS_ESPERADOS:
            print(
                f"  Aviso: esperados {RUNS_ESPERADOS} runs."
            )

    print()
    print("Formato para o artigo:")
    print("Mean ± 95% CI")


    gerar_grafico(resultados)


if __name__ == "__main__":
    main()