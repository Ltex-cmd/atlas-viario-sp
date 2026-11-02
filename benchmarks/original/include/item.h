/*
Motivacao: Tratar registros NASA e DETRAN por meio de uma interface comum.
Funcionamento geral: Acessa campos, calcula similaridade, imprime e libera registros.
*/

#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include <stdio.h>
#include "query.h"

#define TIPO_NASA 1
#define TIPO_DETRAN 2

#define VALOR_PRESENTE 0
#define VALOR_AUSENTE 1
#define CAMPO_INVALIDO 2

#define SIMILARIDADE_NAO_CALCULADA -1.0
#define SIMILARIDADE_FALHOU -2.0

typedef struct item_ ITEM;

int item_get_tipo(ITEM *item);

int validar_valor(ITEM *item, const char *campo);

void item_apagar(ITEM **item);

bool imprimir_item_arquivo(ITEM *item, FILE *arquivo);

bool obter_valor_numerico_item(ITEM *item, const char *campo, double *valor);

bool item_calcular_similaridade(ITEM *item, QUERY *query);
double item_get_similaridade(ITEM *item);


#endif