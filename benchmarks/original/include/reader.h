/*
Motivacao: Unificar a leitura de registros de diferentes fontes.
Funcionamento geral: Seleciona o leitor NASA ou DETRAN e retorna um ITEM.
*/
#ifndef READER_H
#define READER_H

#include <stdio.h>
#include "item.h"

ITEM *reader_ler_item(FILE *arquivo, const char *tipo);

#endif