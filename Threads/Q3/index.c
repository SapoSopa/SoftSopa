#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue_safe.h"
#include "result_safe.h"

// Parâmetros de teste;
#define N_CLIENTES 5
#define MAX_OPERACOES 30

// Operações disponíveis;
enum operat {
    saque,
    deposito,
    consulta
};

// Informações do banco;
typedef struct bancoInfo {
    int numClientes;                        // Número de clientes;
    int numContas;                          // Número de contas;
} bancoInfo;

// Informações do cliente;
typedef struct clientInfo {
    int myId;                               // Identificador do cliente;
    int accountId;                          // Identificador da conta;
} clientInfo;

// Fila de operações requisitadas pelos clientes;
opQueue *queue;

// Comunicação com o cliente;
cliente *clientes_com[N_CLIENTES];

//Funções das threads;
void *banco_th(void *args);                 // Banco;
void *cliente_th(void *args);               // Cliente;

int main()
{
    // Inicializar a fila de operações e os clientes;
    initQueue(&queue);
    for (int i = 0; i < N_CLIENTES; i++) {
        initCliente(&clientes_com[i], i);
    }

    // Threads do banco e dos clientes;
    pthread_t banco;
    pthread_t clientes[N_CLIENTES];    

    // Inicializar e criar a thread do banco;
    bancoInfo *b_info = (bancoInfo *)malloc(sizeof(bancoInfo));
    b_info->numClientes = N_CLIENTES;
    b_info->numContas = N_CLIENTES;
    pthread_create(&banco, NULL, &banco_th, (void *)b_info);

    // Inicializar e criar as threads dos clientes;
    for (int i = 0; i < N_CLIENTES; i++) 
     {
        clientInfo *info = (clientInfo *)malloc(sizeof(clientInfo));
        info->myId = i;
        info->accountId = i;
        pthread_create(&(clientes[i]), NULL, &cliente_th, (void *)info);
    }

    // Aguardar as threads terminarem;
    pthread_join(banco, NULL);
    for (int i = 0; i < N_CLIENTES; i++) {
        pthread_join(clientes[i], NULL);
    }

    //Encerrar o programa;
    pthread_exit(NULL);
}

// Código do banco
void *banco_th(void *args)
{
    // Variáveis auxiliares;
    bancoInfo * info = (bancoInfo *)args;       // Informações do banco;
    float valores[info->numContas];             // Saldo das contas;
    for (int i = 0; i < info->numContas; i++)
    {
        valores[i] = 0.0f;
    }

    // Processar as operações dos clientes;
    for (int i = 0; i < info->numClientes * MAX_OPERACOES; i++)
    {
        // Próxima operação a ser processada;
        operacao next;
        resultado res;;
        strcpy(res.msg, "");

        // Esperar por uma operação;
        next = getOp_wait(queue);

        // Processar a operação;
        if (next.op == consulta)
        {
            res.status = ok;
            sprintf(res.msg, "Cliente %d (%d): Seu saldo é R$%.2f.\n", next.accountId, next.clientId, valores[next.accountId]);
        }
        else if (next.value < 0)
        {
            res.status = fail;
            sprintf(res.msg, "Cliente %d (%d): Saque ou deposito negativos são invalidos.\n", next.accountId, next.clientId);
        }
        else if (next.op == deposito)
        {
            // Atualizar o saldo da conta;
            valores[next.accountId] += next.value;

            res.status = ok;
            sprintf(res.msg, "Cliente %d (%d): Deposito de R$%.2f realizado com sucesso, seu novo saldo é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
        }
        else if (next.op == saque)
        {
            // Verificar se há saldo suficiente na conta;
            if (valores[next.accountId] - next.value >= 0)
            {
                // Atualizar o saldo da conta;
                valores[next.accountId] -= next.value;

                res.status = ok;
                sprintf(res.msg, "Cliente %d (%d): Saque de R$%.2f realizado com sucesso, seu novo saldo é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
            }
            else {
                res.status = fail;
                sprintf(res.msg, "Cliente %d (%d): Saque de R$%.2f invalido, seu saldo atual é R$%.2f.\n", 
                                next.accountId, next.clientId, next.value, valores[next.accountId]);
            }
        }

        // Enviar o resultado para o cliente;
        sendResult(clientes_com[next.clientId], res);
    }

    // Encerrar a thread;
    pthread_exit(NULL);
}

// Código do cliente;
void *cliente_th(void *args)
{
    // Variável auxiliar;
    clientInfo *myInfo = (clientInfo *)args;     // Informações do cliente;

    // Realizar as operações;
    for (int i = 0; i < MAX_OPERACOES; i++)
    {
        // Próxima operação a ser realizada;
        operacao myOperation;
        resultado res;
        myOperation.clientId = myInfo->myId;
        myOperation.accountId = myInfo->accountId;
        myOperation.op = i % 3;
        myOperation.value = i * 3;

        // Enviar a operação para o banco e aguardar o resultado;
        setWaitingResult(clientes_com[myInfo->myId]);
        sendOp(queue, myOperation);
        res = getWaitingResult(clientes_com[myInfo->myId]);

        // print do resultado obtido
        printf("%s", res.msg);
    }

    // Liberar a memória e encerrar a thread;
    free(myInfo);
    pthread_exit(NULL);
}