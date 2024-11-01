#!/usr/bin/python
import os

 #for f in ["random2", "cloudnet2", "hvc", "gcf"]: #algorithm
#os.system('awk -v algo="'+f+'" -f times-no-recovery.awk ../../results/'+b+'-'+c+'-'+d+'-'+e+'-'+f+'.tr >> ../../plots/dat/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

for a in ["gn"]:  #gn: pega os ganhos de tempo em relacao a baseline (tempo executado apenas localmente)
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade                  
            for d in ["50half","100"]: #coverage
                for e in ["6", "8", "10"]: #workload            
	                os.system('rm ../../dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-w'+e+'.csv')

#para o tp, o arquivo a ser gerado eh (exemplo): ...
for a in ["gn"]:  #times: pega os tempos nos casos de sucesso
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade
            for d in ["50half","100"]: #coverage    	        	    
                for e in ["6", "8", "10"]: #workload
                    print "calculando resultados para "+a+'-'+b+'-'+c+'-'+d+'-w'+e+"\n"                    
                    os.system('awk -f gn.awk ../../dat/hyp-test/junction-'+b+'-'+c+'-'+d+'-w'+e+'.temp >> ../../dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-w'+e+'.csv')
                    print "resultados calculados para "+a+'-'+b+'-'+c+'-'+d+'-w'+e
