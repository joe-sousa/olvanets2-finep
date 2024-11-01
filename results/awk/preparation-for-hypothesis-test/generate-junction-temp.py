#!/usr/bin/python
import os

 #for f in ["random2", "cloudnet2", "hvc", "gcf"]: #algorithm
#os.system('awk -v algo="'+f+'" -f times-no-recovery.awk ../../results/'+b+'-'+c+'-'+d+'-'+e+'-'+f+'.tr >> ../../plots/dat/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

#apagar os .dat antes de calcular os novos .dat
for a in ["junction"]:  #junction: junta os algoritmos num arquivo so
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade                  
            for d in ["50half","100"]: #coverage
                for e in ["6", "8", "10"]: #workload            
	                os.system('rm ../../dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-w'+e+'.temp')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
#para o tp, o arquivo a ser gerado eh (exemplo): ...
for a in ["junction"]:  #times: pega os tempos nos casos de sucesso
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade    	    
            for d in ["50half", "100"]: #workload
                for e in ["6", "8", "10"]: #workload
                    print "calculando resultados para "+b+'-'+c+'-w'+d+"\n"
                    os.system('paste -d , treatedresults/'+b+'-'+c+'-'+d+'-w'+e+'-random2.tr treatedresults/'+b+'-'+c+'-'+d+'-w'+e+'-hvc.tr treatedresults/'+b+'-'+c+'-'+d+'-w'+e+'-gtt.tr treatedresults/'+b+'-'+c+'-'+d+'-w'+e+'-abc.tr >> ../../dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-w'+e+'.temp')
                    print "resultados calculados para "+b+'-'+c+'-w'+d
