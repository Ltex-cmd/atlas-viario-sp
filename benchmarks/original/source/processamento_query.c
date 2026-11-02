#include <stdbool.h>
#include <string.h>

#include "processamento_query.h"
#include "comparador.h"

/*
Objetivo: Verificar se um registro atende a todos os criterios FILTER da consulta.
Cada filtro compara o campo do item com o valor indicado e aplica o operador
relacional solicitado. Os filtros sao combinados por E logico: basta um deles
falhar para que o registro seja rejeitado. Na ausencia de FILTER, aceita o item.

Parametros:
- item: Registro NASA ou DETRAN cujos campos serao avaliados.
- query: Consulta que pode conter uma ou mais diretivas FILTER.

Retorno: true quando o item e valido e satisfaz todos os filtros (ou nao
existem filtros); false para item/consulta invalidos, campos ausentes,
comparacoes impossiveis ou algum filtro nao satisfeito.
*/
bool item_satisfaz_filter(ITEM *item, QUERY *query){

    if (item == NULL ||
        (item_get_tipo(item) != TIPO_NASA && item_get_tipo(item) != TIPO_DETRAN) ||
        query == NULL)
        return false;

    /* Sem FILTER, nao existe restricao adicional para o registro. */
    if (query_get_possui_filtros(query) == false)
        return true;

    int quantidade_filtros = query_get_quantidade_filtros(query);

    /* Todos os filtros devem passar: interrompe na primeira rejeicao. */
    for (int i = 0; i < quantidade_filtros; i++){

        const char *campo = query_get_filtro_campo(query, i);
        const char *operador = query_get_filtro_operador(query, i);
        const char *valor = query_get_filtro_valor(query, i);

        if (campo == NULL ||
            operador == NULL ||
            valor == NULL)
            return false;

        if (validar_valor(item, campo) != 0)
            return false;

        int comparacao = comparar_item_valor(item, campo, valor);

        if (comparacao == COMPARACAO_FALHOU)
            return false;

        /* Traduz o resultado da comparacao para o operador do filtro. */
        if (strcmp(operador, "<") == 0){

            if (comparacao >= 0)
                return false;
        }

        else if (strcmp(operador, "<=") == 0){

            if (comparacao > 0)
                return false;
        }

        else if (strcmp(operador, "=") == 0 ||
                 strcmp(operador, "==") == 0){

            if (comparacao != 0)
                return false;
        }

        else if (strcmp(operador, "!=") == 0){

            if (comparacao == 0)
                return false;
        }

        else if (strcmp(operador, ">=") == 0){

            if (comparacao < 0)
                return false;
        }

        else if (strcmp(operador, ">") == 0){

            if (comparacao <= 0)
                return false;
        }

        else{

            return false;
        }
    }

    return true;
}

/*
Objetivo: Verificar se o registro alcanca a similaridade minima configurada
pela diretiva ACCEPT. Quando a nota ainda nao foi calculada, solicita seu
calculo usando os criterios SIM armazenados na consulta. Sem ACCEPT, aceita
o registro sem exigir uma nota de similaridade.

Parametros:
- item: Registro NASA ou DETRAN que sera avaliado.
- query: Consulta contendo, quando aplicavel, SIM e o limiar ACCEPT.

Retorno: true se nao houver ACCEPT ou se a nota valida do registro for maior
ou igual ao limiar; false para entradas invalidas, falta de SIM quando
necessario, falha de calculo, nota fora de [0,1] ou nota abaixo do limiar.
*/
bool item_satisfaz_accept(ITEM *item, QUERY *query){

    if (item == NULL ||
        (item_get_tipo(item) != TIPO_NASA &&
         item_get_tipo(item) != TIPO_DETRAN) ||
        query == NULL)
        return false;

    if (query_get_possui_accept(query) == false)
        return true;

    if (query_get_possui_similaridade(query) == false)
        return false;

    /* Reaproveita a similaridade calculada anteriormente, quando existir. */
    double similaridade = item_get_similaridade(item);

    /* Calcula sob demanda apenas quando ainda nao ha nota armazenada. */    
    if (similaridade == SIMILARIDADE_NAO_CALCULADA){

        if (!item_calcular_similaridade(item, query))
            return false;

        similaridade = item_get_similaridade(item);
    }

    /* Rejeita notas invalidas; a expressao tambem rejeita NaN. */
    if (!(similaridade >= 0 && similaridade <= 1))
        return false;

    double accept = query_get_accept(query);

    if (similaridade >= accept)
        return true;

    return false;
}

