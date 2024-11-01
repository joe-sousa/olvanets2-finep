#!/usr/bin/python
import os

calctype = ["range"]
scenario = ["urban", "highway"]
density = ["low", "medium", "high"]
workload = ["10"]
algorithm = ["gtt"]
txpower = ["16.700000", "22.350000", "28.550000"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  #gn: ganho de tempo em relacao a baseline (execucao apenas local)
    for b in scenario: #qual o cenario        
        for c in density: #densidade
            for d in workload: #workload
                for f in algorithm: #algorithm
			        os.system('rm ../dat/range/'+a+'-'+b+'-'+c+'-w'+d+'-'+f+'.dat')           

#calcular os novos .dat
for a in calctype:
    for b in scenario: #qual o cenario
        for c in density: #densidade
            for d in workload: #workload
                for e in txpower: #txpower
                    for f in algorithm: #algorithm
	                    print "calculando resultados ... \n"
	                    os.system('awk -f range.awk ../traces/range/'+b+'-'+c+'-w'+d+'-'+e+'-'+f+'.tr >> ../dat/range/'+a+'-'+b+'-'+c+'-w'+d+'-'+f+'.dat')				
	                    print "resultados calculados ..."

#ja coloca numeros na primeira coluna dos .dat (o add-initial-column.py)
for a in calctype: #range
    for b in scenario: #cenario
        for c in density: #density
            for d in workload: #workload
	            for e in algorithm: #algoritmo
		            os.system('awk -i inplace -f add-initial-column.awk ../dat/range/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.dat >> ../dat/range/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.dat')
