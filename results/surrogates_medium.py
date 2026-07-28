from pathlib import Path
import statistics
import matplotlib.pyplot as plt

# Diretório dos resultados
base = Path("results")

# -------------------------------
# Cenário sem bolsão
# -------------------------------
"""
arquivos = {
    "10/90": "urban-medium-no-infra-10moto90car-w1-gtt.tr",
    "30/70": "urban-medium-no-infra-30moto70car-w1-gtt.tr",
    "50/50": "urban-medium-no-infra-50moto50car-w1-gtt.tr",
    "70/30": "urban-medium-no-infra-70moto30car-w1-gtt.tr",
    "90/10": "urban-medium-no-infra-90moto10car-w1-gtt.tr",
}


# Cenário com bolsão
arquivos = {
    "10/90": "urban-medium-bikebox-10moto90car-w1-gtt.tr",
    "30/70": "urban-medium-bikebox-30moto70car-w1-gtt.tr",
    "50/50": "urban-medium-bikebox-50moto50car-w1-gtt.tr",
    "70/30": "urban-medium-bikebox-70moto30car-w1-gtt.tr",
    "90/10": "urban-medium-bikebox-90moto10car-w1-gtt.tr",
}

arquivos = {
    "10/90" : "urban-medium-no-infra-static-ids-10moto90car-w1-gtt.tr",
    "90/10" : "urban-medium-no-infra-static-ids-90moto10car-w1-gtt.tr",
}
"""

arquivos = {
    "10/90" : "urban-medium-no-infra-moto-agressive-10moto90car-w1-gtt.tr",
    "90/10" : "urban-medium-no-infra-moto-agressive-90moto10car-w1-gtt.tr",
}


cenarios = []
medias_surrogates = []

print("=" * 60)
print("Average Number of Surrogates")
print("=" * 60)

for nome, arquivo in arquivos.items():

    surrogates = []

    with open(base / arquivo) as f:

        for linha in f:

            if not linha.strip():
                continue

            campos = linha.strip().split(";")
            if campos[0] == "ExecutionResult":
                continue

            run = int(campos[14])

            # Usa apenas as 30 execuções planejadas
            if run > 31:
                continue

            # numberOfSurrogates
            surrogates.append(int(campos[3]))

    media_surrogates = statistics.mean(surrogates)

    cenarios.append(nome)
    medias_surrogates.append(media_surrogates)

    print(f"{nome:>5} -> {media_surrogates:.2f}")

# -------------------------------
# Gráfico
# -------------------------------

plt.figure(figsize=(7,4))

plt.bar(cenarios, medias_surrogates)

# Valor acima das barras
for x, y in zip(cenarios, medias_surrogates):
    plt.text(x, y + 0.03, f"{y:.2f}",
             ha="center", va="bottom", fontsize=9)

plt.xlabel("Motorcycle / Car proportion")
plt.ylabel("Average number of surrogates")
plt.title("Scenario No infra")

plt.ylim(0, max(medias_surrogates) + 0.5)

plt.grid(axis="y", linestyle="--", alpha=0.5)

plt.tight_layout()

# plt.savefig("average_surrogates_no_bikebox.png", dpi=300)

plt.show()