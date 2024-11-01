# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Maio de 2020

BEGIN {
    FS=":|;|,";    
    #occurences = 0; #quantidade de ocorrencias de sucesso
    printf("random2;hvc;gtt;abc\n");         
} 
{ 
    random2_gn = "";hvc_gn = "";gtt_gn = "";abc_gn = "";
    if(($1!="")&&($16!="")&&($31!="")&&($46!="")){    
        gsub(/\./, ",",$8);gsub(/\./, ",",$23);gsub(/\./, ",",$38);gsub(/\./, ",",$53);s
        random2_gn = $8;
        hvc_gn = $23;        
        gtt_gn = $38;
        abc_gn = $53;
    }
    printf("%s;%s;%s;%s\n", random2_gn, hvc_gn, gtt_gn, abc_gn);    
}
END {
    if(occurences == 0) {

    } else {

    }
    #printf("%s\n", highest_pt);
    #printf("%s\n", algo);
    #printf("%s %s %s\n", mean_pt, lowest_pt, highest_pt);    
} 
