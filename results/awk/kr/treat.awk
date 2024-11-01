# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Maio de 2020

BEGIN {
    FS=":|;|,";    
    numberOfLines = 1; #quantidade de linhas do arquivo         
} 
{   
    while($14!=numberOfLines){
        printf(";;;;;;;;;;;;%s\n", numberOfLines);
        numberOfLines++;
    }
    printf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n", $1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14);            
    numberOfLines++;
}
END {
    if(occurences == 0) {

    } else {

    }
    #printf("%s\n", highest_pt);
    #printf("%s\n", algo);
    #printf("%s %s %s\n", mean_pt, lowest_pt, highest_pt);    
} 
