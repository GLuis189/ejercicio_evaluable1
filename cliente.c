// Código del cliente - nada de colas y funciones como si fuesen locales

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"


int main(){
    if (init() == 0){
        printf("Inicializado cliente\n");
    };
    // provar el set_value asi int set_value(int key, char *value1, int N_value2, double *V_value_2). 
    if(set_value(1, "hola", 3, (double[]){1.0, 2.0, 3.0}) == 0){
        printf("Seteado valor\n");
    }
    return 0;
}