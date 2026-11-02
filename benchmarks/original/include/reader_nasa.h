/*
Motivacao: Converter os dados de asteroides em registros manipulaveis.
Funcionamento geral: Le registros da fonte NASA e cria os respectivos ITEMs.
*/
#ifndef READER_NASA_H
#define READER_NASA_H

#include <stdio.h>
#include "item.h"

ITEM *reader_ler_item_nasa(FILE *arquivo);

#endif