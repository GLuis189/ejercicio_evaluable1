// Funciones de la biblioteca
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "mensajes.h"
#include "claves.h"


int init(){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;

    printf("Sizeof struct peticion: %lu\n", sizeof(struct peticion));
    printf("Sizeof struct respuesta: %lu\n", sizeof(int));


    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje

    sprintf(queuename,  "/Cola-%d", getpid());
	q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
		perror("mq_open 1");
		return -1;
	}

    mq_getattr(q_cliente, &attr);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
		mq_close(q_cliente);
		perror("mq_open 2");
		return -1;
	}

    // Realizar la petición
    p.op = INIT;
	strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
		perror("mq_send");
		return -1;
	}	
    if (mq_receive(q_cliente, (char *) &r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }
    // char buffer[attr.mq_msgsize];

    // if (mq_receive(q_cliente, buffer, sizeof(buffer), 0) < 0){
    //     perror("mq_recv");
    //     r.status = -1;
    // }
    
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    return r;
}

int set_value(int key, char *value, int N_value, double *V_value){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;
    if(N_value > 32 || N_value < 0){
        return -1;
    }
    // if(sizeof(V_value)!= N_value){
    //     return -1;
    // }
    p.op = SET;
    p.key = key;
    strcpy(p.value1, value);
    p.N_value = N_value;
    memcpy(p.V_value, V_value, sizeof(double) * N_value);

    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje

    sprintf(queuename,  "/Cola-%d", getpid());
    q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -1;
    }

    mq_getattr(q_cliente, &attr);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        return -1;
    }

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    // printf("Status: %d\n", r.status);
    return r;
}

int get_value(int key, char *value, int *N_value, double *V_value){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;

    p.op = GET;
    p.key = key;

    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje

    sprintf(queuename,  "/Cola-%d", getpid());
    q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -1;
    }

    mq_getattr(q_cliente, &attr);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        return -1;
    }

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }

    if(r == 0){
        strcpy(value, p.value1);
        *N_value = p.N_value;
        memcpy(V_value, p.V_value, sizeof(double) * p.N_value);
    }

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    // printf("Status: %d\n", r.status);
    return r;
}

int modify_value(int key, char *value, int N_value, double *V_value){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;

    p.op = MODIFY;
    p.key = key;
    strcpy(p.value1, value);
    p.N_value = N_value;
    memcpy(p.V_value, V_value, sizeof(double) * N_value);

    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje
    
    sprintf(queuename,  "/Cola-%d", getpid());
    q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -1;
    }

    mq_getattr(q_cliente, &attr);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        return -1;
    }

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);

    // printf("Status: %d\n", r);

    return r;
}

int delete_key(int key){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;

    p.op = DELETE;
    p.key = key;

    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje

    sprintf(queuename,  "/Cola-%d", getpid());
    q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -1;
    }

    mq_getattr(q_cliente, &attr);
    
    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        return -1;
    }

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);

    // printf("Status: %d\n", r);

    return r;
}

int exist(int key){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    int r;

    p.op = EXIST;
    p.key = key;

    // Atributos de la cola
    struct mq_attr attr;
    char queuename[MAX];
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion); //Tamaño del mensaje

    sprintf(queuename,  "/Cola-%d", getpid());
    q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
        perror("mq_open 1");
        return -1;
    }

    mq_getattr(q_cliente, &attr);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        return -1;
    }

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        return -1;
    }

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);

    // printf("Status: %d\n", r);

    return r;
}