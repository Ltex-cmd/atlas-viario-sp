/*
Motivacao: Unificar as comparacoes entre registros de fontes diferentes.
Funcionamento geral: Compara ITEMs entre si ou com valores de um campo.
*/
#ifndef COMPARADOR_H
#define COMPARADOR_H

#include "item.h"

#define COMPARACAO_FALHOU -2

int comparar_itens(ITEM *item1, ITEM *item2, const char *campo);
int comparar_item_valor(ITEM *item, const char *campo, const char *valor);

#endif
