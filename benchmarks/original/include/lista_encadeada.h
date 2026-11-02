/*
Motivacao: Armazenar registros dinamicamente sem exigir um vetor contiguo.
Funcionamento geral: Adiciona nos ao fim e percorre suas ligacoes.
*/
#ifndef LISTA_ENCADEADA_H
#define LISTA_ENCADEADA_H

#include <stdbool.h>
#include "item.h"

typedef struct lista_encadeada_ LISTA_ENCADEADA;

LISTA_ENCADEADA *lista_encadeada_criar(int tamanho_max);

/* Insere sempre no fim da lista. */
bool lista_encadeada_inserir(LISTA_ENCADEADA *lista, ITEM *item);

/* Apaga os nos; se apagar_itens for true, apaga tambem os itens. */
void lista_encadeada_apagar(LISTA_ENCADEADA **lista, bool apagar_itens);

ITEM *lista_encadeada_percorrer(LISTA_ENCADEADA *lista);

#endif
