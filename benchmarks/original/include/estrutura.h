/*
Motivacao: Permitir a troca da estrutura de dados sem mudar o fluxo principal.
Funcionamento geral: Cria, insere, percorre e libera pilhas, filas e listas pela mesma interface.
*/

#ifndef ESTRUTURA_H
#define ESTRUTURA_H

#include <stdbool.h>

#include "item.h"
#include "query.h"

typedef struct estrutura_ ESTRUTURA;

ESTRUTURA *estrutura_criar_base(const char *tipo, const char *const *campos, int quantidade_campos, int tamanho_max);

ESTRUTURA *estrutura_criar_saida(const char *tipo, QUERY *query, int tamanho_max);

bool estrutura_inserir(ESTRUTURA *estrutura, ITEM *item);

ITEM *estrutura_percorrer(ESTRUTURA *estrutura, const char *campo);

void estrutura_apagar(ESTRUTURA **estrutura, bool apagar_itens);

#endif