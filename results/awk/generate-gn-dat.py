#!/usr/bin/python
import os

#apagar os .dat antes de calcular os novos .dat
for a in ["gn"]:  #gn: ganho de tempo em relacao a baseline (execucao apenas local)
    for b in ["urban", "highway"]: #qual o cenario                
        for c in ["low", "medium", "high"]: #density
            for d in ["random2", "hvc", "mdo", "gtt", "abc"]: #algorithm
		        os.system('rm ../dat/'+a+'-'+b+'-'+c+'-'+d+'.dat') #coverage e workload vao dentro do arquivo

#calcular os novos .dat
for a in ["gn"]:
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade    	    
            for d in ["50half", "100"]: #coverage
                #for e in ["1", "2", "3", "4", "5"]: #workload
                for e in ["6", "8", "10"]: #workload
                    for f in ["random2", "hvc", "mdo", "gtt", "abc"]: #algorithm
		                print "calculando resultados para "+a+"-"+b+"-"+c+"-"+f+"\n"
		                os.system('awk -f gn.awk ../traces/'+b+'-'+c+'-'+d+'-w'+e+'-'+f+'.tr >> ../dat/'+a+'-'+b+'-'+c+'-'+f+'.dat')				
		                print "resultados calculados para "+a+"-"+b+"-"+c+"-"+f
