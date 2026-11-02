/*
Motivacao: Converter os dados de sinistros em registros manipulaveis.
Funcionamento geral: Le registros da fonte DETRAN e cria os respectivos ITEMs.
*/
#ifndef READER_DETRAN_H
#define READER_DETRAN_H

#include <stdio.h>
#include "item.h"

ITEM *reader_ler_item_detran(FILE *arquivo);

#endif
