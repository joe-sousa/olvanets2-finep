#!/usr/bin/python
import os

 #for f in ["random2", "cloudnet2", "hvc", "gcf"]: #algorithm
#os.system('awk -v algo="'+f+'" -f times-no-recovery.awk ../../results/'+b+'-'+c+'-'+d+'-'+e+'-'+f+'.tr >> ../../plots/dat/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

for a in ["times-norecovery"]:  #times: pega os tempos nos casos de sucesso
    for b in ["highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade      
            for d in ["50half", "50interleaved", "100"]: #cobertura celular  
                for e in ["558000", "1200000"]: #tamanho do pacote            
		            os.system('rm ../../plots/dat/hyp-test/'+a+'-'+b+'-'+c+'-'+d+'-'+e+'.dat')

den = ""
cov = ""
#para o tp, o arquivo a ser gerado eh (exemplo): ...
for a in ["times-norecovery"]:  #times: pega os tempos nos casos de sucesso
    for b in ["highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade
    	    for d in ["50half", "50interleaved", "100"]: #cobertura celular
                for e in ["558000", "1200000"]: #tamanho do pacote
                    print "calculando resultados para "+a+'-'+b+'-'+c+'-'+d+'-'+e+"\n"
                    if(c=="low"):
                        den="L"
                    if(c=="medium"):
                        den="M"
                    if(c=="high"):
                        den="H"
                    if(d=="50half"):
                        cov="H"
                    if(d=="50interleaved"):
                        cov="I"
                    if(d=="100"):
                        cov="F"
                    os.system('awk -f times-norecovery.awk ../../plots/dat/hyp-test/junction-'+b+'-'+c+'-'+d+'-'+e+'.temp >> ../../plots/dat/hyp-test/'+a+'-'+b+'-'+den+'-'+cov+'-'+e+'.csv')
                    print "resultados calculados para "+a+'-'+b+'-'+c+'-'+d+'-'+e
