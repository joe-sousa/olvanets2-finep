#!/usr/bin/python3
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
path = os.path.join(BASE_DIR, "mobilityTraces") + "/"

EXPERIMENT_TAG = "urban-medium-90moto10car-no-infra-moto-fixed-test"

scenario = "urban"
density = "medium"
cellcoverage = "100"
knownroutes = "50"
workload = "1"

# Apenas um run para analisar a mobilidade
run = 1

cmd = (
    "./build/scratch/olvanets/olvanets "
    f"--run={run} "
    f"--tracePath={path} "
    f"--scenario={scenario} "
    f"--density={density} "
    f"--cellcoverage={cellcoverage} "
    f"--knownroutes={knownroutes} "
    f"--workload={workload} "
    f"--clientType=moto "
    f"--experimentTag={EXPERIMENT_TAG} "
    "--analysisMode=1"
)

print(cmd)
os.system(cmd)