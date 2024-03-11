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
        fprintf(fp, "%d,%s,%d", tuplas[i].clave, tuplas[i].valor1, tuplas[i].N);
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

int r_set_value(int key, char *value1, int N_value, double *V_value){
    printf("Set value\n");
    Tupla t;
    t.clave = key;
    strcpy(t.valor1, value1);
    t.N = N_value;
    t.vector = (double *)malloc(N_value * sizeof(double));
    for (int i = 0; i < N_value; i++) {
        t.vector[i] = V_value[i];
    }
    tuplas[numTuplas++] = t;
    escribirTuplas();
    return 0;
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
            printf("Set valuasdasde\n");
            r.result = r_set_value(p->key, p->value1, p->N_value, &p->V_value);
            if (r.result == 0){
                r.status = 0;
            }
            break;
        // case GET:
        //     r->result = get_value(p->key, p->value1, &p->N_value, &p->V_value);
        //     break;
        // case MODIFY:
        //     r->result = modify_value(p->key, p->value1, p->N_value, p->V_value);
        //     break;
        // case DELETE:
        //     r->result = delete_key(p->key);
        //     break;
        // case EXIST:
        //     r->result = exist(p->key);
        //     break;
        // default:
        //     r->result = -1;
        //     break;
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
    pthread_exit(0);
    mq_close(q_cliente);
}

int main(){
    struct peticion mess;
    struct mq_attr attr;

    pthread_attr_t t_attr;
    pthread_t thread;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion);

    //printf("Sizeof struct respuesta servidor: %lu\n", sizeof(struct respuesta));
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
        
        if (mq_receive(q_servidor, (char *) &mess, sizeof(struct peticion) + 10, 0) < 0){
            perror("mq_receive");
            return -1;
        }
        if (pthread_create(&thread, &t_attr, (void *) tratar_peticion, (void *) &mess) == 0){
            pthread_mutex_lock(&mutex_mensaje);
			while (mensaje_no_copiado)
				pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
			mensaje_no_copiado = true;
			pthread_mutex_unlock(&mutex_mensaje);
        }
    }

    mq_close(q_servidor);
    mq_unlink(SERVIDOR);
    return 0;
}


/*pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t  q_servidor;

void tratar_peticion(struct peticion *p){
    struct respuesta r;
    switch (p->op){
        case INIT:
            r.result = r_init(); // Aquí deberías poner el resultado de la operación INIT
            break;
    }
    mqd_t q_cliente = mq_open(p->q_name, O_WRONLY);
    if (q_cliente == -1){
        perror("No se puede abrir la cola del cliente");
        mq_close(q_servidor);
        mq_unlink(SERVIDOR);
    }
    else {
        if (mq_send(q_cliente, (const char *) &r, sizeof(struct respuesta), 0) <0) {
            perror("mq_send");
            mq_close(q_servidor);
            mq_unlink(SERVIDOR);
            mq_close(q_cliente);
        }
    }
    free(p);
    pthread_exit(0);
}

int r_init(){
    printf("Inicializado\n");
    return 0;
}

int main(){
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
        struct peticion *mess = malloc(sizeof(struct peticion));
        if (mess == NULL) {
            perror("malloc");
            return -1;
        }
        if (mq_receive(q_servidor, (char *) mess, sizeof(*mess) + 10, 0) < 0){
            perror("mq_receive");
            return -1;
        }
        if (pthread_create(&thread, &t_attr, (void *) tratar_peticion, (void *) mess) == 0){
            pthread_mutex_lock(&mutex_mensaje);
            while (mensaje_no_copiado)
                pthread_cond_wait(&cond_mensaje, &mutex_mensaje);
            mensaje_no_copiado = true;
            pthread_mutex_unlock(&mutex_mensaje);
        }
    }
    return 0;
}*/
