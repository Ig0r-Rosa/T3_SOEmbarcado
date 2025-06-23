// Inclusão das bibliotecas necessárias
#include <stdio.h>         // printf()
#include <stdlib.h>        // exit(), malloc(), etc
#include <pthread.h>       // threads POSIX
#include <mqueue.h>        // filas de mensagens POSIX
#include <semaphore.h>     // semáforos POSIX
#include <unistd.h>        // sleep()

// Definições da fila de mensagens
#define QUEUE_NAME "/my_queue"     // Nome da fila POSIX (precisa começar com '/')
#define MAX_MSG_SIZE 64            // Tamanho máximo de cada mensagem
#define QUEUE_SIZE 10              // Número máximo de mensagens na fila

// Semáforo global usado para sincronizar produtor e consumidor
sem_t sem;
// Função da thread produtora
void *producer_thread(void *arg) 
{
    mqd_t mq;                          // Descritor da fila
    char msg[MAX_MSG_SIZE];           // Buffer para armazenar a mensagem
    int count = 0;                    // Contador de mensagens

    mq = mq_open(QUEUE_NAME, O_WRONLY);  // Abre a fila no modo escrita (escritor)
    
    while (1) {
        // Escreve uma mensagem no formato "Mensagem número: X"
        snprintf(msg, MAX_MSG_SIZE, "Mensagem número: %d", count++);

        // Envia a mensagem para a fila
        mq_send(mq, msg, MAX_MSG_SIZE, 0);

        // Log no console para debug
        printf("[Produtor] Enviado: %s\n", msg);

        // Sinaliza o semáforo (avisa o consumidor que tem mensagem nova)
        sem_post(&sem);

        // Aguarda 1 segundo antes de produzir a próxima mensagem
        sleep(1);
    }

    return NULL;
}

// Função da thread consumidora
void *consumer_thread(void *arg) {
    mqd_t mq;                         // Descritor da fila
    char msg[MAX_MSG_SIZE];          // Buffer para receber a mensagem

    mq = mq_open(QUEUE_NAME, O_RDONLY);  // Abre a fila no modo leitura (leitor)

    while (1) {
        // Espera o semáforo ser liberado pelo produtor
        sem_wait(&sem);

        // Lê a próxima mensagem da fila
        mq_receive(mq, msg, MAX_MSG_SIZE, NULL);

        // Mostra a mensagem recebida no terminal
        printf("[Consumidor] Recebido: %s\n", msg);
    }

    return NULL;
}

int main(void) {
    pthread_t producer, consumer;   // Identificadores das threads
    struct mq_attr attr;            // Atributos da fila de mensagens
    mqd_t mq;                       // Descritor da fila

    // Define os atributos da fila
    attr.mq_flags = 0;              // Sem flags especiais
    attr.mq_maxmsg = QUEUE_SIZE;   // Máximo de mensagens na fila
    attr.mq_msgsize = MAX_MSG_SIZE;// Tamanho de cada mensagem
    attr.mq_curmsgs = 0;           // Começa vazia

    // Remove fila anterior (se existir) para evitar erro de duplicidade
    mq_unlink(QUEUE_NAME);

    // Cria a fila de mensagens com leitura e escrita
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);

    // Inicializa o semáforo com valor 0 (bloqueado)
    sem_init(&sem, 0, 0);

    // Cria a thread produtora
    pthread_create(&producer, NULL, producer_thread, NULL);

    // Cria a thread consumidora
    pthread_create(&consumer, NULL, consumer_thread, NULL);

    // Aguarda as threads (esse programa nunca termina, então na prática isso bloqueia)
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    // Fecha e remove a fila
    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    // Destroi o semáforo
    sem_destroy(&sem);

    return 0;
}