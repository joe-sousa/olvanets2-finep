#!/usr/bin/python
import os

calctype = ["ecv"] #join workloads to ecv
scenario = ["highway", "urban"]
density = ["low", "medium", "high"]
coverage = ["50half", "100"]
workload = ["6", "8", "10"]
algorithm = ["mdo"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  
    for b in scenario: #qual o cenario
        for c in density: #densidade                  
            for d in coverage: #coverage
                for e in algorithm: #algorithm            
                    os.system('rm ../../traces/ecv/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.temp')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
for a in calctype:  #times: pega os tempos nos casos de sucesso
    for b in scenario: #qual o cenario
        for c in density: #densidade    	
            for d in coverage: #coverage    
                for e in algorithm: #algorithm
                    print "calculando resultados ... \n"
                    os.system('cat ../../traces/'+b+'-'+c+'-'+d+'-w6'+'-'+e+'.tr ../../traces/'+b+'-'+c+'-'+d+'-w8'+'-'+e+'.tr ../../traces/'+b+'-'+c+'-'+d+'-w10'+'-'+e+'.tr >> ../../traces/ecv/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.temp')
                    print "resultados calculados ..."
