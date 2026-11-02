/*
Motivacao: Armazenar registros em um vetor que cresce conforme necessario.
Funcionamento geral: Insere ao fim e percorre os elementos por indice.
*/
#ifndef LISTA_SEQUENCIAL_H
#define LISTA_SEQUENCIAL_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_sequencial_ LISTA_SEQUENCIAL;

LISTA_SEQUENCIAL *lista_sequencial_criar(int tamanho_max);

bool lista_sequencial_inserir(LISTA_SEQUENCIAL *lista, ITEM *item);

void lista_sequencial_apagar(LISTA_SEQUENCIAL **lista, bool apagar_itens);

ITEM *lista_sequencial_percorrer(LISTA_SEQUENCIAL *lista);

#endif
