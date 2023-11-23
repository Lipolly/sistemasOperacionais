#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// Estrutura do semaforo
typedef struct {
  pthread_mutex_t lock;
  pthread_cond_t wait;
  int vagas;
} semaforo;

void error(char *msg) {
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(1);
}

// Função para inicializar a estrutura
void init(semaforo *s, int vagas) {
  if (vagas <= 0) {
    error("O valor de vagas tem que ser maior que 0!");
  }

  s->vagas = vagas;
  if (pthread_cond_init(&(s->wait), NULL) == -1) {
    error("Erro ao inicializar a condição");
  }
  
  if (pthread_mutex_init(&(s->lock), NULL) == -1) {
    error("Erro ao inicializar o mutext");
  }

  return;
}

// Função para decrementar as vagas e esperar quando não existirem 
// vagas disponiveis
void p(semaforo *s) {
  pthread_mutex_lock(&(s->lock));
  s->vagas--;
  
  if (s->vagas < 0) {
    printf("Não existem vagas disponiveis, aguarde...\n");
    pthread_cond_wait(&(s->wait), &(s->lock));
    printf("Vaga liberada!\n");
  }

  printf("Um carro esta passando, %d vagas disponiveis.\n", s->vagas);

  pthread_mutex_unlock(&(s->lock));
  return;
}

// Função para liberar as vagas e mandar o sinal
// quando foi liberada uma vaga bloqueada
void v(semaforo *s) {
  pthread_mutex_lock(&(s->lock));

  s->vagas++;

  if (s->vagas <= 0) {
    pthread_cond_signal(&(s->wait));
  }

  printf("Uma vaga liberada. %d vagas disponiveis.\n", s->vagas);
  pthread_mutex_unlock(&(s->lock));
  return;
}

// Função para simular a entrada dos carros
void* entrando_carro(void* void_s) {
  semaforo *s = void_s;

  for (int i = 0; i < 20;i++) {
    p(s);
    sleep(1);
  }

  return NULL;
}

// Função para simular a saida dos carros
void* saida_carro(void* void_s) {
  semaforo *s = void_s;

  for (int i = 0; i < 15;i++) {
    v(s);
    sleep(3);
  }

  return NULL;
}

int main() {
  semaforo s;

  init(&s, 10);

  pthread_t t_saida_carros;
  pthread_t t_entrando_carros;

  // Criando as threads de entrada e saida de carros
  if (pthread_create(&t_entrando_carros, NULL, entrando_carro, &s) == -1) {
    error("Erro ao criar a thread 't_entrando_carros'!");
  }

  if (pthread_create(&t_saida_carros, NULL, saida_carro, &s) == -1) {
    error("Erro ao criar a thread 't_saida_carros'!");
  }

  // Aguardando as threads terminarem
  if (pthread_join(t_entrando_carros, (void *) &s) == -1) {
    error("Erro ao realizar o join da thread 't_entrando_carros'!");
  }

  if (pthread_join(t_saida_carros, (void *) &s) == -1) {
    error("Erro ao realizar o join da thread 't_saida_carros'!");
  }

  return 0;
}
