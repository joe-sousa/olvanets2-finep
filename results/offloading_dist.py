from pathlib import Path
import matplotlib.pyplot as plt

base = Path("results")

'''
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

arquivos = {
    "10/90" : "urban-medium-no-infra-static-ids-10moto90car-w1-gtt.tr",
    "90/10" : "urban-medium-no-infra-static-ids-90moto10car-w1-gtt.tr",
}
'''

arquivos = {
    "10/90" : "urban-medium-no-infra-moto-agressive-10moto90car-w1-gtt.tr",
    "90/10" : "urban-medium-no-infra-moto-agressive-90moto10car-w1-gtt.tr",
}

cenarios = []

local_pct = []
success_pct = []
recovery_pct = []

print("="*70)
print("Task Outcome Distribution")
print("="*70)

for nome, arquivo in arquivos.items():

    local = 0
    success = 0
    recovery = 0
    total = 0

    with open(base / arquivo) as f:

        for linha in f:

            if not linha.strip():
                continue

            campos = linha.strip().split(";")

            # Ignora o cabeçalho
            if campos[0] == "ExecutionResult":
                continue

            run = int(campos[14])

            if run > 31:
                continue

            local += int(campos[8])
            success += int(campos[9])
            recovery += int(campos[10])
            total += int(campos[11])

    local = 100*local/total
    success = 100*success/total
    recovery = 100*recovery/total

    print(nome)
    print("Local =", local)
    print("Offloaded =", success)
    print("Recovered =", recovery)
    print("Total =", total)
    print("Soma =", local + success + recovery)
    print()

    print(f"{nome}:")
    print(f"   Local     : {local:.2f}%")
    print(f"   Success   : {success:.2f}%")
    print(f"   Recovery  : {recovery:.2f}%")
    print()

    cenarios.append(nome)
    local_pct.append(local)
    success_pct.append(success)
    recovery_pct.append(recovery)

# -----------------------------
# Gráfico
# -----------------------------

plt.figure(figsize=(8,5))

plt.bar(
    cenarios,
    local_pct,
    label="Local Execution"
)

plt.bar(
    cenarios,
    success_pct,
    bottom=local_pct,
    label="Successful Offloading"
)

plt.ylabel("Task occurrence (%)")
plt.xlabel("Motorcycle / Car proportion")
plt.title("Task Outcome Distribution (No Bikebox) Aggressive Moto")
plt.legend()

plt.tight_layout()

plt.show()