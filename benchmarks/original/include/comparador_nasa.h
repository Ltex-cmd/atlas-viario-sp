/*
Motivacao: Comparar dados de asteroides segundo seus atributos.
Funcionamento geral: Compara registros NASA e seus campos com valores informados.
*/
#ifndef COMPARADOR_NASA_H
#define COMPARADOR_NASA_H

#include "item.h"

#define COMPARACAO_FALHOU -2

int comparar_itens_nasa(ITEM *item1, ITEM *item2, const char *campo);
int comparar_item_nasa_valor(ITEM *item, const char *campo, const char *valor);

#endif