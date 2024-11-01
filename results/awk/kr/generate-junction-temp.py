#!/usr/bin/python
import os

calctype = ["junction"]
scenario = ["urban"]
density = ["low", "medium", "high"]
workload = ["5"]
algorithm = ["gtt", "abc"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  #junction: junta os algoritmos num arquivo so
    for b in scenario: #qual o cenario
        for c in density: #densidade                  
            for d in workload: #workload            
                for e in algorithm: #algorithm            
                    os.system('rm ../../traces/kr/junction/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.temp')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
for a in calctype:  #times: pega os tempos nos casos de sucesso
    for b in scenario: #qual o cenario
        for c in density: #densidade    	    
            for d in workload: #workload
                for e in algorithm: #algorithm
                    print "calculando resultados ... \n"
                    os.system('paste -d , ../../traces/kr/treated/'+b+'-'+c+'-kr0-w'+d+'-'+e+'.tr ../../traces/kr/treated/'+b+'-'+c+'-kr25-w'+d+'-'+e+'.tr ../../traces/kr/treated/'+b+'-'+c+'-kr50-w'+d+'-'+e+'.tr ../../traces/kr/treated/'+b+'-'+c+'-kr75-w'+d+'-'+e+'.tr ../../traces/kr/treated/'+b+'-'+c+'-kr100-w'+d+'-'+e+'.tr >> ../../traces/kr/junction/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.temp')
                    print "resultados calculados ..."
