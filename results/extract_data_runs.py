#!/usr/bin/env python3

from pathlib import Path
import argparse
import csv
import sys

# Colunas do arquivo .tr
COL_RUN = 14
COL_CLIENT = 15
COL_TIMESTAMP = 17
COL_NEIGHBORS = 18


def main():

    parser = argparse.ArgumentParser(
        description="Extract Run, Client ID, Timestamp, and Number of Neighbors from a .tr File"
    )

    parser.add_argument("input", help="Arquivo .tr")
    parser.add_argument("output", help="Arquivo CSV de saída")
    parser.add_argument("--scenario", required=True,
                        help="Nome do cenário (90/10 ou 10/90)")

    args = parser.parse_args()

    rows = []

    with open(args.input, "r") as f:

        for line in f:

            line = line.strip()

            if not line:
                continue

            if not (line.startswith("N;") or line.startswith("T;")):
                continue

            fields = line.split(";")

            if len(fields) < 19:
                continue

            rows.append({
                "scenario": args.scenario,
                "run": int(fields[COL_RUN]),
                "clientId": int(fields[COL_CLIENT]),
                "timestamp": int(fields[COL_TIMESTAMP]),
                "numberOfNeighbors": int(fields[COL_NEIGHBORS])
            })

    rows.sort(key=lambda x: x["run"])

    with open(args.output, "w", newline="") as csvfile:

        writer = csv.writer(csvfile, delimiter=";")

        writer.writerow([
            "scenario",
            "run",
            "clientId",
            "timestamp",
            "numberOfNeighbors"
        ])

        for row in rows:
            writer.writerow([
                row["scenario"],
                row["run"],
                row["clientId"],
                row["timestamp"],
                row["numberOfNeighbors"]
            ])

    print(f"Arquivo gerado: {args.output}")
    print(f"Runs encontradas: {len(rows)}")


if __name__ == "__main__":
    main()