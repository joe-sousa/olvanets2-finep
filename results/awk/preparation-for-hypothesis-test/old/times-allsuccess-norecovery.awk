# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Maio de 2020

BEGIN {
    FS=":|;|,";    
    #occurences = 0; #quantidade de ocorrencias de sucesso
    printf("random2;cloudnet2;hvc;gcf\n");        
} 
{ 
    random2_success = $1;random2_recoveries = $6;cloudnet2_success = $7;cloudnet2_recoveries = $12;hvc_success = $13;hvc_recoveries = $18;gcf_success = $19;gcf_recoveries = $24;
    random2_time = "";cloudnet2_time = "";hvc_time = "";gcf_time = "";
    if((random2_success == "T")&&(cloudnet2_success == "T")&&(hvc_success == "T")&&(gcf_success == "T")){
        gsub(/\./, ",",$3);gsub(/\./, ",",$9);gsub(/\./, ",",$15);gsub(/\./, ",",$21);  
        if(random2_recoveries == 0){    
            random2_time = $3;                        
        }
        if(cloudnet2_recoveries == 0){                
            cloudnet2_time = $9;           
        }
         if(hvc_recoveries == 0){                
            hvc_time = $15;           
        }
        if(gcf_recoveries == 0){                
            gcf_time = $21;           
        }
    }      
    printf("%s;%s;%s;%s\n", random2_time, cloudnet2_time, hvc_time, gcf_time);    
}
END {
    if(occurences == 0) {

    } else {

    }
    #printf("%s\n", highest_pt);
    #printf("%s\n", algo);
    #printf("%s %s %s\n", mean_pt, lowest_pt, highest_pt);    
} 
