#include <stdio.h>
#include <string.h>

#include "reader.h"
#include "reader_nasa.h"
#include "reader_detran.h"

/*
Objetivo: Encaminhar a leitura de um registro CSV para a funcao especifica
       da base escolhida (NASA ou DETRAN). A funcao nao abre nem fecha o
       arquivo: apenas utiliza o ponteiro recebido, que avanca durante a leitura.

Parametros:
- arquivo: Ponteiro para o CSV ja aberto, posicionado no proximo registro
           a ser lido.
- tipo: String que identifica qual leitor deve ser utilizado ("nasa" ou
        "detran").

Retorno: Ponteiro para o ITEM criado pelo leitor especifico ou NULL se os
         parametros forem invalidos, o tipo nao for reconhecido ou o leitor
         nao conseguir retornar um registro (inclusive no fim do arquivo).
*/
ITEM *reader_ler_item(FILE *arquivo, const char *tipo){

    if (arquivo == NULL || tipo == NULL)
        return NULL;

    if (strcmp(tipo, "nasa") == 0)
        return reader_ler_item_nasa(arquivo); /* Leitura do CSV de asteroides. */

    if (strcmp(tipo, "detran") == 0)
        return reader_ler_item_detran(arquivo); /* Leitura do CSV de sinistros. */

    return NULL;
}