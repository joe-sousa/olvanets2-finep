#!/usr/bin/python
import os

calctype = ["treat"]
scenario = ["urban"]
density = ["low", "medium", "high"]
workload = ["5"]
algorithm = ["gtt", "abc"]
kr = ["0", "25", "50", "75", "100"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  #treat: trata os resultados p imprimir as linhas q n tem run
    for b in scenario: #qual o cenario
        for c in density: #densidade                  
            for d in kr: #kr
                for e in workload: #workload
                    for f in algorithm: #algorithm            
	                    os.system('rm ../../traces/kr/treated/'+b+'-'+c+'-kr'+d+'-w'+e+'-'+f+'.tr')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
for a in calctype:  #treat
    for b in scenario: #qual o cenario
        for c in density: #densidade    	    
            for d in kr: #kr
                for e in workload: #workload
                    for f in algorithm: #algorithm
                        print "calculando resultados ... \n"
                        os.system('awk -f treat.awk ../../traces/kr/'+b+'-'+c+'-kr'+d+'-w'+e+'-'+f+'.tr >> ../../traces/kr/treated/'+b+'-'+c+'-kr'+d+'-w'+e+'-'+f+'.tr')
                        print "resultados calculados ..."
