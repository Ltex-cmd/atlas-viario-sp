/*
Motivacao: Processar registros do mais recente ao mais antigo (LIFO).
Funcionamento geral: Empilha no topo e percorre os nos na ordem inversa de insercao.
*/
#ifndef PILHA_H
#define PILHA_H

#include <stdbool.h>
#include "item.h"

typedef struct pilha_ PILHA;

PILHA *pilha_criar(int tamanho_max);

bool pilha_inserir(PILHA *pilha, ITEM *item);

void pilha_apagar(PILHA **pilha, bool apagar_itens);

ITEM *pilha_percorrer(PILHA *pilha);

#endif
