#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#ifndef RESULT_H
#define RESULT_H

// Status para o retorno da operação do cliente;
enum status
{
    ok,
    fail,
    waiting,
    off = -1
};

// Definir as structs para o resultado e o cliente;
typedef struct resultado resultado;
typedef struct cliente cliente;

// Resultado da operação do cliente;
struct resultado {
    int status;                             // Status final da operação;
    char msg[150];                          // Mensagem de retorno da operação;
};

// Cliente que solicita operações ao banco;
struct cliente {
    int id;                                 // ID do cliente;
    resultado result;                       // Resultado da última operação;
    pthread_mutex_t mu_result;              // Mutex do resultado do cliente;
    pthread_cond_t cond_result;             // Condição para acordar quem estiver esperando o resultado;
};

// Inicializar um cliente com um ID;
void initCliente(cliente **cl, int id)
{
    // Alocar memória para o cliente;
    cliente *new_client = (cliente *)malloc(sizeof(cliente));
    new_client->id = id;
    new_client->result.status = off;
    strcpy(new_client->result.msg, "");

    // Inicializar o mutex e a condição do resultado;
    pthread_mutex_init(&new_client->mu_result, NULL);
    pthread_cond_init(&new_client->cond_result, NULL);
    *cl = new_client;
}

// Enviar atomicamente o resultado da operação para o cliente;
void sendResult(cliente *cl, resultado r)
{
    // Travar o mutex do resultado;
    pthread_mutex_lock(&cl->mu_result);
    
    // Definir o status e a mensagem de retorno;
    cl->result.status = r.status;
    strcpy(cl->result.msg, r.msg);

    // Liberar o mutex do resultado corretamente;
    pthread_cond_signal(&cl->cond_result);
    pthread_mutex_unlock(&cl->mu_result);
}

// Definir atomicamente o resultado como em espera;
void setWaitingResult(cliente *cl)
{
    // Travar o mutex do resultado;
    pthread_mutex_lock(&cl->mu_result);
    
    // Definir o status e a mensagem de retorne;
    cl->result.status = waiting;
    strcpy(cl->result.msg, "");
}

// Obter o resultado da operação atômica do cliente;
resultado getWaitingResult(cliente *cl)
{
    // Definir o resultado temporário;
    resultado tmp_res;

    // Verificar se o resultado está em espera;
    while (cl->result.status == waiting)
    {
        pthread_cond_wait(&cl->cond_result, &cl->mu_result);
    }

    // Definir o status e a mensagem de retorno;
    tmp_res.status = cl->result.status;
    strcpy(tmp_res.msg, cl->result.msg);

    // Liberar o mutex do resultado;
    pthread_mutex_unlock(&cl->mu_result);

    // Retornar o resultado;
    return tmp_res;
}

#endif