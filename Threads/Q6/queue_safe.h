#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct node node;
typedef struct queue fila;
typedef struct process process;

// Struct do processo;
struct process {
        char *name;                             // Nome do processo;
        pthread_cond_t con;                     // Condição para acordar o processo;
        pthread_mutex_t m_exec, m_end;          // Mutex para controlar a execução e o fim do processo;
        pthread_t id;                           // ID da thread do processo;
        int flag_exec, flag_end;                // Flags para controlar a execução e o fim do processo;
        int exec_qtd;                           // Quantidade de vezes que o processo deve ser executado;
};

// Criar um processo especificando o nome e a quantidade de vezes que ele deve ser executado;
process* process_create(char *nome, int qtd)
{
        // Alocar memória para o processo e inicializar os campos;
        process *ptr = (process*) malloc(sizeof(process));
        ptr->name = nome;
        ptr->flag_end = 0;
        ptr->exec_qtd = qtd;

        // Inicializar os mutex e a condição;
        pthread_cond_init(&ptr->con, NULL);
        pthread_mutex_init(&ptr->m_exec, NULL);
        pthread_mutex_init(&ptr->m_end, NULL);

        return ptr;
}

// Nó da fila de processos;
struct node
{
        process *data;                          // Thread guardada no nó;
        node *next;                             // Ponteiro para o próximo nó;
};

// Fila de processos;
struct queue
{
        node *head;                             // Nó mais antigo;
        node *tail;                             // Nó mais recente;
        int lenght;                             // Tamanho da fila;
        pthread_mutex_t mutex;                  // Mutex para controlar a fila;
        pthread_cond_t cond;                    // Condição para acordar o escalonador;
};

// Inicializar a fila;
void initQueue(fila **q)
{
        // Alocar memória para a fila e inicializar os campos;
        fila *new_queue = (fila *)malloc(sizeof(fila));
        new_queue->head = NULL;
        new_queue->tail = NULL;
        new_queue->lenght = 0;

        // Inicializar o mutex e a condição;
        pthread_mutex_init(&new_queue->mutex, NULL);
        pthread_cond_init(&new_queue->cond, NULL);
        *q = new_queue;
}

// Adicionar uma nova operação atomicamente na fila;
void push(fila *q, process *dado)
{
        // Alocar memória para o nó e inicializar os campos;
        node *new_node;
        new_node = (node *)malloc(sizeof(node));
        new_node->next = NULL;
        new_node->data = dado;

        // Travar o mutex da fila para alterá-la;
        pthread_mutex_lock(&(q->mutex));

        // Adicionar o nó na fila;
        if (q->tail != NULL)
        {
                q->tail->next = new_node;
        }
        q->tail = new_node;
        
        // Se a fila estiver vazia, o nó é o primeiro;
        if (q->head == NULL)
        {
                q->head = new_node;
        }

        // Redimensionar a fila;
        q->lenght++;

        // Desbloquear o mutex da fila corretamente;
        pthread_cond_signal(&q->cond);
        pthread_mutex_unlock(&(q->mutex));
}

// Remover uma operação atomicamente da fila;
process *pop(fila *q)
{
        // Variável para armazenar o processo que será retirado da fila;
        process *value;

        // Travar o mutex da fila para alterá-la;
        pthread_mutex_lock(&(q->mutex));

        // Se a fila estiver vazia, retornar NULL e liberar o mutex;
        while (q->lenght == 0)
        {
                pthread_mutex_unlock(&(q->mutex));
                return NULL;
        }

        // Retirar o processo da fila e redimensioná-la;
        value = q->head->data;
        node *tmp_node = q->head;
        if (q->head == q->tail)
        {
                q->tail = NULL;
        }
        q->lenght--;
        q->head = q->head->next;
        free(tmp_node);

        // Desbloquear o mutex da fila;
        pthread_mutex_unlock(&(q->mutex));

        // Retornar o processo retirado da fila;
        return value;
}

#endif
