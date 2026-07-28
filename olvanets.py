#!/usr/bin/python
import os

#examples
#oneImage = ["558000", "1200000"]
#os.system("./waf --run \"fkastelein_script --nWifi=" +str(x) + "  --packetSize=1024 --mode=1 --dataRateSetting=1 --fileName='dataRateTotal.txt'\"")
#os.system('./waf --run "scratch/mcRouteVariableTxPower --seed='+`run`+' --np='+`pu`+'" >> 2016_12_04_pu.txt')
#--tracePath=/home/alisson/ns-3.29/mobilityTraces/ --traceFile=highway-low.tcl --logFile=/home/alisson/ns-3.29/work/saida.txt --oneImage=66490-1.903 --trange=200 --surrogates=2 --choice=true
#os.system('./waf --run "scratch/vanets_tcp_manhattan --run=' + run + ' --tracePath=' + path + ' --scenario=' + a + ' --density=' + b + ' --cellcoverage=' + c + ' --oneImage=' + d + ' --trange=' + str(e) + ' --surrogates=' + str(f) +  ' --algorithm=' + g +   '"')
#--run=1 --tracePath=/home/alisson/ns-3.29/mobilityTraces/ --scenario=urban --density=low --cellcoverage=100 --oneImage=72000 --trange=251.0 --surrogates=2 --algorithm=cloudnet2
#os.system('./build/scratch/olvanets --run=' + run + ' --tracePath=' + path + ' --scenario=' + a + ' --density=' + b + ' --cellcoverage=' + c + ' --oneImage=' + d + ' --trange=' + str(trange) + ' --surrogates=' + str(surrogates) +  ' --algorithm=' + e +   '"')

#os.system('./waf shell') #executar o ns-3 sem precisar buildar todas as vezes

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(BASE_DIR, "mobilityTraces") + "/"

#path = "/home/joel/ns3-docker/olvanets2/mobilityTraces/"
#trange = "251.0"
#scenario = ["urban", "highway"]
scenario = ["urban"]
#density = ["low", "medium", "high"]
density = ["medium"]
#cellcoverage = ["50half", "50interleaved", "100"]
cellcoverage = ["100"]
#knownroutes = ["0", "25", "50", "75", "100"]
knownroutes = ["50"]
#workload = ["1", "2", "3", "4", "5"]
workload = ["1"]
#algorithm = ["random2", "gtt", "hvc", "gcf", "gcf2"]
algorithm = ["gtt"]
EXPERIMENT_TAG = "urban-medium-10moto90car-no-infra-moto-timestemp-finep" 

#for run in xrange(1,52):
for run in range(1,101): #30
    #if(run==16):
        #continue    
    for a in scenario:
        for b in density:
            for c in cellcoverage:
                for f in knownroutes:
                    for d in workload:
                        for e in algorithm:
                            os.system('./build/scratch/olvanets/olvanets --run=' + str(run) + ' --tracePath=' + path + ' --scenario=' + a + ' --density=' + b + ' --cellcoverage=' + c + ' --knownroutes=' + f + ' --workload=' + d + ' --algorithm=' + e + ' --clientType=moto'+ " --experimentTag=" + EXPERIMENT_TAG)

#os.system('exit') #sair do modo shell

