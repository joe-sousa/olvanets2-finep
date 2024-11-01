# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Março de 2021

function sum_deviation_squared(values, mean_pt){
	sum2 = 0;
	for(i=0; i < length(values); i++){
		sum2 = sum2 + (values[i] - mean_pt)**2;
    }
	return sum2;
}

function std_deviation(sum_dev_sqr, occurences){
   return sqrt(sum_dev_sqr/occurences);
}

function confidence_95(sum, occurences, values, mean_pt){
    sum_dev_sqr = sum_deviation_squared(values, mean_pt);
	std_dev = std_deviation(sum_dev_sqr, occurences)
	return 1.96 * std_dev / sqrt(occurences)
}

BEGIN {
    FS=":|;";
    numberOfLines = 0; #numero de linhas do arquivo
    sum = 0; #soma das variacoes
    mean = 0; #media das variacoes
    conf_int = 0; #intervalo de confianca
    sumTime = 0; #soma dos tempos do algoritmo
    meanTime = 0; #media dos tempos do algoritmo
    conf_intTime = 0; #intervalo de confianca dos tempos do algoritmo
} 
{     
    variation = $8*-1; #variacao de tempo em relacao a baseline            
    values[numberOfLines] = variation; #armazena cada valor num array
    sum = sum + variation;
    time = $13; #tempo do algoritmo
    valuesTime[numberOfLines] = time; #armazena cada valor num array
    sumTime = sumTime + time;
    numberOfLines = numberOfLines + 1;
}
END {     
    mean = sum/numberOfLines;
    meanTime = sumTime/numberOfLines;
    conf_int = confidence_95(sum, numberOfLines, values, mean);
    conf_intTime = confidence_95(sumTime, numberOfLines, valuesTime, meanTime);  
    printf("%s %s %s %s\n", meanTime, conf_intTime, mean, conf_int);
} 
