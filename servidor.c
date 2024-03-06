// Código del servidor 
#include <mqueue.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "claves.h"


pthread_mutex_t mutex_mensaje;
int mensaje_no_copiado = true;
pthread_cond_t cond_mensaje;
mqd_t  q_servidor;

void tratar_peticion(struct peticion *p){
    struct respuesta r;
    switch (p->op){
        case INIT:
            r.result = init();
            break;
        // case SET:
        //     r->result = set_value(p->key, p->value1, p->N_value, p->V_value);
        //     break;
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
        if (mq_send(q_cliente, (const char *) &r, sizeof(struct respuesta), 0) <0) {
            perror("mq_send");
            mq_close(q_servidor);
            mq_unlink(SERVIDOR);
            mq_close(q_cliente);
        }
    }
    pthread_exit(0);
}

int init(){
    if (init() == 0){
        printf("Inicializado\n");
    };
    return 0;
}


int main(){
    struct peticion mess;
    struct mq_attr attr;

    pthread_attr_t t_attr;
    pthread_t thread;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(struct peticion);

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
        if (mq_receive(q_servidor, (char *) &mess, sizeof(mess), 0) < 0){
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
    return 0;
}