/*
Objetivo: Comparar dois registros para a ordenacao solicitada por ORDER.
Interpreta ASC/DESC e compara o campo escolhido ou a nota de similaridade.
No criterio similarity, calcula notas ainda indisponiveis e posiciona notas
invalidas depois das validas. Para outros campos, considera valores ausentes
maiores que os presentes, conforme a regra usada pela comparacao generica.

Parametros:
- item1: Primeiro registro a ser ordenado.
- item2: Segundo registro a ser ordenado.
- query: Consulta que define o criterio e a direcao ORDER; pode conter SIM.

Retorno: -1 se o primeiro deve vir antes, 0 se sao equivalentes no criterio
ou 1 se deve vir depois; COMPARACAO_FALHOU (-2) para entradas ou criterios
invalidos. A inversao de sinal em DESC ocorre nas comparacoes validas.
*/
int comparar_itens_order(ITEM *item1, ITEM *item2, QUERY *query){

    if (item1 == NULL || item2 == NULL || query == NULL)
        return COMPARACAO_FALHOU;

    if (query_get_possui_ordenacao(query) == false)
        return COMPARACAO_FALHOU;

    const char *campo = query_get_ordenacao_criterio(query);
    const char *direcao = query_get_ordenacao_direcao(query);

    if (campo == NULL || direcao == NULL)
        return COMPARACAO_FALHOU;

    bool decrescente;

    /* A direcao determina se a ordem da comparacao sera invertida. */
    if (strcmp(direcao, "ASC") == 0)
        decrescente = false;
    else if (strcmp(direcao, "DESC") == 0)
        decrescente = true;
    else
        return COMPARACAO_FALHOU;

    int comparacao;

    /* ORDER por similarity usa a nota calculada pelos criterios SIM. */
    if (strcmp(campo, "similarity") == 0){

        if (query_get_possui_similaridade(query) == false)
            return COMPARACAO_FALHOU;

        double valor1 = item_get_similaridade(item1);
        double valor2 = item_get_similaridade(item2);

        if (valor1 == SIMILARIDADE_NAO_CALCULADA){

            if (item_calcular_similaridade(item1, query))
                valor1 = item_get_similaridade(item1);
            else
                valor1 = SIMILARIDADE_FALHOU;
        }

        if (valor2 == SIMILARIDADE_NAO_CALCULADA){

            if (item_calcular_similaridade(item2, query))
                valor2 = item_get_similaridade(item2);
            else
                valor2 = SIMILARIDADE_FALHOU;
        }

        bool valido1 = valor1 >= 0 && valor1 <= 1;
        bool valido2 = valor2 >= 0 && valor2 <= 1;

        if (!valido1 && !valido2) return 0;
        if (!valido1) return 1;
        if (!valido2) return -1;

        if (valor1 > valor2) comparacao = 1;
        else if (valor1 == valor2) comparacao = 0;
        else comparacao = -1;
    }
    else{

        /* Para campos comuns, identifica ausencias antes de comparar valores. */
        int estado1 = validar_valor(item1, campo);
        int estado2 = validar_valor(item2, campo);

        if (estado1 == 2 || estado2 == 2)
            return COMPARACAO_FALHOU;

        /* Estado 1: ausente; estado 2: campo invalido (checado acima). */     
        if (estado1 == 1 && estado2 == 1) return 0;
        if (estado1 == 1) return 1;
        if (estado2 == 1) return -1;

        comparacao = comparar_itens(item1, item2, campo);
        if (comparacao == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;
    }
    /* Inverte a ordem de comparacoes validas ao selecionar DESC. */
    if (decrescente)
        return -comparacao;

    return comparacao;
}