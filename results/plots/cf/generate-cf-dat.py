#!/usr/bin/python
import os

#apagar os .dat antes de calcular os novos .dat
for a in ["cf"]:  #gn: ganho de tempo em relacao a baseline (execucao apenas local)
    for b in ["highway"]: #qual o cenario
        for c in ["high"]: #densidade
            for d in ["10"]: #workload
                for e in ["20", "50", "100"]: #cycles
                    for f in ["20", "50", "100"]: #foods
                        for g in ["abc"]: #algorithm
            			    os.system('rm '+b+'-'+c+'-w'+d+'-c'+e+'-f'+f+'-'+g+'.dat')           

#calcular os novos .dat
for a in ["cf"]:
    for b in ["highway"]: #qual o cenario
        for c in ["high"]: #densidade
            for d in ["10"]: #workload
                for e in ["20", "50", "100"]: #cycles
                    for f in ["20", "50", "100"]: #foods
                        for g in ["abc"]: #algorithm
		                    print "calculando resultados ... \n"
		                    os.system('awk -f cf.awk '+b+'-'+c+'-w'+d+'-c'+e+'-f'+f+'-'+g+'.tr >> '+b+'-'+c+'-w'+d+'-c'+e+'-f'+f+'-'+g+'.dat')				
		                    print "resultados calculados ..."
