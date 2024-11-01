# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Abril de 2019
BEGIN {
    FS=":|;";
    numberOfViolations = 0; #numero de violacoes da restricao de energia
    numberOfTotalTasks = 0;
} 
{ 
    numberOfLines = numberOfLines + 1;
    violations = $14;
    total = $12;
    
    numberOfViolations = numberOfViolations + violations;
    numberOfTotalTasks = numberOfTotalTasks + total;
}
END { 
    violationsRate = (numberOfViolations*100)/numberOfTotalTasks;   
    printf("%s\n", violationsRate);
} 
