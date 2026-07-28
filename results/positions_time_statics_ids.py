from pathlib import Path
from reportlab.lib import colors
from reportlab.lib.pagesizes import A4, landscape
from reportlab.lib.units import cm
from reportlab.platypus import SimpleDocTemplate, Table, TableStyle, Paragraph
from reportlab.lib.styles import getSampleStyleSheet

base = Path("results")

arquivo_10 = base / "urban-medium-no-infra-static-ids-10moto90car-w1-gtt.tr"
arquivo_90 = base / "urban-medium-no-infra-static-ids-90moto10car-w1-gtt.tr"


def carregar(arquivo):

    dados = {}

    with open(arquivo) as f:

        for linha in f:

            linha = linha.strip()

            if not linha:
                continue

            campos = linha.split(";")

            if campos[0] == "ExecutionResult":
                continue

            run = int(campos[14])

            if run > 30:
                continue

            dados[run] = {
                "client": int(campos[15]),
                "x": float(campos[17]),
                "y": float(campos[18]),
                "time": float(campos[19]),
            }

    return dados


dados10 = carregar(arquivo_10)
dados90 = carregar(arquivo_90)


styles = getSampleStyleSheet()

doc = SimpleDocTemplate(
    "Comparison_Selected_Clients.pdf",
    pagesize=landscape(A4),
    leftMargin=0.6*cm,
    rightMargin=0.6*cm,
    topMargin=0.7*cm,
    bottomMargin=0.7*cm,
)

elementos = []

elementos.append(
    Paragraph(
        "<b>Comparison of Selected Clients Between 10/90 and 90/10 Scenarios</b>",
        styles["Title"],
    )
)

elementos.append(
    Paragraph(
        "The same Client IDs were intentionally selected in both scenarios. "
        "The table reports the client position and timestamp at the beginning "
        "of the offloading process.",
        styles["Normal"],
    )
)

tabela = [[
    "Run",
    "Client\n(10/90)",
    "X",
    "Y",
    "Timestamp",
    "Client\n(90/10)",
    "X",
    "Y",
    "Timestamp"
]]

for run in range(1,31):

    a = dados10[run]
    b = dados90[run]

    tabela.append([
        run,
        a["client"],
        f"{a['x']:.3f}",
        f"{a['y']:.3f}",
        f"{a['time']:.3f}",
        b["client"],
        f"{b['x']:.3f}",
        f"{b['y']:.3f}",
        f"{b['time']:.3f}",
    ])


larguras = [
    1.0*cm,   # Run
    2.0*cm,   # Client
    2.8*cm,   # X
    2.8*cm,   # Y
    2.4*cm,   # Time
    2.0*cm,   # Client
    2.8*cm,   # X
    2.8*cm,   # Y
    2.4*cm,   # Time
]

table = Table(tabela, colWidths=larguras)

table.setStyle(TableStyle([

    ('BACKGROUND',(0,0),(-1,0),colors.HexColor("#1F4E78")),
    ('TEXTCOLOR',(0,0),(-1,0),colors.white),

    ('FONTNAME',(0,0),(-1,0),'Helvetica-Bold'),
    ('FONTNAME',(0,1),(-1,-1),'Helvetica'),

    ('FONTSIZE',(0,0),(-1,-1),8),

    ('ALIGN',(0,0),(-1,-1),'CENTER'),
    ('VALIGN',(0,0),(-1,-1),'MIDDLE'),

    ('GRID',(0,0),(-1,-1),0.25,colors.grey),

    ('ROWBACKGROUNDS',(0,1),(-1,-1),
        [colors.whitesmoke, colors.beige]),

    ('BOTTOMPADDING',(0,0),(-1,0),7),
]))

elementos.append(table)

doc.build(elementos)

print("\nPDF criado com sucesso:")
print("Comparison_Selected_Clients.pdf")