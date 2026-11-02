/*
Motivacao: Simplificar o encadeamento com um no-cabeca sentinela.
Funcionamento geral: Insere, percorre e libera registros ligados por nos.
*/
#ifndef LISTA_CABECA_H
#define LISTA_CABECA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_cabeca_ LISTA_CABECA;

LISTA_CABECA *lista_cabeca_criar(int tamanho_max);
bool lista_cabeca_inserir(LISTA_CABECA *lista, ITEM *item);
void lista_cabeca_apagar(LISTA_CABECA **lista, bool apagar_itens);
ITEM *lista_cabeca_percorrer(LISTA_CABECA *lista);

#endif