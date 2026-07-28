#!/usr/bin/env python3

import csv
import argparse

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.styles import getSampleStyleSheet
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle


def read_csv(filename):

    runs = {}

    with open(filename, "r") as f:

        reader = csv.DictReader(f, delimiter=";")

        for row in reader:

            run = int(row["run"])

            runs[run] = {
                "clientId": int(row["clientId"]),
                "timestamp": int(row["timestamp"]),
                "neighbors": int(row["numberOfNeighbors"])
            }

    return runs


def main():

    parser = argparse.ArgumentParser(
        description="Generate a PDF comparing paired executions from the 90/10 and 10/90 scenarios."
    )

    parser.add_argument(
        "scenario90",
        help="CSV generated from the 90/10 scenario"
    )

    parser.add_argument(
        "scenario10",
        help="CSV generated from the 10/90 scenario"
    )

    parser.add_argument(
        "output",
        help="Output PDF"
    )

    args = parser.parse_args()

    data90 = read_csv(args.scenario90)
    data10 = read_csv(args.scenario10)

    all_runs = sorted(set(data90.keys()) | set(data10.keys()))

    table_data = [[
        "Run",
        "Client (90/10)",
        "Timestamp (90/10)",
        "Neighbors (90/10)",
        "Client (10/90)",
        "Timestamp (10/90)",
        "Neighbors (10/90)"
    ]]

    for run in all_runs:

        d90 = data90.get(run)
        d10 = data10.get(run)

        if d90 is None or d10 is None:
            continue

        table_data.append([
            run,
            d90["clientId"],
            d90["timestamp"],
            d90["neighbors"],
            d10["clientId"],
            d10["timestamp"],
            d10["neighbors"]
        ])

    doc = SimpleDocTemplate(
        args.output,
        pagesize=landscape(A4),
        leftMargin=20,
        rightMargin=20,
        topMargin=20,
        bottomMargin=20
    )

    table = Table(table_data)

    table.setStyle(TableStyle([

        ('BACKGROUND', (0,0), (-1,0), colors.lightgrey),

        ('TEXTCOLOR', (0,0), (-1,0), colors.black),

        ('ALIGN', (0,0), (-1,-1), 'CENTER'),

        ('VALIGN', (0,0), (-1,-1), 'MIDDLE'),

        ('FONTNAME', (0,0), (-1,0), 'Helvetica-Bold'),

        ('FONTNAME', (0,1), (-1,-1), 'Helvetica'),

        ('FONTSIZE', (0,0), (-1,-1), 9),

        ('BOTTOMPADDING', (0,0), (-1,0), 8),

        ('GRID', (0,0), (-1,-1), 0.5, colors.black),

    ]))

    elements = [table]

    doc.build(elements)

    print(f"PDF saved to {args.output}")


if __name__ == "__main__":
    main()