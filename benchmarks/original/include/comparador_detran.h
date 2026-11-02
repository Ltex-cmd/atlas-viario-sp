/*
Motivacao: Comparar os dados de sinistros de transito com criterios consistentes.
Funcionamento geral: Compara registros DETRAN e seus campos com valores informados.
*/

#ifndef COMPARADOR_DETRAN_H
#define COMPARADOR_DETRAN_H

#include "item.h"

#define COMPARACAO_FALHOU -2

int comparar_itens_detran(ITEM *item1, ITEM *item2, const char *campo);
int comparar_item_detran_valor(ITEM *item, const char *campo, const char *valor);

#endif