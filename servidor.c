// Código del servidor 
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "claves.h"

#define MAX_LONGITUD 256
#define MAX_ELEMENTOS 32

pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t  q_servidor;

#define MAX_TUPLAS 100 // Define el máximo número de tuplas que se pueden almacenar

// Estructura para representar una tupla <clave-valor1-valor2>
typedef struct {
    int clave;
    char valor1[256];
    int N;
    double *vector;
} Tupla;

// Definición del vector global para almacenar las tuplas
Tupla tuplas[MAX_TUPLAS];
int numTuplas = 0; // Variable global para almacenar el número actual de tuplas

// Definición de la variable global para el nombre del archivo
char filename[FILENAME_MAX];

// Función para escribir las tuplas en un archivo de texto
void escribirTuplas() {
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("Error al abrir el archivo para escribir.\n");
        return;
    }

    for (int i = 0; i < numTuplas; i++) {
        fprintf(fp, "%d,%s", tuplas[i].clave, tuplas[i].valor1);
        for (int j = 0; j < tuplas[i].N; j++) {
            fprintf(fp, ",%.2f", tuplas[i].vector[j]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}


// Función para leer las tuplas desde un archivo de texto
void leerTuplas() {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error al abrir el archivo para leer.\n");
        return;
    }

    while (!feof(fp) && numTuplas < MAX_TUPLAS) {
        Tupla t;
        int result = fscanf(fp, "%d,%[^,],%d", &t.clave, t.valor1, &t.N);
        if (result == EOF) {
            break;
        }
        t.vector = (double *)malloc(t.N * sizeof(double));
        for (int i = 0; i < t.N; i++) {
            fscanf(fp, ",%lf", &t.vector[i]);
        }
        tuplas[numTuplas++] = t; // Almacena la tupla en el vector global
    }

    fclose(fp);
}

int r_init(){   
    printf("Inicializado\n");
    strcpy(filename, "datos.txt");
    remove(filename);
    escribirTuplas();
    leerTuplas();
    return 0;
}

int keys[MAX_TUPLAS]; // Array para almacenar las claves

int r_set_value(int key, char *value1, int N_value, double *V_value){
    printf("Set value\n");

    // Buscar si la clave ya existe
    for (int i = 0; i < numTuplas; i++) {
        if (keys[i] == key) {
            // Si la clave ya existe, imprimir un mensaje de error y devolver un código de error
            printf("Error set_value: Ya existe una tupla con la clave %d\n", key);
            return -1;
        }
    }

    // Si la clave no existe, añadir una nueva tupla
    Tupla t;
    t.clave = key;
    strcpy(t.valor1, value1);
    t.N = N_value;
    t.vector = (double *)malloc(N_value * sizeof(double));
    for (int i = 0; i < N_value; i++) {
        t.vector[i] = V_value[i];
    }
    tuplas[numTuplas] = t;
    keys[numTuplas] = key; // Añadir la clave al array de claves
    numTuplas++;
    escribirTuplas();
    return 0;
}

int r_get_value(int key, char *value1, int *N_value, double *V_value){
    printf("Get value\n");
    for (int i = 0; i < numTuplas; i++) {
        if (keys[i] == key) {
            strcpy(value1, tuplas[i].valor1);
            *N_value = tuplas[i].N;
            for (int j = 0; j < tuplas[i].N; j++) {
                V_value[j] = tuplas[i].vector[j];
            }
            return 0;
        }
    }
    printf("Error get_value: No existe una tupla con la clave %d\n", key);
    return -1;
}

int r_modify_value(int key, char *value1, int N_value, double *V_value){
    printf("Modify value\n");
    for (int i = 0; i < numTuplas; i++) {
        if (keys[i] == key) {
            strcpy(tuplas[i].valor1, value1);
            tuplas[i].N = N_value;
            for (int j = 0; j < N_value; j++) {
                tuplas[i].vector[j] = V_value[j];
            }
            escribirTuplas();
            return 0;
        }
    }
    printf("Error modify_value: No existe una tupla con la clave %d\n", key);
    return -1;
}

int r_delete_key(int key){
    printf("Delete key\n");
    for (int i = 0; i < numTuplas; i++) {
        if (keys[i] == key) {
            for (int j = i; j < numTuplas - 1; j++) {
                tuplas[j] = tuplas[j + 1];
                keys[j] = keys[j + 1];
            }
            numTuplas--;
            escribirTuplas();
            return 0;
        }
    }
    printf("Error delete_key: No existe una tupla con la clave %d\n", key);
    return -1;
}

int r_exist(int key){
    printf("Exist\n");
    for (int i = 0; i < numTuplas; i++) {
        if (keys[i] == key) {
            printf("si existe");
            return 0;
        }
    }
    return -1;
}

void tratar_peticion(struct peticion *p){
    struct respuesta r;
    switch (p->op){
        case INIT:
            r.result = r_init();
            if (r.result == 0){
                r.status = 0;
            }
            break;
        case SET:
            r.result = r_set_value(p->key, p->value1, p->N_value, &p->V_value);
            if (r.result == 0){
                r.status = 0;
            }
            break;
        case GET:
            r.result = r_get_value(p->key, p->value1, &p->N_value, &p->V_value);
            break;
        case MODIFY:
            r.result = r_modify_value(p->key, p->value1, p->N_value, p->V_value);
            break;
        case DELETE:
            r.result = r_delete_key(p->key);
            break;
        case EXIST:
            r.result = r_exist(p->key);
            break;
        default:
            r.result = -1;
            break;
    }


    mqd_t q_cliente = mq_open(p->q_name, O_WRONLY);

    
    if (q_cliente == -1){
        perror("No se puede abrir la cola del cliente");
        mq_close(q_servidor);
        mq_unlink(SERVIDOR);
    }
    else {
         printf("Cola del cliente abierta correctamente.\n");
        
        if (mq_send(q_cliente, (const char *) &r, sizeof(struct respuesta), 0) <0) {
            perror("mq_send");
            mq_close(q_servidor);
            mq_unlink(SERVIDOR);
            mq_close(q_cliente);
        }
    }
    free(p);
    mq_close(q_cliente);
    pthread_exit(0);
    
}

int main(){
    struct peticion *p;
    struct mq_attr attr;
    pthread_attr_t t_attr;
    pthread_t thread;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion);

    printf("Sizeof struct peticion servidor: %lu\n", sizeof(struct peticion));

    q_servidor = mq_open(SERVIDOR, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_servidor == -1){
        perror("mq_open servidor");
        return -1;
    }

    pthread_mutex_init(&mutex_mensaje, NULL);
    pthread_cond_init(&cond_mensaje, NULL);
    pthread_attr_init(&t_attr);

    pthread_attr_setdetachstate(&t_attr, PTHREAD_CREATE_DETACHED);

    while (1){
        p = malloc(sizeof(struct peticion));
        if (mq_receive(q_servidor, (char *) p, sizeof(struct peticion) + 10, 0) < 0){
            perror("mq_receive");
            free(p);
            return -1;
        }
        if (pthread_create(&thread, &t_attr, (void *) tratar_peticion, (void *) p) != 0){
            perror("pthread_create");
            free(p);
        }
    }

    mq_close(q_servidor);
    mq_unlink(SERVIDOR);
    return 0;
}
