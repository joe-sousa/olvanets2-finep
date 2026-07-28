from pathlib import Path
import statistics
import matplotlib.pyplot as plt

# Diretório dos resultados
base = Path("results")

"""
arquivos = {
    "10/90": "urban-medium-no-infra-10moto90car-w1-gtt.tr",
    "30/70": "urban-medium-no-infra-30moto70car-w1-gtt.tr",
    "50/50": "urban-medium-no-infra-50moto50car-w1-gtt.tr",
    "70/30": "urban-medium-no-infra-70moto30car-w1-gtt.tr",
    "90/10": "urban-medium-no-infra-90moto10car-w1-gtt.tr",
}

arquivos = {
    "10/90": "urban-medium-bikebox-10moto90car-w1-gtt.tr",
    "30/70": "urban-medium-bikebox-30moto70car-w1-gtt.tr",
    "50/50": "urban-medium-bikebox-50moto50car-w1-gtt.tr",
    "70/30": "urban-medium-bikebox-70moto30car-w1-gtt.tr",
    "90/10": "urban-medium-bikebox-90moto10car-w1-gtt.tr",
}
"""

arquivos = {
    "10/90" : "urban-medium-no-infra-moto-agressive-10moto90car-w1-gtt.tr",
    "90/10" : "urban-medium-no-infra-moto-agressive-90moto10car-w1-gtt.tr",
}


cenarios = []
medias = []

for nome, arquivo in arquivos.items():

    reducoes = []
    surrogates = []

    with open(base / arquivo) as f:
        for linha in f:

            if not linha.strip():
                continue

            campos = linha.strip().split(";")
            if campos[0] == "ExecutionResult":
                continue

            run = int(campos[14])

            # usa apenas as 30 execuções válidas
            if run > 31:
                continue

            variation = float(campos[7])

            reducoes.append(-variation)
            
            # coluna numberOfSurrogates
            surrogates.append(int(campos[3]))
                     
    cenarios.append(nome)
    medias.append(statistics.mean(reducoes))
    media_surrogates = statistics.mean(surrogates)

    print(f"  Surrogates médios : {media_surrogates:.2f}")

"""
# -------------------------------
# Gráfico
# -------------------------------

plt.figure(figsize=(7,4))

plt.plot(
    cenarios,
    medias,
    marker='o',
    linestyle='None',
    markersize=8
)

for x, y in zip(cenarios, medias):
    plt.text(x, y+0.5, f"{y:.1f}%", ha='center', fontsize=9)

plt.xlabel("Motorcycle / Car proportion")
plt.ylabel("Average execution time reduction (%)")
plt.title("Scenario With Bikebox")
#plt.grid(True, linestyle="--", alpha=0.5)

plt.tight_layout()

#plt.savefig("reduction_execution_time_with_bikebox.png", dpi=300)

plt.show()
"""

# -------------------------------
# Gráfico de barras
# -------------------------------

plt.figure(figsize=(6,4))

bars = plt.bar(
    cenarios,
    medias,
    width=0.55
)

# Valor acima de cada barra
for bar, media in zip(bars, medias):
    plt.text(
        bar.get_x() + bar.get_width()/2,
        media + 0.5,
        f"{media:.1f}%",
        ha='center',
        va='bottom',
        fontsize=10
    )

plt.xlabel("Motorcycle / Car proportion")
plt.ylabel("Average execution time reduction (%)")
plt.title("Scenario Without Infrastructure - Aggressive Moto")

plt.ylim(0, max(medias) + 5)

plt.grid(axis='y', linestyle='--', alpha=0.4)

plt.tight_layout()

# plt.savefig("reduction_execution_time_bar.png", dpi=300)

plt.show()