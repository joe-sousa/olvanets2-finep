import xml.etree.ElementTree as ET

input_file = "AvenidaPaulistaVersaoSemBolsoesNasVias.net.xml"
output_file = "AvenidaPaulista.net.xml"

tree = ET.parse(input_file)
root = tree.getroot()

# Detecta namespace automaticamente
ns = ''
if root.tag.startswith('{'):
    ns = root.tag.split('}')[0] + '}'

# Conta semáforos
tls_ids = set()
for junction in root.findall(f".//{ns}junction"):
    if junction.get("type") == "traffic_light":
        tls_ids.add(junction.get("id"))

print(f"Encontrados {len(tls_ids)} junctions com semáforo.")

count_lanes = 0

for edge in root.findall(f".//{ns}edge"):
    to_node = edge.get("to")
    if to_node in tls_ids:
        for lane in edge.findall(f"{ns}lane"):
            lane.set("stopOffset", "6.0")
            lane.set("stopOffsetException", "motorcycle")
            count_lanes += 1

tree.write(output_file)

print(f"Aplicado stopOffset em {count_lanes} lanes.")
print("Arquivo salvo como AvenidaPaulista_bolsao.net.xml")
