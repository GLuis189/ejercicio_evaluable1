// Funciones de la biblioteca
#include <mqueue.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "claves.h"

// peticion
// struct peticion{
//     int op;
//     int key;
//     char value1[MAX];
//     int N_value;
//     double V_value;
//     char q_name[MAX];
// };

// // respuesta = (valor. estado)
// struct respuesta{
//     int result;
//     char status;
// };


int init(){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    struct respuesta r;

    printf("Sizeof struct peticion: %lu\n", sizeof(struct peticion));
    printf("Sizeof struct respuesta: %lu\n", sizeof(struct respuesta));


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
    printf("mq_maxmsg: %ld, mq_msgsize: %ld\n", attr.mq_maxmsg, attr.mq_msgsize);

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
		r.status = -1;
	}	
    if (mq_receive(q_cliente, (char *) &r, sizeof(p), 0) < 0){
        perror("mq_recv");
        r.status = -1;
    }
    // char buffer[attr.mq_msgsize];

    // if (mq_receive(q_cliente, buffer, sizeof(buffer), 0) < 0){
    //     perror("mq_recv");
    //     r.status = -1;
    // }
    
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    return r.status;
}

int set_value(int key, char *value, int N_value, double *V_value){
    mqd_t q_servidor;
    mqd_t q_cliente;

    struct peticion p;
    struct respuesta r;

    p.op = SET;
    p.key = key;
    strcpy(p.value1, value);
    //p.N_value = N_value;
    //p.V_value = *V_value;

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
    printf("mq_maxmsg: %ld, mq_msgsize: %ld\n", attr.mq_maxmsg, attr.mq_msgsize);

    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
        mq_close(q_cliente);
        perror("mq_open 2");
        return -1;
    }
    printf("a1\n");
    strcpy(p.q_name, queuename);

    // Envio de la petición
    if (mq_send(q_servidor, (const char *)&p, sizeof(p), 0) < 0){
        perror("mq_send");
        r.status = -1;
    }

    printf("a2\n");

    if (mq_receive(q_cliente, (char *)&r, sizeof(p), 0) < 0){
        perror("mq_recv");
        r.status = -1;
    }
    printf("a3\n");

    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    return r.status;
}