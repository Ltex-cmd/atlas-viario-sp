/*
Motivacao: Manter os registros classificados por um criterio de comparacao.
Funcionamento geral: Insere cada ITEM na posicao ordenada e percorre essa sequencia.
*/
#ifndef LISTA_ORDENADA_H
#define LISTA_ORDENADA_H

#include <stdbool.h>
#include "item.h"
#include "query.h"
#include "tipo_lista.h"

typedef struct lista_ordenada_ LISTA_ORDENADA;

/* BASE: recebe um campo; SAIDA: recebe uma query com ORDER.
   A lista copia o campo, mas nao possui a query. */
LISTA_ORDENADA *lista_ordenada_criar_base(const char *campo, int tamanho_max);
LISTA_ORDENADA *lista_ordenada_criar_saida(QUERY *query, int tamanho_max);

/* Insere em ordem, preservando a ordem de insercao entre valores iguais. */
bool lista_ordenada_inserir(LISTA_ORDENADA *lista, ITEM *item);

void lista_ordenada_apagar(LISTA_ORDENADA **lista, bool apagar_itens);
 
/* Percorre uma unica vez. Apos inserir, inicia novamente pelo primeiro no. */
ITEM *lista_ordenada_percorrer(LISTA_ORDENADA *lista);

#endif