from pathlib import Path
from collections import defaultdict
import statistics
import matplotlib.pyplot as plt
import numpy as np

# Diretório dos resultados
base = Path("results")

arquivos = {
    "10/90": "urban-medium-100-urban-medium-10moto90car-no-infra-moto-fixed-w1-gtt.tr",
    "90/10": "urban-medium-100-urban-medium-90moto10car-no-infra-moto-fixed-w1-gtt.tr",
}

# ----------------------------------------------------
# Lê os arquivos
# ----------------------------------------------------

dados = {}

for nome, arquivo in arquivos.items():

    # chave = nº de surrogates
    # valor = lista de reduções
    reducoes_por_surrogate = defaultdict(list)

    with open(base / arquivo) as f:

        for linha in f:

            if not linha.strip():
                continue

            campos = linha.strip().split(";")

            if campos[0] == "ExecutionResult":
                continue

            run = int(campos[14])

            # apenas as 30 execuções
            if run > 30:
                continue

            reduction = -float(campos[7])

            number_of_surrogates = int(campos[3])

            reducoes_por_surrogate[number_of_surrogates].append(reduction)

    dados[nome] = reducoes_por_surrogate

# ----------------------------------------------------
# Estatísticas
# ----------------------------------------------------

print("\n==============================")

for cenario in dados:

    print(f"\n{cenario}")

    for s in sorted(dados[cenario]):

        valores = dados[cenario][s]

        media = statistics.mean(valores)

        if len(valores) > 1:
            desvio = statistics.stdev(valores)
        else:
            desvio = 0

        print(
            f"{s} surrogate(s)"
            f" | n={len(valores):2d}"
            f" | média={media:.2f}%"
            f" | desvio={desvio:.2f}"
        )

print("\n==============================")

# ----------------------------------------------------
# Preparação do gráfico
# ----------------------------------------------------

todos_surrogates = sorted(
    set(
        list(dados["10/90"].keys()) +
        list(dados["90/10"].keys())
    )
)

media_1090 = []
media_9010 = []

n_1090 = []
n_9010 = []

desvio_1090 = []
desvio_9010 = []

'''
for s in todos_surrogates:

    if s in dados["10/90"]:
        media_1090.append(
            statistics.mean(dados["10/90"][s])
        )
    else:
        media_1090.append(0)

    if s in dados["90/10"]:
        media_9010.append(
            statistics.mean(dados["90/10"][s])
        )
    else:
        media_9010.append(0)

for s in todos_surrogates:

    if s in dados["10/90"]:
        valores = dados["10/90"][s]
        media_1090.append(statistics.mean(valores))
        n_1090.append(len(valores))
    else:
        media_1090.append(0)
        n_1090.append(0)

    if s in dados["90/10"]:
        valores = dados["90/10"][s]
        media_9010.append(statistics.mean(valores))
        n_9010.append(len(valores))
    else:
        media_9010.append(0)
        n_9010.append(0)
'''

for s in todos_surrogates:

    if s in dados["10/90"]:
        valores = dados["10/90"][s]
        media_1090.append(statistics.mean(valores))
        desvio_1090.append(
            statistics.stdev(valores) if len(valores) > 1 else 0
        )
        n_1090.append(len(valores))
    else:
        media_1090.append(0)
        desvio_1090.append(0)
        n_1090.append(0)

    if s in dados["90/10"]:
        valores = dados["90/10"][s]
        media_9010.append(statistics.mean(valores))
        desvio_9010.append(
            statistics.stdev(valores) if len(valores) > 1 else 0
        )
        n_9010.append(len(valores))
    else:
        media_9010.append(0)
        desvio_9010.append(0)
        n_9010.append(0)

# ----------------------------------------------------
# Gráfico
# ----------------------------------------------------

x = np.arange(len(todos_surrogates))

largura = 0.35

plt.figure(figsize=(7,4))

'''
bars1 = plt.bar(
    x - largura/2,
    media_1090,
    largura,
    label="10/90"
)
'''

bars1 = plt.bar(
    x - largura/2,
    media_1090,
    largura,
    yerr=desvio_1090,
    capsize=5,
    label="10/90"
)

'''
bars2 = plt.bar(
    x + largura/2,
    media_9010,
    largura,
    label="90/10"
)
'''

bars2 = plt.bar(
    x + largura/2,
    media_9010,
    largura,
    yerr=desvio_9010,
    capsize=5,
    label="90/10"
)

'''
for bar in bars1:

    plt.text(
        bar.get_x()+bar.get_width()/2,
        bar.get_height()+0.4,
        f"{bar.get_height():.1f}",
        ha='center',
        fontsize=9
    )
'''

for bar, n in zip(bars1, n_1090):

    if bar.get_height() > 0:
        plt.text(
            bar.get_x() + bar.get_width()/2,
            bar.get_height() + 0.4,
            f"{bar.get_height():.1f}%\n(n={n})",
            ha='center',
            va='bottom',
            fontsize=8
        )

'''
for bar in bars2:

    plt.text(
        bar.get_x()+bar.get_width()/2,
        bar.get_height()+0.4,
        f"{bar.get_height():.1f}",
        ha='center',
        fontsize=9
    )
'''

for bar, n in zip(bars2, n_9010):

    if bar.get_height() > 0:
        plt.text(
            bar.get_x() + bar.get_width()/2,
            bar.get_height() + 0.4,
            f"{bar.get_height():.1f}%\n(n={n})",
            ha='center',
            va='bottom',
            fontsize=8
        )

plt.xticks(
    x,
    [str(s) for s in todos_surrogates]
)

plt.xlabel("Number of candidate surrogates")
plt.ylabel("Average execution time reduction (%)")
plt.title("Reduction Time grouped by number of surrogates")
plt.legend()

plt.grid(axis="y", linestyle="--", alpha=0.4)
plt.ylim(0, 75)

plt.tight_layout()

plt.show()