// Cabecera para las funciones de claves.c
#ifndef CLAVES_H
#define CLAVES_H

#define MAX	256

#define INIT	0
#define SET     1
#define GET     2
#define MODIFY  3
#define DELETE  4
#define EXIST   5

#define SERVIDOR "/S-100472006"

// peticion
struct peticion{
    int op;
    int key;
    char value1[MAX];
    int N_value;
    double V_value[32];
    char q_name[MAX];
};

// respuesta = (valor. estado)
struct respuesta{
    int result;
    char status;
};

// Funciones
int init();
int set_value(int key, char *value, int N_value, double *V_value);
int get_value(int key, char *value, int *N_value, double *V_value);
int modify_value(int key, char *value, int N_value, double *V_value);
int delete_key(int key);
int exist(int key);

#endif