#define _XOPEN_SOURCE 600               // Para garantir a compatibilidade com o POSIX;

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/***************************************************************************************************************/
//Pode alterar os valores de I, P e do SEL para outros testes (lembrar de mudar as matrizes quando necessário);//
/***************************************************************************************************************/

// Parâmetros de teste;
#define I 4                             // Número de equações;
#define P 10                            // Número de iterações de refino;

                /////////////////////////////
                //   O sistema exemplo é:  //
                //  6w -  x -  y + 4z= 17  //
                //   w -10x + 2y -  z=-17  //
                //  3w - 2x + 8y -  z= 19  //
                //   w +  x +  y - 5z=-14  //
                /////////////////////////////
                //      A solução é:       //
                //      w = 1.00           //
                //      x = 2.00           //
                //      y = 3.00           //
                //      z = 4.00           //
                // P > 20 se feito a mão   //
                /////////////////////////////

// Matrizes exemplo para o médoto de Jacobi;
float A[I][I] = {                       // Matriz dos coeficientes das equações;
    {6, -1, -1, 4},
    {1, -10, 2, -1},
    {3, -2, 8, -1},
    {1, 1, 1, -5}
};
float B[I] = {17, -17, 19, -14};        // Vetor (matriz coluna) dos resultados das equações;
float X[I] = {1, 1, 1, 1};              // Vetor (matriz coluna) dos resultados iniciais das equações (1 por padrão);
float aux[I] = {0, 0, 0, 0};            // Vetor auxiliar para armazenar os resultados temporários;

// Variável de quantidade de threads/núcleo/processadores;
int N;

// Definir a barreira;
pthread_barrier_t barrier;

// Função em que as threads irão executar;
void *jacobi(void *arg);                // Método de Jacobi;

int main()
{
    // Determinar a quantidade N de threads;
    printf("Digite a quantidade de processadores (ou núcleos): ");
    scanf("%d", &N);

    // Variável das threads;
    pthread_t threads[N];               // N threads para o método de Jacobi;

    // Variáveis auxiliares;
    int *ids[N];                        // Identificadores das threads;
    int i, j;

    // Inicializar a barreira para N threads;
    pthread_barrier_init(&barrier, NULL, N);

    // Criar N threads para o método de Jacobi;
    for (i = 0; i < N; i++)
    {
        ids[i] = (int *)malloc(sizeof(int));
        *ids[i] = i;
        pthread_create(&threads[i], NULL, jacobi, (void *)ids[i]);
    }

    // Aguardar a conclusão das N threads;
    for (i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destruir a barreira;
    pthread_barrier_destroy(&barrier);

    // Desalocar os argumentos das threads;
    for (i = 0; i < N; i++)
    {
        free(ids[i]);
    }

    // Imprimir sistema de equações;
    printf("Sistema de equações:\n");
    for (i = 0; i < I; i++)
    {
        for (j = 0; j < I; j++)
        {
            printf("%.0fX[%d] ", A[i][j], j + 1);
            if (j < I - 1)
            {
                printf("+ ");
            }
            else 
            {
                printf("= %.0f\n", B[i]);
            }
        }
    }

    // Imprimir a solução do sistema;
    printf("Solução do sistema:\n");
    for (i = 0; i < I; i++)
    {
        printf("X[%d] = %.2f\n", i + 1, X[i]);
    }

    // Encerrar o programa;
    pthread_exit(NULL);
    return 0;                           // Não deve ser executado, mas é necessário por boas práticas e para evitar warnings;
}

// Método de Jacobi;
void *jacobi(void *arg)
{
    // Variáveis auxiliares;
    int i , j, k;
    float soma;                         // Soma dos valores de A[][] * X[] do método de Jacobi;
    int start = *((int*) arg) * (I/N);  // Primeiro valor de X[] a ser calculado pela thread;
    int end = start + (I/N);            // Primeiro valor de X[] a não ser calculado pela thread;
    if (*((int*) arg) == N-1)           // Se for a última thread, calcular até o último valor de X[];
    {
        end = I;                        // O último valor de X[] pode ser diferente dos outros;
    }
    
    // Algoritmo de refino de Jacobi;
    for (k = 0; k < P ; k++)
    {
        // Calcular os valores de X[];
        for (i = start ; i < end ; i++)
        {
            soma = 0;
            for (j = 0 ; j < I ; j++)
            {
                if (j != i)
                {
                    soma += A[i][j] * X[j];
                }
            }
            aux[i] = (B[i] - soma) / A[i][i];
        }

        // Aguardar a conclusão das N threads os valores de X[] calculados;
        pthread_barrier_wait(&barrier);

        // Atualizar os valores de X[];
        for (i = start; i < end; i++)
        {
            X[i] = aux[i];
        }
    }

    // Encerrar a thread;
    pthread_exit(NULL);
}