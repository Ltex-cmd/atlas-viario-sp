#include <stddef.h>

#include "comparador.h"
#include "comparador_nasa.h"
#include "comparador_detran.h"
#include "item.h"

/*
Objetivo: Comparar dois registros de uma mesma base utilizando o campo
informado. Identifica se os itens pertencem a NASA ou ao DETRAN e chama
o comparador especifico. Para a NASA, verifica antes se o campo possui
valor, tratando os valores ausentes de maneira padronizada.

Parametros:
- item1: Primeiro registro a ser comparado.
- item2: Segundo registro a ser comparado.
- campo: Nome do atributo utilizado como criterio de comparacao.

Retorno: -1 se item1 for menor, 0 se os valores forem iguais e 1 se
item1 for maior. Retorna COMPARACAO_FALHOU (-2) quando os parametros,
os tipos ou os campos nao permitem a comparacao.
*/
int comparar_itens(ITEM *item1, ITEM *item2, const char *campo){

    if (item1 == NULL || item2 == NULL || campo == NULL)
        return COMPARACAO_FALHOU;

    int tipo1 = item_get_tipo(item1);
    int tipo2 = item_get_tipo(item2);
    
    /* Evita comparar diretamente registros de bases diferentes. */
    if (tipo1 != tipo2)
        return COMPARACAO_FALHOU;

    if (tipo1 == TIPO_NASA){

         /* Confere se o campo existe e se ha valor para cada asteroide. */
        int estado1 = validar_valor(item1, campo);
        int estado2 = validar_valor(item2, campo);

         /* Estado 2 indica campo invalido: a comparacao nao pode continuar. */
        if (estado1 == 2 || estado2 == 2)
            return COMPARACAO_FALHOU;

        /* Campos ausentes (estado 1) sao considerados maiores que os presentes. */
        if (estado1 == 1 && estado2 == 1) return 0;
        if (estado1 == 1) return 1;
        if (estado2 == 1) return -1;
        
        /* Com ambos os valores presentes, usa a comparacao propria da NASA. */
        return comparar_itens_nasa(item1, item2, campo);
    }

    /* O DETRAN possui regras de comparacao proprias para seus campos. */
    if (tipo1 == TIPO_DETRAN)
        return comparar_itens_detran(item1, item2, campo);

    return COMPARACAO_FALHOU; /* Tipo de registro nao reconhecido. */
}

/*
Objetivo: Comparar o conteudo de um campo de um registro com um valor
fornecido em texto, encaminhando a operacao para o comparador da base
correspondente (NASA ou DETRAN). A conversao do texto, quando necessaria,
e realizada pela funcao especifica de cada base.

Parametros:
- item: Registro cujo campo sera comparado com o valor informado.
- campo: Nome do atributo a ser consultado no registro.
- valor: Representacao textual do valor utilizado como referencia.

Retorno: -1 se o campo do registro for menor que o valor de referencia,
0 se forem iguais e 1 se for maior. Retorna COMPARACAO_FALHOU (-2)
quando a comparacao nao puder ser realizada.
*/
int comparar_item_valor(ITEM *item, const char *campo, const char *valor){

    if (item == NULL || campo == NULL || valor == NULL)
        return COMPARACAO_FALHOU;

    int tipo = item_get_tipo(item);

    /* Cada base conhece os tipos de seus campos e como converter 'valor'. */
    if (tipo == TIPO_NASA)
        return comparar_item_nasa_valor(item, campo, valor);

    if (tipo == TIPO_DETRAN)
        return comparar_item_detran_valor(item, campo, valor);

    return COMPARACAO_FALHOU;
}