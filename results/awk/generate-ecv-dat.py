#!/usr/bin/python
import os

#apagar os .dat antes de calcular os novos .dat
for a in ["ecv"]:  #ecv: number of energy constraint violations
    for b in ["urban", "highway"]: #qual o cenario        
        for c in ["mdo"]: #algorithm
		    os.system('rm ../dat/ecv/'+a+'-'+b+'-'+c+'.dat')           

#calcular os novos .dat
for a in ["ecv"]:
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["mdo"]: #algorithm
            for e in ["50half", "100"]: #coverage
                for d in ["low","medium","high"]: #densidade                                    
                    print "calculando resultados para "+b+"-"+c+"\n"
                    os.system('awk -f ecv.awk ../traces/ecv/'+a+'-'+b+'-'+d+'-'+e+'-'+c+'.temp >> ../dat/ecv/'+a+'-'+b+'-'+c+'.dat')				
                    print "resultados calculados para "+b+"-"+c
