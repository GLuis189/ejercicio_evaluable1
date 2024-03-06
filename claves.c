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

    // Atributos de la cola
    struct mq_attr attr;
    attr.mq_maxmsg = 1;
    if (sizeof(struct peticion) > sizeof(struct respuesta)){
        attr.mq_msgsize = sizeof(struct peticion);
    }
    else{
        attr.mq_msgsize = sizeof(struct respuesta);
    }
    char queuename[MAX];
    sprintf(queuename,  "/Cola-%d", getpid());
	q_cliente = mq_open(queuename, O_CREAT|O_RDONLY, 0700, &attr);
    if (q_cliente == -1) {
		perror("mq_open cliente");
		return -1;
	}
    q_servidor = mq_open(SERVIDOR, O_WRONLY);
    if (q_servidor == -1){
		mq_close(q_cliente);
		perror("mq_open servitor");
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
    if (mq_receive(q_cliente, (char *) &r, sizeof(r), 0) < 0){
		perror("mq_recv");
		r.status = -1;
	}
    mq_close(q_servidor);
    mq_close(q_cliente);
    mq_unlink(queuename);
    return r.result;
}