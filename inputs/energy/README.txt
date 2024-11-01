50% dos carros em movimento são a combustão e 50% são elétricos.
50% dos carros parados são a combustão e 50% são elétricos.


Cenário de highway não tem carros desligados e estacionados. Temos 4 estacionamentos no cenário urbano com as seguintes quantidades de veículos: 10 (Low), 30 (Medium) e 50 (High).
Lembrando que apenas 10% dos carros desligados continuam com sua parte eletrônica ativa p comunicações (paper da Susana Sargento de ~2015).


A energia mínima é dividida assim:
VCM (Veículo a Combustão em Movimento): -999 Wh (Watt-hour) - não há limite mínimo pois o alternador do carro recarrega a bateria
VCP (Veículo a Combustão Parado): 612 Wh
VEM (Veículo Elétrico em Movimento): 4000 Wh
VEP (Veículo Elétrico Parado): 4000 Wh
Edge: -999 Wh - não há limite mínimo pois o servidor de borda está ligado à operadora de energia elétrica (não setada nos txts; apenas no código)

A Edge será setada sem precisar de arquivo. Em Highway, não existem VCPs e VEPs. Em Urban, os veículos que são VCPs e VEPs são os 52-61 (Low density), 276-305 (Medium density) e 602-651 (High density).


O nível de energia atual é dividida assim:
VCM: 999999 Wh - não há limite mínimo pois o alternador do carro recarrega a bateria
VCP: [600, 720] Wh - new: [620, 720] Wh
VEM: [2000, 20000] Wh - new: [5000, 20000] Wh
VEP: [2000, 20000] Wh - new: [5000, 20000] Wh
Edge: 999999 Wh - não há limite mínimo pois o servidor de borda está ligado à operadora de energia elétrica (não setada nos txts; apenas no código)

A Edge será setada sem precisar de arquivo. Em Highway, não existem VCPs e VEPs. Em Urban, os veículos que são VCPs e VEPs são os 52-61 (Low density), 276-305 (Medium density) e 602-651 (High density).


A energia gasta nas comunicações (transmissões) são iguais p todos os veículos; entre as edges, são iguais tb. Por isso, são setadas apenas no código (sem precisar desses txts).


A energia gasta no processamento de tasks é dividida assim:
Veículos
CPU de 0.5 GHz: {18,20,22} Wh (~20 Wh)
CPU de 1.0 GHz: {28,30,32} Wh (~30 Wh)
Edge Servers
CPU de 1.5 GHz: 40 Wh
CPU de 2.0 GHz: 50 Wh
CPU de 2.5 GHz: 60 Wh

Detalhe: um consumo 20 Wh significa que durante 1 hora (3600 segundos) são consumidos 20 Watts. Para saber calcular o Wh em 1 segundo, é só fazer uma regra de 3 simples.

