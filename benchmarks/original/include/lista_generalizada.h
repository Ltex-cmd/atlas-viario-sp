/*
Motivacao: Organizar registros em varios niveis de agrupamento.
Funcionamento geral: Cria subgrupos hierarquicos e permite insercao e percurso.
*/

#ifndef LISTA_GENERALIZADA_H
#define LISTA_GENERALIZADA_H

#include <stdbool.h>
#include "item.h"
#include "query.h"

typedef struct lista_generalizada_ LISTA_GENERALIZADA;

LISTA_GENERALIZADA *lista_generalizada_criar_base(const char *const *campos, int profundidade, int tamanho_max);

LISTA_GENERALIZADA *lista_generalizada_criar_saida(QUERY *query, int tamanho_max);

bool lista_generalizada_inserir(LISTA_GENERALIZADA *lista, ITEM *item);
void lista_generalizada_apagar(LISTA_GENERALIZADA **lista, bool apagar_itens);
ITEM *lista_generalizada_percorrer(LISTA_GENERALIZADA *lista);

#endif
