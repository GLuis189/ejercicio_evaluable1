// Código del cliente - nada de colas y funciones como si fuesen locales

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "claves.h"


int main(){
    // provar el set_value asi int set_value(int key, char *value1, int N_value2, double *V_value_2). 
    if(set_value(3, "hola c2", 33, (double[]){1.0, 2.0, 3.0}) == 0){
        printf("Seteado valor\n");
    }
    if(set_value(4, "hola2 c2", 4, (double[]){1.0, 2.0, 3.0, 4.0}) == 0){
        printf("Seteado valor 2\n");
    }
    return 0;
}