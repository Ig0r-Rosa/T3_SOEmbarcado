// Inclusão das bibliotecas necessárias
#include <stdio.h>         // printf()
#include <stdlib.h>        // exit(), malloc(), etc
#include <pthread.h>       // threads POSIX
#include <mqueue.h>        // filas de mensagens POSIX
#include <semaphore.h>     // semáforos POSIX
#include <unistd.h>        // sleep()

// Definições da fila de mensagens
#define QUEUE_NAME "/my_queue"  // Nome da fila POSIX (obrigatoriamente começa com '/')
#define MAX_MSG_SIZE 32         // Tamanho máximo de cada mensagem
#define QUEUE_SIZE 4            // Máximo de mensagens na fila

// Semáforo global usado para sincronizar produtor e consumidor
sem_t sem;

// Função da thread produtora
void *producer_thread(void *arg) 
{
    mqd_t mq;
    struct mq_attr attr;
    char msg[MAX_MSG_SIZE];
    int count = 0;

    // Define atributos da fila
    attr.mq_flags = 0;
    attr.mq_maxmsg = QUEUE_SIZE;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // Cria a fila com O_CREAT
    mq_unlink(QUEUE_NAME);  // Garante que não exista uma fila antiga
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1) {
        perror("[Produtor] Erro ao criar a fila");
        pthread_exit(NULL);
    }

    // Loop infinito para enviar mensagens
    while (1) 
    {
        snprintf(msg, MAX_MSG_SIZE, "Mensagem número: %d", count++);
        mq_send(mq, msg, MAX_MSG_SIZE, 0);
        printf("[Produtor] Enviado: %s\n", msg);
        sem_post(&sem);
        sleep(1);
    }

    return NULL;
}

// Função da thread consumidora
void *consumer_thread(void *arg) 
{
    mqd_t mq;
    char msg[MAX_MSG_SIZE];

    printf("[Consumidor] Thread iniciada\n");

    // Abre a fila de mensagens para leitura
    mq = mq_open(QUEUE_NAME, O_RDWR);
    if (mq == (mqd_t)-1) {
        perror("[Consumidor] Erro ao abrir a fila");
        pthread_exit(NULL);
    }

    // Loop infinito para receber mensagens
    while (1) 
    {
        sem_wait(&sem);
        mq_receive(mq, msg, MAX_MSG_SIZE, NULL);
        printf("[Consumidor] Recebido: %s\n", msg);
    }

    return NULL;
}

int main(void) 
{
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