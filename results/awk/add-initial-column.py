#!/usr/bin/python
import os

#colocar numeros na primeira coluna dos .dat
for a in ["nts"]:  #tp: tempo de processamento; tso: taxa de sucesso de offloading; qto: quantidade de offloadings feitos; ol: % de execucoes apenas locais; tpr: tempo de processamento quando houve recuperacao; tsot: % total de sucessos, falhas, execucoes locais, recuperacoes; tpt: tempo de processamento total (offloading + execucoes apenas locais); gn: ganho de tempo em relacao a baseline (execucao apenas local); #nts: number of tasks by success type; kr: known routes; ecv: number of energy constraint violations
    for b in ["urban", "highway"]: #cenario
        for c in ["low", "medium", "high"]: #density
    	    for d in ["random2", "hvc", "mdo", "gtt", "abc"]: #algoritmo
			    os.system('awk -i inplace -f add-initial-column.awk ../dat/'+a+'-'+b+'-'+c+'-'+d+'.dat >> ../dat/'+a+'-'+b+'-'+c+'-'+d+'.dat')
