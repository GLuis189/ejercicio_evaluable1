// Código del cliente - nada de colas y funciones como si fuesen locales

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"


int main(){
    if (init() == 0){
        printf("Inicializado cliente\n");
    };
    return 0;
}