from pathlib import Path
import matplotlib.pyplot as plt

# ============================================================
# Arquivo de resultados
# ============================================================

arquivo = Path("results/urban-medium-no-infra-static-ids-90moto10car-w1-gtt.tr")

# ============================================================
# IDs das motos (extraídos do TCL)
# ============================================================

#motorcycles = {102,103,104,105,106,107,108,109,118,121,131,133,143,146}
motorcycles = {
    6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
    45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
    64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,
    83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,
    102,103,105,106,107,108,109,110,111,112,113,115,116,118,
    119,120,121,122,123,124,125,126,127,129,130,132,133,134,
    135,136,137,138,139,140,141,143
}

# ============================================================

total_motorcycles = 0
total_cars = 0
runs_without_servers = 0
runs_analyzed = 0

print("=" * 60)
print("Server Type Distribution")
print("=" * 60)

with open(arquivo) as f:

    for linha in f:

        linha = linha.strip()

        if not linha:
            continue

        campos = linha.split(";")

        # ignora cabeçalho
        if campos[14] == "Run":
            continue

        if len(campos) < 20:
            continue

        run = int(campos[14])

        # Apenas as 30 execuções planejadas
        if run > 30:
            continue

        runs_analyzed += 1

        client = int(campos[15])

        print(f"\nRun {run}")
        print(f"Client {client}")

        # ====================================================
        # IDs dos servidores
        # ====================================================

        server_field = campos[16].strip()

        if server_field == "":
            runs_without_servers += 1
            print("   No registered servers.")
            continue

        server_ids = [int(x) for x in server_field.split(",") if x.strip() != ""]

        motos = 0
        carros = 0

        for sid in server_ids:

            if sid in motorcycles:
                motos += 1
                total_motorcycles += 1
                print(f"   {sid:3d} -> Motorcycle")
            else:
                carros += 1
                total_cars += 1
                print(f"   {sid:3d} -> Car")

        print(f"Motorcycle servers : {motos}")
        print(f"Car servers        : {carros}")

# ============================================================
# Estatísticas finais
# ============================================================

print("\n" + "=" * 60)
print("Overall Statistics")
print("=" * 60)

print(f"Runs analyzed                : {runs_analyzed}")
print(f"Runs without server recorded : {runs_without_servers}")
print()

print(f"Motorcycle servers : {total_motorcycles}")
print(f"Car servers        : {total_cars}")

total_servers = total_motorcycles + total_cars

if total_servers == 0:
    print("\nNo servers were recorded.")
    exit()

perc_moto = 100 * total_motorcycles / total_servers
perc_car = 100 * total_cars / total_servers

print()
print(f"Motorcycle : {perc_moto:.2f}%")
print(f"Car        : {perc_car:.2f}%")

# ============================================================
# Plot
# ============================================================

plt.figure(figsize=(6,4))

labels = ["Motorcycle", "Car"]
values = [perc_moto, perc_car]

bars = plt.bar(labels, values, width=0.55)

for bar, value in zip(bars, values):
    plt.text(
        bar.get_x() + bar.get_width()/2,
        value + 1,
        f"{value:.1f}%",
        ha="center",
        fontsize=10
    )

plt.xlabel("Server Type")
plt.ylabel("Selected Servers (%)")
plt.title("Server Type Distribution (90/10)")
plt.ylim(0,100)

plt.grid(axis="y", linestyle="--", alpha=0.5)

plt.tight_layout()

# plt.savefig("server_type_distribution_10_90.png", dpi=300)

plt.show()