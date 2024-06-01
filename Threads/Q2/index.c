#define _XOPEN_SOURCE 600           // Para garantir a compatibilidade com o POSIX;

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>

/**************************************************************************************************************/
//Pode alterar os valores de N e TAM para testar outros casos (lembrar de mudar o array também se necessário);//
/**************************************************************************************************************/

// Parâmetros de teste;
#define N 10                        // Número de threads e partições do array (não deve ser maior que TAM);
#define TAM 100                     // Tamanho do array;

// Array exemplo para teste de ordenação;
int array[TAM] = {
    45, 72, 34, 29, 88, 12, 28, 84, 41, 60, 
    76, 3, 26, 65, 1, 87, 55, 70, 33, 17, 
    10, 50, 96, 38, 58, 73, 21, 66, 9, 14, 
    79, 30, 64, 8, 91, 44, 89, 22, 59, 46, 
    81, 24, 95, 67, 75, 2, 40, 36, 80, 92, 
    31, 77, 56, 15, 6, 98, 83, 19, 4, 27, 
    85, 68, 23, 51, 99, 11, 7, 53, 86, 35, 
    20, 93, 62, 48, 13, 5, 74, 32, 90, 18, 
    57, 25, 71, 61, 37, 16, 47, 100, 42, 39, 
    52, 63, 94, 49, 78, 69, 43, 54, 97, 82
};
int aux[TAM];                       // Array auxiliar para mesclar as partes ordenadas;

// Definir a barreira;
pthread_barrier_t barrier;

// Funções em que as threads irão executar;
void *bubblesort(void *arg);        // Ordenar as partes do array;
void *merge(void *arg);             // Mesclar as partes ordenadas;

int main()
{
    // Variável das threads;
    pthread_t threads[N + 1];       // N threads para ordenar as partes do array e 1 thread para mesclar;
    
    // Variáveis auxiliares;
    int *ids[N];                    // Identificadores das threads;
    int i;

    // Inicializar a barreira para N threads;
    pthread_barrier_init(&barrier, NULL, N);

    // Criar N threads para ordenar N partes do array;
    for (i = 0; i < N; i++)
    {
        ids[i] = (int *)malloc(sizeof(int));
        *ids[i] = i * (TAM / N);
        pthread_create(&threads[i], NULL, bubblesort, (void *)ids[i]);
    }

    // Aguardar a conclusão das N threads por segurança;
    for (i = 0; i < N; i++)         
    {
        pthread_join(threads[i], NULL);
    }

    // Criar uma thread para mesclar as partes ordenadas;
    pthread_create(&threads[N], NULL, merge, NULL);

    // Aguardar a conclusão da thread de mesclagem;
    pthread_join(threads[N], NULL);

    // Liberar a memória alocada para os identificadores das threads;
    for (i = 0; i < N; i++)
    {
        free(ids[i]);
    }

    // Destruir a barreira;
    pthread_barrier_destroy(&barrier);

    // Encerrar o programa;
    pthread_exit(NULL);
    return 0;                       // Não deve ser executado, mas é necessário por boas práticas e para evitar warnings;
}

// Código do bubble sort das partes do array;
void *bubblesort(void *arg)
{
    // Variáveis auxiliares;
    int i, j, temp;
    int start = *((int *)arg);      // Início da parte do array;
    int end = start + (TAM / N);    // Fim da parte do array;                                 
    if (start == (N-1) * (TAM/N))   // Ajustar o fim da última parte do array;
    {
        end = TAM;                  // A última parte do array terá um tamanho menor caso TAM não seja divisível por N;
    }

    // Copiar a parte do array para o array auxiliar;
    for (i = start; i < end; i++)
    {
        aux[i] = array[i];
    }

    // Ordenar a parte do array no auxiliar;
    for (i = start; i < end; i++)
    {
        for (j = start; j < end - 1; j++)
        {
            // Ordenar em ordem crescente; para ordem decrescente, trocar o sinal de comparação;
            if (aux[j] > aux[j + 1])
            {
                temp = aux[j];
                aux[j] = aux[j + 1];
                aux[j + 1] = temp;
            }
        }
    }

    // Aguardar a conclusão das N threads;
    pthread_barrier_wait(&barrier);

    // Encerrar a thread;
    pthread_exit(NULL);
}

// Código da mesclagem das partes ordenadas;
void *merge(void *arg)
{
    // Variáveis auxiliares;
    int i, j, k;
    int min = INT_MAX;              // Valor mínimo para comparação;
    int index_min = -1;             // Índice do valor mínimo;
    int start[N];                   // Início de cada parte do array;
    int end[N];                     // Fim de cada parte do array;
    int index[N];                   // Índice de cada parte do array;

    // Inicializar os índices das partes do array;
    for (i = 0; i < N; i++)
    {
        start[i] = i * (TAM / N);
        index[i] = start[i];
        if (i != N - 1)
        {
            end[i] = start[i] + (TAM / N);
        }
        else
        {
            end[i] = TAM;
        }
    }

    // Mesclar, de forma ordenada, as partes ordenadas do array auxiliar no array original;
    for (i = 0; i < TAM; i++)
    {
        for (j = 0; j < N; j++)
        {
            if (index[j] < end[j] && aux[index[j]] < min)
            {
                min = aux[index[j]];
                index_min = j;
            }
        }
        array[i] = min;
        min = INT_MAX;
        index[index_min]++;
    }

    // Exibir o array ordenado;
    printf("Array ordenado:\n");
    for (i = 0; i < TAM; i++)
    {
        printf("%d ", array[i]);
        if ((i + 1) % (TAM/N) == 0)
        {
            printf("\n");
        }
    }

    // Encerrar a thread;
    pthread_exit(NULL);
}