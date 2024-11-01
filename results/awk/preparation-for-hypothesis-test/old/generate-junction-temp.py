#!/usr/bin/python
import os

 #for f in ["random2", "cloudnet2", "hvc", "gcf"]: #algorithm
#os.system('awk -v algo="'+f+'" -f times-no-recovery.awk ../../results/'+b+'-'+c+'-'+d+'-'+e+'-'+f+'.tr >> ../../plots/dat/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

#apagar os .dat antes de calcular os novos .dat
#for p in ["AODV", "DSDV", "DSR"]
for a in ["junction"]:  #junction: junta os algoritmos num arquivo so
    for b in ["highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade      
            for d in ["50half", "50interleaved", "100"]: #cobertura celular  
                for e in ["558000", "1200000"]: #tamanho do pacote            
		            os.system('rm ../../plots/dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.temp')

#gerar um arquivo temporario com a juncao dos arquivos dos algoritmos num so
#para o tp, o arquivo a ser gerado eh (exemplo): ...
for a in ["junction"]:  #times: pega os tempos nos casos de sucesso
    for b in ["highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade
    	    for d in ["50half", "50interleaved", "100"]: #cobertura celular
                for e in ["558000", "1200000"]: #tamanho do pacote
                    print "calculando resultados para "+a+'-'+b+'-'+c+'-'+d+'-'+e+"\n"
                    os.system('paste -d , ../../results/'+b+'-'+c+'-'+d+'-'+e+'-random2.tr ../../results/'+b+'-'+c+'-'+d+'-'+e+'-cloudnet2.tr ../../results/'+b+'-'+c+'-'+d+'-'+e+'-hvc.tr ../../results/'+b+'-'+c+'-'+d+'-'+e+'-gcf.tr >> ../../plots/dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.temp')
                    print "resultados calculados para "+a+'-'+b+'-'+c+'-'+d+'-'+e
