#include "queue_safe.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

// Definir parâmetros de execução;
#define N 8                            // Número de processos que podem ser executados simultaneamente;
#define quatum_ms 1                     // Tempo de execução de cada processo em milissegundos;
#define MAX_REQUEST 300                 // Número máximo de requisições de processos;

// Lista de processos prontos para execução;
fila* lista_pronto;

// Funções das threads;
void *escalonador_func(void* args);     // Escalonar processos;
void *generic_func(void *args);         // Função genérica para execução de processos;
void *user(void* args);                 // Criar requisições de processos;

int main() 
{
    // Inicializar a lista de processos prontos;
    initQueue(&lista_pronto);

    // Inicializar o escalonador e o usuário;
    pthread_t escalonador_th;
    pthread_t user_th;

    // Criar as threads do escalonador e do usuário;
    pthread_create(&escalonador_th, NULL, escalonador_func, NULL);
    pthread_create(&user_th, NULL, user, NULL);

    // Esperar um tempo para iniciar o escalonador;
    int wait_s = 2;
    printf("Escalonador criado, vai iniciar em %ds\n", wait_s);
    sleep(wait_s);

    // Aguardar a conclusão das threads e encerrar o programa;
    pthread_join(escalonador_th, NULL);
    pthread_join(user_th, NULL);

    return 0;
}

// Código do escalonador de processos;
void *escalonador_func(void* args)
{
    // Variáveis auxiliares;
    int q = quatum_ms;                  // Tempo de execução de cada processo em milissegundos;
    int total_processed = 0;            // Número total de processos processados;

    // Inicializar as variáveis do escalonador;
    process** temp_pro = (process**) malloc(sizeof(process*) * N);
    for (int i = 0; i < N; i++) temp_pro[i] = NULL;

    // Executar o escalonador enquanto houverem processos a serem executados;
    while (total_processed < MAX_REQUEST)
    {
        // Verificar se há processos prontos para execução e aguardar se não houver;
        pthread_mutex_lock(&lista_pronto->mutex); // trava o mutex da queue se estiver livre
        while (lista_pronto->lenght == 0) {
            printf("Fila Vazio\n");
            fflush(stdout);             // Limpar o buffer de saída; 
            pthread_cond_wait(&lista_pronto->cond, &lista_pronto->mutex); // dorme se a queue estiver vazia
        }
        pthread_mutex_unlock(&lista_pronto->mutex); // libera o mutex

        // Listar os processos prontos e identificá-los;
        for (int i = 0; i < N; i++)
        {   
            temp_pro[i] = pop(lista_pronto);
            if (temp_pro[i] != NULL)
            { 
                printf("Executando processo %s por %d ms\n", temp_pro[i]->name, q);
                fflush(stdout);         // Limpar o buffer de saída;
            }
        }

        // Manter a organização do terminal;
        printf("\n------------------------------------\n\n");
        fflush(stdout);                 // Limpar o buffer de saída;

        // Evitar erros de execução;
        for (int i = 0; i < N; i++)
        {
            if (temp_pro[i] == NULL) continue;

            pthread_mutex_lock(&temp_pro[i]->m_exec);
            temp_pro[i]->flag_exec = 0;
            pthread_cond_signal(&temp_pro[i]->con);
            pthread_mutex_unlock(&temp_pro[i]->m_exec);
        }

        // Definir o tempo de execução de cada processo;
        usleep(1000 * quatum_ms);

        // Verificar se os processos foram concluídos e aguardar se não foram;
        for (int i = 0; i < N; i++)
        {
            if (temp_pro[i] == NULL) continue;

            pthread_mutex_lock(&temp_pro[i]->m_exec);
            temp_pro[i]->flag_exec = 1;
            pthread_mutex_unlock(&temp_pro[i]->m_exec);               
            pthread_mutex_lock(&temp_pro[i]->m_end);

            // Verificar se o processo foi concluído ou não e ajustar a fila de processos;
            if (temp_pro[i]->flag_end)
            {
                // Exibir a finalização do processo;
                printf("Processo %s finalizado\n", temp_pro[i]->name);
                fflush(stdout);         // Limpar o buffer de saída;

                // Encerrar o processo e liberar a memória;
                pthread_mutex_unlock(&temp_pro[i]->m_end);
                pthread_mutex_destroy(&temp_pro[i]->m_end);
                pthread_mutex_destroy(&temp_pro[i]->m_exec);
                pthread_cond_destroy(&temp_pro[i]->con);
                free(temp_pro[i]);
                // Atualizar o número total de processos processados;
                total_processed++;
            } else {
                pthread_mutex_unlock(&temp_pro[i]->m_end);

                // Exibir a não finalização do processo;             
                printf("Processo %s nao terminou\n", temp_pro[i]->name);
                fflush(stdout);         // Limpar o buffer de saída;

                // Reorganizar a fila de processos;
                push(lista_pronto, temp_pro[i]);
            }

            temp_pro[i] = NULL;
        }
    }

    // Encerrar o escalonador;
    pthread_exit(NULL);
}

// Código da função genérica para execução de processos (atualizar terminal);
void *generic_func(void *args)
{
    process* p_data = (process* ) args;
    printf("Processo de nome %s iniciado\n", p_data->name);
    
    pthread_mutex_lock(&p_data->m_exec);
    pthread_cond_wait(&p_data->con, &p_data->m_exec);
    pthread_mutex_unlock(&p_data->m_exec);

    for (int i = 0; i < p_data->exec_qtd; i++)
    {
        if (p_data->flag_exec)
        {
            pthread_mutex_lock(&p_data->m_exec);
            pthread_cond_wait(&p_data->con, &p_data->m_exec);
            pthread_mutex_unlock(&p_data->m_exec);
        }
    }

    // Finalizar o processo;
    pthread_mutex_lock(&p_data->m_end);
    p_data->flag_end = 1;
    pthread_mutex_unlock(&p_data->m_end);
    pthread_exit(NULL);
}

// Código do usuário para criar requisições de processos;
void *user(void* args)
{
    // Criar requisições de processos;
    for (int i = 0; i < MAX_REQUEST; i++)
    {
        // Aguardar um tempo aleatório entre 1µs e 5µs;
        float sleep_between_req = (float) (rand() % 5);
        usleep(sleep_between_req);
        
        // Criar um novo processo e adicionar à lista de processos prontos;
        char* name = (char*)malloc(sizeof(char) * 50);
        sprintf(name, "process_%d", i);

        // Definir o tempo de execução de cada processo e criar a thread do processo;
        int process_quantum = (int) (rand() % 9) * 100.0;
        process* new_process = process_create(name, process_quantum);
        pthread_create(&new_process->id, NULL, generic_func, new_process);
        
        // Adicionar o processo à lista de processos prontos;
        push(lista_pronto, new_process);
    }

    // Encerrar a thread do usuário;
    pthread_exit(NULL);
}