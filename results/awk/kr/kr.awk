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
    sum0 = 0;
    sum25 = 0;
    sum50 = 0;
    sum75 = 0;
    sum100 = 0; #soma das variacoes
    mean0 = 0;
    mean25 = 0;
    mean50 = 0;
    mean75 = 0;
    mean100 = 0; #media das variacoes
    conf_int0 = 0;
    conf_int25 = 0;
    conf_int50 = 0;
    conf_int75 = 0;
    conf_int100 = 0; #intervalo de confianca

    rcvSrvSum0 = 0; rcvSrvSum25 = 0; rcvSrvSum50 = 0; rcvSrvSum75 = 0; rcvSrvSum100 = 0; #soma de recuperacoes por servidor
    rcvTskSum0 = 0; rcvTskSum25 = 0; rcvTskSum50 = 0; rcvTskSum75 = 0; rcvTskSum100 = 0; #soma de recuperacoes por tarefa
} 
{
    dev0 = $2*-1;
    dev25 = $5*-1;
    dev50 = $8*-1;
    dev75 = $11*-1;
    dev100 = $14*-1; #variacao de tempo em relacao a baseline            
    values0[numberOfLines] = dev0; values25[numberOfLines] = dev25; values50[numberOfLines] = dev50; values75[numberOfLines] = dev75; values100[numberOfLines] = dev100; #armazena cada valor num array
    sum0 = sum0 + dev0;
    sum25 = sum25 + dev25;
    sum50 = sum50 + dev50;
    sum75 = sum75 + dev75;
    sum100 = sum100 + dev100; 
    
    rcvSrvSum0 = rcvSrvSum0 + $1; rcvSrvSum25 = rcvSrvSum25 + $4; rcvSrvSum50 = rcvSrvSum50 + $7; rcvSrvSum75 = rcvSrvSum75 + $10; rcvSrvSum100 = rcvSrvSum100 + $13;    
    rcvTskSum0 = rcvTskSum0 + $3; rcvTskSum25 = rcvTskSum25 + $6; rcvTskSum50 = rcvTskSum50 + $9; rcvTskSum75 = rcvTskSum75 + $12; rcvTskSum100 = rcvTskSum100 + $15;

    numberOfLines = numberOfLines + 1;
}
END {
    mean0 = sum0/numberOfLines;
    mean25 = sum25/numberOfLines;
    mean50 = sum50/numberOfLines;
    mean75 = sum75/numberOfLines;
    mean100 = sum100/numberOfLines;
    conf_int0 = confidence_95(sum0, numberOfLines, values0, mean0); conf_int25 = confidence_95(sum25, numberOfLines, values25, mean25); conf_int50 = confidence_95(sum50, numberOfLines, values50, mean50); conf_int75 = confidence_95(sum75, numberOfLines, values75, mean75); conf_int100 = confidence_95(sum100, numberOfLines, values100, mean100);

    rcvSrvDev0 = ((rcvSrvSum0/rcvSrvSum0)-1)*100*-1; rcvSrvDev25 = ((rcvSrvSum25/rcvSrvSum0)-1)*100*-1; rcvSrvDev50 = ((rcvSrvSum50/rcvSrvSum0)-1)*100*-1; rcvSrvDev75 = ((rcvSrvSum75/rcvSrvSum0)-1)*100*-1; rcvSrvDev100 = ((rcvSrvSum100/rcvSrvSum0)-1)*100*-1;#variacao percentual das recuperacoes de servidores; baseline: 0% de kr
    rcvTskDev0 = ((rcvTskSum0/rcvTskSum0)-1)*100*-1; rcvTskDev25 = ((rcvTskSum25/rcvTskSum0)-1)*100*-1; rcvTskDev50 = ((rcvTskSum50/rcvTskSum0)-1)*100*-1; rcvTskDev75 = ((rcvTskSum75/rcvTskSum0)-1)*100*-1; rcvTskDev100 = ((rcvTskSum100/rcvTskSum0)-1)*100*-1;#variacao percentual das recuperacoes de servidores; baseline: 0% de kr    
      
    printf("%s\t%s\t%s\t%s\n", mean0, conf_int0, rcvSrvDev0, rcvTskDev0);
    printf("%s\t%s\t%s\t%s\n", mean25, conf_int25, rcvSrvDev25, rcvTskDev25);
    printf("%s\t%s\t%s\t%s\n", mean50, conf_int50, rcvSrvDev50, rcvTskDev50);
    printf("%s\t%s\t%s\t%s\n", mean75, conf_int75, rcvSrvDev75, rcvTskDev75);
    printf("%s\t%s\t%s\t%s\n", mean100, conf_int100, rcvSrvDev100, rcvTskDev100);
} 
