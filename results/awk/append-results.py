#!/usr/bin/python
import os

#append results
for a in ["append"]:
    for b in ["urban", "highway"]: #qual o cenario
        for c in ["low","medium","high"]: #densidade
            for d in ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10"]: #workload
                for e in ["abc"]: #algorithm
		            print "calculando resultados para "+b+"-"+c+"-w"+d+"-"+e+"\n"
		            os.system('cat /home/alisson/desktop/results/tese/abc/06042021/102-202/results/'+b+'-'+c+'-w'+d+'-'+e+'.tr >> ../traces/'+b+'-'+c+'-w'+d+'-'+e+'.tr')				
		            print "resultados calculados para "+b+"-"+c+"-w"+d+"-"+e
