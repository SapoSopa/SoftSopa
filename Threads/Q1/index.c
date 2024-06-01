#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Passe o nome dos arquivos de texto ao executar o programa (ficar atento com o valor de x);
// Parâmetros de teste;
#define x 4                         // Número de arquivos de texto;
#define TAM 1000                    // Tamanho do texto;

// Variável global;
int contador = 0;
pthread_mutex_t mutex;              // Mutex para controle de acesso à variável global;

// Struct para passar argumentos;
typedef struct {
    const char *arquivo;
    const char *palavra;
} Args;

// Função para a thread;
void *busca(void *args);

int main(int argc, char *argv[]) {
    // Palavra a se buscar nos arquivos;
    const char *word = "threads";  // Pode ser alterada para outra palavra;

    // Inicializar o mutex;
    pthread_mutex_init(&mutex, NULL);

    // Variável dad threads;
    pthread_t thread[x];            // x threads para buscar a palavra nos arquivos;

    // Variável auxiliar;
    Args threadArgs[x];             // Argumentos para as threads;

    // Criar as x threads para buscar a palavra nos x arquivos;
    for (int i = 0; i < x; i++) {
        threadArgs[i].arquivo = argv[i + 1];
        threadArgs[i].palavra = word;

        if (pthread_create(&thread[i], NULL, busca, &threadArgs[i]) != 0) {
            printf("Erro ao criar a thread\n");
            return 1;
        }
    }

    // Aguardar a conclusão das threads;
    for (int j = 0; j < x; j++) {
        pthread_join(thread[j], NULL);
    }

    // Destroir o mutex;
    pthread_mutex_destroy(&mutex);

    // Exibir o resultado;
    printf("Total de ocorrências: %d\n", contador);

    return 0;
}

// Código para a realização da busca por threads;
void *busca(void *args) {
    // Variáveis auxiliares;
    Args *threadArgs = (Args *)args;            // Argumentos para a thread;
    FILE *file;                                 // Arquivo de texto;
    const char *arquivo = threadArgs->arquivo;  // Nome do arquivo;
    const char *palavra = threadArgs->palavra;  // Palavra a ser buscada;
    int count = 0;                              // Contador de ocorrências;
    char texto[TAM];                            // Texto do arquivo;

    // Abrir o arquivo;
    file = fopen(arquivo, "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        pthread_exit(NULL);
    }

    // Buscar a palavra no arquivo;
    while (fgets(texto, TAM, file)) {
        char *linha = texto;                    // Aponta para o início do texto;
        while ((linha = strstr(linha, palavra)) != NULL) {
            count++;
            linha += strlen(palavra);           // Atualiza a posição do ponteiro;
        }
    }

    // Fechar o arquivo;
    fclose(file);

    // Atualizar contador global;
    pthread_mutex_lock(&mutex);
    contador += count;
    pthread_mutex_unlock(&mutex);

    // Encerrar a thread;
    pthread_exit(NULL);
}
