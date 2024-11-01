#!/usr/bin/python
import os

calctype = ["junction"]
scenario = ["urban"]
density = ["low", "medium", "high"]
workload = ["5"]
algorithm = ["gtt", "abc"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  #junction
    for b in scenario: #qual o cenario
        for c in density: #densidade                  
            for d in workload: #workload                                
                for e in algorithm: #algorithm                                
                    os.system('rm ../../traces/kr/after-treated/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.temp2')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
for a in calctype:  #junction
    for b in scenario: #qual o cenario
        for c in density: #densidade
            for d in workload: #workload                    
                for e in algorithm: #algorithm
                    print "calculando resultados ... \n"
                    os.system('awk -f after-treat.awk ../../traces/kr/junction/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.temp >> ../../traces/kr/after-treated/'+a+'-'+b+'-'+c+'-w'+d+'-'+e+'.temp2')
                    print "resultados calculados ..."
