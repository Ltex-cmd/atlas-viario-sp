/*
Motivacao: Organizar os registros respeitando a ordem de chegada (FIFO).
Funcionamento geral: Permite inserir e percorrer registros sequencialmente, sem expor os detalhes internos da estrutura.
*/
#ifndef FILA_H
#define FILA_H

#include <stdbool.h>
#include "item.h"

typedef struct fila_ FILA;

FILA *fila_criar(int tamanho_max);

/* Insere no fim. Em sucesso, a fila recebe o item; em falha, nao o apaga. */
bool fila_inserir(FILA *fila, ITEM *item);

void fila_apagar(FILA **fila, bool apagar_itens);

/* true: reinicia e retorna o primeiro item existente.
   false: continua e retorna o proximo item existente.
   Pula itens NULL e retorna NULL no fim ou para fila NULL.
   Nao remove os itens. Ha um unico percurso ativo por fila.
   O ponteiro retornado e emprestado: apaga-lo nao zera o ponteiro na fila. */
ITEM *fila_percorrer(FILA *fila);

#endif

