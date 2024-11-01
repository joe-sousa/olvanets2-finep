# Alisson Barbosa de Souza - GREat
# Federal University of Ceará (UFC)
# Abril de 2019
BEGIN {
    FS=":|;";
    number = 0;
} 
{ 
    teste = $0;
    printf("%s %s\n", number, teste);
    number = number + 20;
}
END { 
    number = 0;
} 
