#!/usr/bin/python
import os

 #for f in ["random2", "cloudnet2", "hvc", "gcf"]: #algorithm
#os.system('awk -v algo="'+f+'" -f times-no-recovery.awk ../../results/'+b+'-'+c+'-'+d+'-'+e+'-'+f+'.tr >> ../../plots/dat/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

#apagar os .dat antes de calcular os novos .dat
for a in ["urban", "highway"]: #qual o cenario
    for b in ["low","medium","high"]: #densidade                  
        for c in ["6", "8", "10"]: #workload
            for d in ["6", "8", "10"]: #workload
                for e in ["random2", "hvc", "gtt", "abc"]: #algorithm            
                    os.system('rm treatedresults/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.tr')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
#para o tp, o arquivo a ser gerado eh (exemplo): ...
for a in ["urban", "highway"]: #qual o cenario
    for b in ["low","medium","high"]: #densidade    	    
        for c in ["50half", "100"]: #coverage
            for d in ["6", "8", "10"]: #workload
                for e in ["random2", "hvc", "gtt", "abc"]: #algorithm
                    print "calculando resultados para "+a+'-'+b+'-'+c+'-'+d+'-'+e+"\n"
                    os.system('awk -f treat.awk /home/alisson/ns-3.29/results/traces/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.tr >> treatedresults/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.tr')
                    print "calculando resultados para "+a+'-'+b+'-'+c+'-'+d+'-'+e
