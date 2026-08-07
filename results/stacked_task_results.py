#!/usr/bin/env python3
"""
stacked_task_results.py

Lê arquivos de resultados do ns-3/OLVANETS no formato .tr, calcula a
porcentagem média de ocorrências de:

    TL = Task Local
    TS = Task Success
    TR = Task Recovery

para cada cenário e gera um gráfico de barras empilhadas.

O cálculo é realizado primeiro para cada execução (run):

    TL_run = tasksOnlyLocal / tasksCounted
    TS_run = tasksOffloadedSuc / tasksCounted
    TR_run = tasksRecovered / tasksCounted

Em seguida, é calculada a média dos percentuais das execuções.

Essa abordagem atribui o mesmo peso a cada run, independentemente da
quantidade de tarefas contadas em cada execução. Somar todas as tarefas
antes de calcular os percentuais daria maior peso aos runs com mais
tarefas, o que pode distorcer a comparação entre execuções.
"""

from pathlib import Path
from statistics import mean

import matplotlib.pyplot as plt


# ---------------------------------------------------------------------------
# CONFIGURAÇÃO DOS ARQUIVOS
# ---------------------------------------------------------------------------
# Substitua os caminhos abaixo pelos nomes reais dos seus arquivos .tr.
#
# Os rótulos do dicionário também serão usados:
#   1. na tabela impressa no terminal;
#   2. nos rótulos do eixo X do gráfico.
#
# Exemplo:
# Path("/home/usuario/resultados/cenario_10_90_workload1.tr")
#
ARQUIVOS = {
    "10/90 W1": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload1-w1-gtt.tr"),
    "10/90 W2": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload2-w2-gtt.tr"),
    "10/90 W4": Path("urban-medium-100-urban-medium-10moto90car-no-infra-workload4-w4-gtt.tr"),
    "90/10 W1": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload1-w1-gtt.tr"),
    "90/10 W2": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload2-w2-gtt.tr"),
    "90/10 W4": Path("urban-medium-100-urban-medium-90moto10car-no-infra-workload4-w4-gtt.tr"),
}

ARQUIVO_SAIDA = Path("stacked_task_occurrences.png")

# Prefixos de linhas consideradas relevantes.
PREFIXOS_VALIDOS = ("N;", "T;")

# Índices Python das colunas informadas.
#
# A numeração fornecida no experimento considera:
#   coluna 8  -> tasksOnlyLocal
#   coluna 9  -> tasksOffloadedSuc
#   coluna 10 -> tasksRecovered
#   coluna 11 -> tasksCounted
#   coluna 14 -> run
#
# Como listas Python começam no índice zero, os índices usados são:
COL_TASKS_ONLY_LOCAL = 8
COL_TASKS_OFFLOADED_SUCCESS = 9
COL_TASKS_RECOVERED = 10
COL_TASKS_COUNTED = 11
COL_RUN = 14

# Número esperado de execuções por cenário.
RUNS_ESPERADOS = 30

# Tolerância, em pontos percentuais, para a soma TL + TS + TR.
TOLERANCIA_SOMA = 0.5


def ler_resultados_por_run(caminho):
    """
    Lê um arquivo .tr e retorna os dados finais encontrados para cada run.

    Retorno:
        {
            run: {
                "tl": int,
                "ts": int,
                "tr": int,
                "total": int
            }
        }

    Caso existam várias linhas N; ou T; para o mesmo run, a última linha
    válida é mantida. Isso é adequado quando os campos são contadores
    acumulados e a última linha representa o estado final da execução.
    """
    resultados = {}

    if not caminho.exists():
        raise FileNotFoundError(f"Arquivo não encontrado: {caminho}")

    with caminho.open("r", encoding="utf-8", errors="replace") as arquivo:
        for numero_linha, linha in enumerate(arquivo, start=1):
            linha = linha.strip()

            if not linha.startswith(PREFIXOS_VALIDOS):
                continue

            colunas = linha.split(";")

            if len(colunas) <= COL_RUN:
                print(
                    f"Aviso: linha {numero_linha} ignorada em "
                    f"'{caminho.name}': quantidade insuficiente de colunas."
                )
                continue

            try:
                tasks_only_local = int(colunas[COL_TASKS_ONLY_LOCAL])
                tasks_offloaded_success = int(
                    colunas[COL_TASKS_OFFLOADED_SUCCESS]
                )
                tasks_recovered = int(colunas[COL_TASKS_RECOVERED])
                tasks_counted = int(colunas[COL_TASKS_COUNTED])
                run = int(colunas[COL_RUN])
            except ValueError:
                print(
                    f"Aviso: linha {numero_linha} ignorada em "
                    f"'{caminho.name}': valor numérico inválido."
                )
                continue

            if tasks_counted <= 0:
                print(
                    f"Aviso: linha {numero_linha} ignorada em "
                    f"'{caminho.name}': tasksCounted = {tasks_counted}."
                )
                continue

            resultados[run] = {
                "tl": tasks_only_local,
                "ts": tasks_offloaded_success,
                "tr": tasks_recovered,
                "total": tasks_counted,
            }

    if not resultados:
        raise ValueError(
            f"Nenhuma linha válida iniciada por N; ou T; foi encontrada em "
            f"'{caminho}'."
        )

    return resultados


