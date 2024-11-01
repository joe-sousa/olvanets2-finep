# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Novembro de 2020

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
} 
{   
    #if($6==0){  
        variation = $8*-1; #variacao de tempo em relacao a baseline            
        values[numberOfLines] = variation; #armazena cada valor num array
        sum = sum + variation;
        numberOfLines = numberOfLines + 1;
    #}
}
END {     
    mean = sum/numberOfLines;
    conf_int = confidence_95(sum, numberOfLines, values, mean); 
    printf("%s %s\n", mean, conf_int);
} 
