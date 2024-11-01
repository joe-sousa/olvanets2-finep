# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Abril de 2019
BEGIN {
    FS=":|;";
    numberOfLines = 0; #numero de linhas do arquivo
    numberOfLocalTasks = 0; #numero de tasks executadas localmente
    numberOfOffloadedTasks = 0; #numero de tasks executadas remotamente
    numberOfRecoveredTasks = 0; #numero de tasks recuperadas
    numberOfTotalTasks = 0; #numero de falhas
} 
{ 
    numberOfLines = numberOfLines + 1;
    local = $9;
    offloaded = $10;
    recovered = $11;
    total = $12;
    
    numberOfLocalTasks = numberOfLocalTasks + local;
    numberOfOffloadedTasks = numberOfOffloadedTasks + offloaded;
    numberOfRecoveredTasks = numberOfRecoveredTasks + recovered;
    numberOfTotalTasks = numberOfTotalTasks + total;
}
END { 
    localRate = (numberOfLocalTasks*100)/numberOfTotalTasks;
    offloadedRate = (numberOfOffloadedTasks*100)/numberOfTotalTasks;
    recoveredRate = (numberOfRecoveredTasks*100)/numberOfTotalTasks;    
    printf("%s\t%s\t%s\n", localRate, offloadedRate, recoveredRate);
} 