def calcular_medias_percentuais(resultados_por_run):
    """
    Calcula TL, TS e TR para cada run e retorna a média percentual dos runs.

    Cada execução tem o mesmo peso na média:

        média TL = média(TL_run_1, TL_run_2, ..., TL_run_n)

    Isso difere de:

        soma(tasksOnlyLocal) / soma(tasksCounted)

    A segunda forma ponderaria implicitamente os runs pela quantidade de
    tarefas. O cálculo usado aqui é o solicitado para comparar as execuções
    de forma equilibrada.
    """
    percentuais_tl = []
    percentuais_ts = []
    percentuais_tr = []

    for run in sorted(resultados_por_run):
        dados = resultados_por_run[run]
        total = dados["total"]

        tl = 100.0 * dados["tl"] / total
        ts = 100.0 * dados["ts"] / total
        tr = 100.0 * dados["tr"] / total

        percentuais_tl.append(tl)
        percentuais_ts.append(ts)
        percentuais_tr.append(tr)

    return {
        "tl": mean(percentuais_tl),
        "ts": mean(percentuais_ts),
        "tr": mean(percentuais_tr),
        "quantidade_runs": len(resultados_por_run),
        "runs": sorted(resultados_por_run),
    }


def imprimir_tabela(resultados_cenarios):
    """Imprime a tabela de percentuais médios no terminal."""
    print()
    print(
        f"{'Scenario':<16}"
        f"{'TL(%)':>10}"
        f"{'TS(%)':>10}"
        f"{'TR(%)':>10}"
        f"{'Sum(%)':>10}"
        f"{'Runs':>8}"
        f"{'Check':>10}"
    )
    print("-" * 74)

    for cenario, dados in resultados_cenarios.items():
        soma = dados["tl"] + dados["ts"] + dados["tr"]
        valido = abs(soma - 100.0) <= TOLERANCIA_SOMA
        status = "OK" if valido else "WARNING"

        print(
            f"{cenario:<16}"
            f"{dados['tl']:>10.2f}"
            f"{dados['ts']:>10.2f}"
            f"{dados['tr']:>10.2f}"
            f"{soma:>10.2f}"
            f"{dados['quantidade_runs']:>8}"
            f"{status:>10}"
        )

    print()

    for cenario, dados in resultados_cenarios.items():
        quantidade = dados["quantidade_runs"]

        if quantidade != RUNS_ESPERADOS:
            print(
                f"Aviso: '{cenario}' possui {quantidade} runs válidos; "
                f"eram esperados {RUNS_ESPERADOS}."
            )


def adicionar_rotulos_segmentos(ax, barras, valores):
    """
    Adiciona os percentuais no centro dos segmentos com espaço suficiente.

    Segmentos muito pequenos não recebem texto para evitar sobreposição.
    """
    limite_minimo = 4.0

    for barra, valor in zip(barras, valores):
        if valor < limite_minimo:
            continue

        centro_x = barra.get_x() + barra.get_width() / 2
        centro_y = barra.get_y() + barra.get_height() / 2

        ax.text(
            centro_x,
            centro_y,
            f"{valor:.1f}%",
            ha="center",
            va="center",
            fontsize=9,
        )


def gerar_grafico(resultados_cenarios, caminho_saida):
    """Gera e salva o gráfico de barras empilhadas."""
    cenarios = list(resultados_cenarios.keys())
    valores_tl = [resultados_cenarios[c]["tl"] for c in cenarios]
    valores_ts = [resultados_cenarios[c]["ts"] for c in cenarios]
    valores_tr = [resultados_cenarios[c]["tr"] for c in cenarios]

    posicoes = list(range(len(cenarios)))

    fig, ax = plt.subplots(figsize=(10, 6))

    barras_tl = ax.bar(
        posicoes,
        valores_tl,
        label="TL",
        edgecolor="black",
        linewidth=0.6,
    )

    barras_ts = ax.bar(
        posicoes,
        valores_ts,
        bottom=valores_tl,
        label="TS",
        edgecolor="black",
        linewidth=0.6,
    )

    base_tr = [
        tl + ts
        for tl, ts in zip(valores_tl, valores_ts)
    ]

    barras_tr = ax.bar(
        posicoes,
        valores_tr,
        bottom=base_tr,
        label="TR",
        edgecolor="black",
        linewidth=0.6,
    )

    adicionar_rotulos_segmentos(ax, barras_tl, valores_tl)
    adicionar_rotulos_segmentos(ax, barras_ts, valores_ts)
    adicionar_rotulos_segmentos(ax, barras_tr, valores_tr)

    ax.set_ylabel("Percentage of task occurrences (%)")
    ax.set_xlabel("Scenario / Workload")
    ax.set_xticks(posicoes)
    ax.set_xticklabels(cenarios)
    ax.set_ylim(0, 100)
    ax.legend(title="Execution type")
    ax.grid(axis="y", linestyle="--", linewidth=0.5, alpha=0.5)

    fig.tight_layout()
    fig.savefig(caminho_saida, dpi=300, bbox_inches="tight")
    plt.close(fig)

    print(f"Gráfico salvo em: {caminho_saida.resolve()}")


def main():
    """Executa a leitura, análise, impressão e geração do gráfico."""
    resultados_cenarios = {}

    for cenario, caminho in ARQUIVOS.items():
        resultados_por_run = ler_resultados_por_run(caminho)
        medias = calcular_medias_percentuais(resultados_por_run)
        resultados_cenarios[cenario] = medias

    imprimir_tabela(resultados_cenarios)
    gerar_grafico(resultados_cenarios, ARQUIVO_SAIDA)


if __name__ == "__main__":
    main()
