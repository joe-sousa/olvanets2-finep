# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Maio de 2020

BEGIN {
    FS=":|;|,";    
    numberOfLines = 1; #quantidade de linhas do arquivo         
} 
{   
    if(($1!="")&&($15!="")&&($29!="")&&($43!="")&&($57!="")){
        if(($11>0)||($25>0)||($39>0)||($53>0)||($67>0)){
            printf("%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n", $6,$8,$11, $20,$22,$25, $34,$36,$39, $48,$50,$53, $62,$64,$67, $70);        
        }
    }                
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
