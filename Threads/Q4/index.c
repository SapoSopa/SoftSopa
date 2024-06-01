#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// Mutex e variável de condição;
pthread_mutex_t mutex_cond = PTHREAD_MUTEX_INITIALIZER;         // Mutex para a variável de condição;
int condicao = 1;                                               // Valido é 1 e inválido é 0;

// Struct para passar informações para cada thread;
struct Cords {
        int altura, largura;
        int** sudoku;
};

// Função para a thread;
void *verificar_grade(void *dados);                             // Função para verificar a grade;

// Função auxiliar;
struct Cords Ccords(int** sudoku, int l, int a);                // Função para passar os dados para a thread;

int main()
{
        // Variável das threads;
        pthread_t ids_threads[3][3];                            // 9 threads em "formato de matriz" para verificar as grades do Sudoku;

        // Variáveis auxiliares;
        int** matrix;                                           // Matriz do Sudoku;
        struct Cords **cordenadas;                              // Coordenadas para passar para as threads;

        // Alocação e leitura da matriz do Sudoku (há 2 casos teste disponíveis no repositório, um válido e outro inválido);
        matrix = (int **)malloc(sizeof(int*) * 9);
        for (int i = 0; i < 9; i++)
        {
                matrix[i] = (int *)malloc(sizeof(int) * 9);
                for (int j = 0; j < 9; j++)
                {
                        scanf("%d ", &matrix[i][j]);
                }
        }

        // Alocação da matriz de coordenadas e inicialização das 9 threads;
        cordenadas = (struct Cords **)malloc(sizeof(struct Cords*) * 3);
        for (int i = 0; i < 3; i++)
        {
                cordenadas[i] = (struct Cords*) malloc(sizeof(struct Cords) * 3);
                for (int j = 0; j < 3; j++)
                {
                        cordenadas[i][j] = Ccords(matrix, 3 * i, 3 * j);
                        pthread_create(&ids_threads[i][j], NULL, verificar_grade, (void *)&cordenadas[i][j]);
                }
        }

        // Aguardar a conclusão das 9 threads;
        for (int i = 0; i < 3; i++)
        {
                for (int j = 0; j < 3; j++) {
                        pthread_join(ids_threads[i][j], NULL);
                }
        }

        // Verificar se o Sudoku é válido ou inválido;
        if(condicao == 1)
        {
                printf("Sudoku Válido\n");
        } else {
                printf("Sudoku Inválido\n");
        }
        
        // Liberar a memória alocada e encerrar o programa;
        for (int i = 0; i < 9; i++)
        {
                free(matrix[i]);
        }
        free(matrix);
        for (int i = 0; i < 3; i++)
        {
                free(cordenadas[i]);
        }
        free(cordenadas);

        return 0;
}

// Código da função auxiliar para passar os dados para a thread;
struct Cords Ccords(int** sudoku, int l, int a)
{
        struct Cords ptr;
        ptr.largura = l;
        ptr.altura = a;
        ptr.sudoku = sudoku;
        return ptr;
};

// Código da thread de verificação da grade do Sudoku;
void *verificar_grade(void *dados)
{
        // Variáveis auxiliares;
        int aux_3x3 = 0;                                        // Auxiliar para verificar a grade 3x3;
        int aux_linha = 0;                                      // Auxiliar para verificar a linha;
        int aux_coluna = 0;                                     // Auxiliar para verificar a coluna;
        struct Cords crd = *((struct Cords*) dados);            // Coordenadas da grade do Sudoku;

        // Realizar a verificação da grade do Sudoku (cada thread verifica uma grade 3x3);
        for (int i = crd.largura; i < crd.largura + 3; i++)
        {
                for (int j = crd.altura; j < crd.altura + 3; j++)
                {       
                        /***********************************************************/
                        // Explicação sobre a ideia dessa parte no final do código;//
                        /***********************************************************/

                        if (!condicao)                          // Checar se o Sudoku é válido ou inválido antes de continuar a verificação;
                        {
                                pthread_exit(NULL);
                        }

                        // Verificar o setor 3x3;
                        int bit_3x3 = 1 << crd.sudoku[i][j];
                        if(aux_3x3 & bit_3x3)                   // Se o número já existe no setor 3x3, o Sudoku é inválido e a thread é encerrada;
                        {                         
                                pthread_mutex_lock(&mutex_cond);
                                condicao = 0;
                                pthread_mutex_unlock(&mutex_cond);

                                pthread_exit(NULL);
                        }
                        aux_3x3 |= bit_3x3;

                        // Verificar a linha;
                        for (int _j = 0; _j < 9; _j++)
                        {
                                if (!condicao)                  // Checar se o Sudoku é válido ou inválido antes de continuar a verificação;
                                {
                                        pthread_exit(NULL);
                                }
                                int bit_linha = 1 << crd.sudoku[i][_j];
                                if(aux_linha & bit_linha)       // Se o número já existe na linha, o Sudoku é inválido e a thread é encerrada;
                                {
                                        pthread_mutex_lock(&mutex_cond);
                                        condicao = 0;
                                        pthread_mutex_unlock(&mutex_cond);

                                        pthread_exit(NULL);
                                }
                                aux_linha |= bit_linha;
                        }

                        // Verificar a coluna;
                        for (int _i = 0; _i < 9; _i++)
                        {
                                if (!condicao)                  // Checar se o Sudoku é válido ou inválido antes de continuar a verificação;
                                {
                                        pthread_exit(NULL);
                                }
                                int bit_coluna = 1 << crd.sudoku[_i][j];
                                if(aux_coluna & bit_coluna)     // Se o número já existe na coluna, o Sudoku é inválido e a thread é encerrada;
                                {
                                        pthread_mutex_lock(&mutex_cond);
                                        condicao = 0;
                                        pthread_mutex_unlock(&mutex_cond);

                                        pthread_exit(NULL);
                                }
                                aux_coluna |= bit_coluna;
                        }
                }

        } 

        // Encerrar a thread;
        pthread_exit(NULL);
}

/********************************************************************************************************/
///////////////////////////// Ideia do código de verificação do Sudoku ///////////////////////////////////
/********************************************************************************************************/
/*
* Se trata de um bitwise que verifica a existência de números repetidos e evita mais for encadeado:      *
* Usar left_shift (<<), ao deslocar o 1 o número de casas do dígito correspondente, nos gera correlações *
* como 100 para o numero 2, 1000 para o 3 e assim por diante, permitindo que o aux_3x3 possa agir como um*
* acumulador que funciona como sendo um vetor, no qual cada bit representa o número correspondente a sua *
* posição, logo para verificar a existência de algum número nesse Aux, basta fazer um and bit a bit, e se* 
* ele resultar em algo diferente de zero, um número se repetiu e o Sudoku é inválido, caso contrário, ele* 
* realiza um OR bit a bit para converter o 0 em 1 na posicao correta.                                    *
*  Ex:                                                                                                   *
*  1011000010 -> {9, 1, 6, 7}                                                                            *
*  0100000000 -> {8}                                                                                     *
*  O 8 não está no "vetor" então não entra no if, pois o AND bit a bit vai dar 0, então o OR bit a bit o *
*  adiciona ao "vetor":                                                                                  *
*  1111000010 -> {1, 6, 7, 8, 9}                                                                         *
*/