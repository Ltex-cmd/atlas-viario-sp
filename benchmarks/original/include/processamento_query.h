/*
Motivacao: Aplicar as regras de consulta aos registros da base.
Funcionamento geral: Avalia FILTER e ACCEPT e compara registros conforme ORDER.
*/
#ifndef PROCESSAMENTO_QUERY_H
#define PROCESSAMENTO_QUERY_H

#include <stdbool.h>
#include "item.h"
#include "query.h"

bool item_satisfaz_filter(ITEM *item, QUERY *query);
bool item_satisfaz_accept(ITEM *item, QUERY *query);
int comparar_itens_order(ITEM *item1, ITEM *item2, QUERY *query);

#endif
