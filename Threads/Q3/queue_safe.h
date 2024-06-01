#include <pthread.h>
#include <stdlib.h>

#ifndef QUEUE_H
#define QUEUE_H

// Definir as structs para a operação e a fila de operações;
typedef struct operacao operacao;
typedef struct node node;
typedef struct queue opQueue;

// Operação a ser realizada no banco;
struct operacao {
    int accountId;                          // ID da conta;
    int clientId;                           // ID do cliente
    int op;                                 // Operação que será realizada;
    float value;                            // Valor em caso de depósito ou saque;
};

// Nó da fila de operações;
struct node {
    operacao op;                            // Operação a ser realizada;
    node *next;                             // Próxima operação;
};

// Fila de operações a serem realizadas no banco;
struct queue {
    node *head;                             // Operação mais antiga;
    node *tail;                             // Operação mais recente;
    int lenght;                             // Tamanho da fila;
    pthread_mutex_t mutex;                  // Mutex para operações na fila;
    pthread_cond_t cond;                    // Condição para acordar quem estiver esperando uma operação;
};

// Inicializar a fila de operações;
void initQueue(opQueue **q)
{
    // Alocar memória para a fila;
    opQueue *new_queue = (opQueue *)malloc(sizeof(opQueue));
    new_queue->head = NULL;
    new_queue->tail = NULL;
    new_queue->lenght = 0;

    // Inicializar o mutex e a condição da fila;
    pthread_mutex_init(&new_queue->mutex, NULL);
    pthread_cond_init(&new_queue->cond, NULL);
    *q = new_queue;
}

// Enviar atomicamente uma operação para a fila;
void sendOp(opQueue *q, operacao op)
{
    // Alocar memória para a nova operação;
    node *new_node;
    new_node = (node *)malloc(sizeof(node));
    new_node->next = NULL;
    new_node->op.clientId = op.clientId;
    new_node->op.accountId = op.accountId;
    new_node->op.op = op.op;
    new_node->op.value = op.value;

    // Travar o mutex da fila e adicionar a operação;
    pthread_mutex_lock(&(q->mutex));
    if (q->tail != NULL) {
        q->tail->next = new_node;
    }
    q->tail = new_node;
    if (q->head == NULL) {
        q->head = new_node;
    }

    // Atualizar tamanho da fila;
    q->lenght++;

    // Liberar o mutex da fila e acordar quem estiver esperando;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&(q->mutex));
}

// Obter atomicamente uma operação da fila;
operacao getOp_wait(opQueue *q)
{
    // Operação a ser obtida da fila;
    operacao ret;

    // Travar o mutex da fila e esperar até que haja uma operação;
    pthread_mutex_lock(&(q->mutex));
    while (q->lenght == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    // Obter a operação da fila;
    ret.clientId = q->head->op.clientId;
    ret.accountId = q->head->op.accountId;
    ret.op = q->head->op.op;
    ret.value = q->head->op.value;
    node *tmp_node = q->head;
    if (q->head == q->tail) {
        q->tail = NULL;
    }

    // Atualizar tamanho da fila e liberar o nó da operação;
    q->lenght--;
    q->head = q->head->next;
    free(tmp_node);

    // Liberar o mutex da fila e retornar a operação;
    pthread_mutex_unlock(&(q->mutex));
    return ret;
}

#endif