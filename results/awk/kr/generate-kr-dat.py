#!/usr/bin/python
import os

calctype = ["kr"]
scenario = ["urban"]
density = ["low", "medium", "high"]
workload = ["5"]
algorithm = ["gtt", "abc"]
kr = ["0", "25", "50", "75", "100"]

#apagar os .dat antes de calcular os novos .dat
for a in calctype:  #kr: known routes
    for b in scenario: #qual o cenario        
        for c in density: #density            
            for d in algorithm: #algorithm
		        os.system('rm ../../dat/kr/'+a+'-'+b+'-'+c+'-'+d+'.dat')           

#calcular os novos .dat
for a in calctype:
    for b in scenario: #qual o cenario
        for c in density: #densidade    	    
            for d in workload: #workload
                for e in algorithm: #algorithm
                    print "calculando resultados ... \n"
                    os.system('awk -f kr.awk ../../traces/kr/after-treated/junction-'+b+'-'+c+'-w'+d+'-'+e+'.temp2 >> ../../dat/kr/'+a+'-'+b+'-'+c+'-'+e+'.dat')				
                    print "resultados calculados ..."

#ja coloca numeros na primeira coluna dos .dat (o add-initial-column.py)
for a in calctype:  #kr: known routes
    for b in scenario: #cenario
        for c in density: #density            
    	    for d in algorithm: #algoritmo
		        os.system('awk -i inplace -f add-initial-column.awk ../../dat/kr/'+a+'-'+b+'-'+c+'-'+d+'.dat >> ../../dat/kr/'+a+'-'+b+'-'+c+'-'+d+'.dat')
