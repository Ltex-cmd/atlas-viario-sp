/*
Motivacao: Permitir diferentes ordenacoes dos mesmos registros.
Funcionamento geral: Organiza uma sequencia por campo e percorre a dimensao escolhida.
*/
#ifndef LISTA_CRUZADA_H
#define LISTA_CRUZADA_H

#include <stdbool.h>
#include "item.h"
#include "query.h"
#include "tipo_lista.h"

typedef struct lista_cruzada_ LISTA_CRUZADA;

LISTA_CRUZADA *lista_cruzada_criar_base(const char *const *campos, int dimensao, int tamanho_max);
LISTA_CRUZADA *lista_cruzada_criar_saida(QUERY *query, int tamanho_max);

bool lista_cruzada_inserir(LISTA_CRUZADA *lista, ITEM *item);
void lista_cruzada_apagar(LISTA_CRUZADA **lista, bool apagar_itens);
ITEM *lista_cruzada_percorrer(LISTA_CRUZADA *lista, const char *campo);

#endif
