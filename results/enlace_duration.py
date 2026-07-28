from pathlib import Path
import statistics
import re

# Diretório dos logs
base = Path("logs")

prefixo = "urban-medium-100urban-medium-10moto90car-no-infra-w1-gtt"

regex = re.compile(r"Tempo de vida do enlace ==> ([0-9.]+)")

medias = []
execucoes = 0

print("=" * 60)
print("LET médio")
print("=" * 60)

for run in range(1, 32):

    arquivo = base / f"{prefixo}-{run}.log"
    #print(arquivo, arquivo.exists())
    
    if not arquivo.exists():
        continue

    lets = []

    with open(arquivo, encoding="utf-8", errors="ignore") as f:
        for linha in f:
            m = regex.search(linha)
            if m:
                lets.append(float(m.group(1).rstrip(".")))

    if lets:
        media = statistics.mean(lets)
        medias.append(media)
        execucoes += 1

        print(f"Run {execucoes:2d} -> LET médio = {media:.2f}")

    if execucoes == 30:
        break

print("\n" + "=" * 60)
print(f"LET médio do cenário : {statistics.mean(medias):.2f}")
print(f"Execuções analisadas : {execucoes}")