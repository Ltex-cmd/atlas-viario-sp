#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comparador_detran.h"
#include "item.h"
#include "item_detran.h"

/* Os resultados validos sao -1 (menor), 0 (igual) e 1 (maior). */

/*
Objetivo: Comparar duas datas no formato DD/MM/AAAA em ordem cronologica.
Verifica apenas o formato (posicao das barras e digitos) e compara ano,
mes e dia, nessa ordem; nao verifica se a data existe no calendario.

Parametros:
- data1: Primeira data a comparar, no formato DD/MM/AAAA.
- data2: Segunda data a comparar, no mesmo formato.

Retorno: -1 se data1 for anterior, 0 se forem iguais, 1 se for posterior
ou COMPARACAO_FALHOU (-2) se o formato estiver incorreto.
*/
int comparar_data(const char *data1, const char *data2){

    if (data1 == NULL || data2 == NULL ||
        strlen(data1) != 10 || strlen(data2) != 10)
        return COMPARACAO_FALHOU;

    /* Confere DD/MM/AAAA, sem avaliar limites de dia e mes. */
    for (int i = 0; i < 10; i++){

        if (i == 2 || i == 5){
            if (data1[i] != '/' || data2[i] != '/')
                return COMPARACAO_FALHOU;
        }
        else if (data1[i] < '0' || data1[i] > '9' ||
                 data2[i] < '0' || data2[i] > '9')
            return COMPARACAO_FALHOU;
    }

    /* Comparar a partir do indice 6 equivale a comparar os anos. */
    int comparacao = strcmp(data1 + 6, data2 + 6);
    if (comparacao > 0) return 1;
    if (comparacao < 0) return -1;

    /* Se o ano empatou, desempata pelo mes e depois pelo dia. */
    char mes1[3] = {data1[3], data1[4], '\0'};
    char mes2[3] = {data2[3], data2[4], '\0'};

    comparacao = strcmp(mes1, mes2);
    if (comparacao > 0) return 1;
    if (comparacao < 0) return -1;

    char dia1[3] = {data1[0], data1[1], '\0'};
    char dia2[3] = {data2[0], data2[1], '\0'};

    comparacao = strcmp(dia1, dia2);
    if (comparacao > 0) return 1;
    if (comparacao < 0) return -1;

    return 0;
}

/*
Objetivo: Converter uma string numerica em double, trocando virgulas por
pontos em uma copia temporaria do texto. Preserva a string original e
libera a copia apos a conversao por atof.

Parametros:
- texto: Representacao textual do numero, possivelmente com virgula decimal.

Retorno: Numero convertido ou 0 quando texto e NULL ou a alocacao falha.
Como usa atof, entradas numericas invalidas tambem podem resultar em 0.
*/
double converter_double(const char *texto){ //no arquivo, os numeros tipo float e double usam virgula, ao inves de ponto, portanto precisa trocar esse caracter para usar funcoes como atof//

    if (texto == NULL)
        return 0;

    /* Usa uma copia para nao alterar o texto original da consulta. */
    char *copia = malloc(strlen(texto) + 1);
    if (copia == NULL)
        return 0;

    strcpy(copia, texto);
    for (int i = 0; copia[i] != '\0'; i++){
        if (copia[i] == ',') /* atof usa ponto decimal no formato esperado. */
            copia[i] = '.';
    }

    /* atof nao distingue texto invalido de um zero numerico. */
    double resultado = atof(copia);
    free(copia);

    return resultado;
}

/*
Objetivo: Transformar o nome de um dia da semana em uma posicao numerica,
para permitir comparacoes pela ordem dos dias (Domingo = 0 ate Sabado = 6).
Aceita algumas variacoes de acentuacao encontradas nos dados.

Parametros:
- dia: Nome do dia da semana registrado no CSV do DETRAN.

Retorno: Posicao de 0 a 6 ou COMPARACAO_FALHOU se nao reconhecer o texto.
*/
int ordem_dia_da_semana(const char *dia){

    if (dia == NULL)
        return COMPARACAO_FALHOU;

    if (strcmp(dia, "Domingo") == 0) return 0;
    if (strcmp(dia, "Segunda-feira") == 0) return 1;
    if (strcmp(dia, "Terca-feira") == 0 || strcmp(dia, "Terça-feira") == 0 || strcmp(dia, "Ter\347a-feira") == 0) return 2;
    if (strcmp(dia, "Quarta-feira") == 0) return 3;
    if (strcmp(dia, "Quinta-feira") == 0) return 4;
    if (strcmp(dia, "Sexta-feira") == 0) return 5;
    if (strcmp(dia, "Sabado") == 0 || strcmp(dia, "Sábado") == 0 || strcmp(dia, "S\341bado") == 0) return 6;

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Associar cada turno do dia a um numero, estabelecendo a ordem
MADRUGADA, MANHA, TARDE, NOITE e NAO DISPONIVEL para comparacoes.

Parametros:
- turno: Nome do turno do sinistro, conforme os valores esperados no CSV.

Retorno: Posicao de 0 a 4 ou COMPARACAO_FALHOU para valor desconhecido.
*/
int ordem_turno(const char *turno){

    if (turno == NULL)
        return COMPARACAO_FALHOU;

    if (strcmp(turno, "MADRUGADA") == 0) return 0;
    if (strcmp(turno, "MANHA") == 0) return 1;
    if (strcmp(turno, "TARDE") == 0) return 2;
    if (strcmp(turno, "NOITE") == 0) return 3;
    if (strcmp(turno, "NAO DISPONIVEL") == 0) return 4;

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Interpretar valores textuais usados nas consultas como booleanos.
Reconhece true, True e S como verdadeiro; false, False e string vazia
como falso. Valores nao reconhecidos sao rejeitados.

Parametros:
- texto: String que representa o valor booleano fornecido na consulta.
- resultado: Ponteiro de saida que recebe o booleano, caso a conversao
  seja valida; nao e alterado em caso de erro.

Retorno: true para conversao bem-sucedida ou false para entrada invalida.
*/
bool converter_bool(const char *texto, bool *resultado){

    if (texto == NULL || resultado == NULL)
        return false;

    if (strcmp(texto, "true") == 0 || strcmp(texto, "True") == 0 ||
        strcmp(texto, "S") == 0)
        *resultado = true;

    else if (strcmp(texto, "false") == 0 || strcmp(texto, "False") == 0 ||
             strcmp(texto, "") == 0)
        *resultado = false;

    else
        return false;

    return true;
}

/*
Objetivo: Comparar o conteudo de um campo em dois registros do DETRAN.
Escolhe a comparacao de acordo com o tipo do campo: inteiros e doubles
numericamente, textos por strcmp, datas cronologicamente, dias e turnos
pela ordem definida, e indicadores booleanos por false/true.

Parametros:
- item1: Primeiro registro ITEM, obrigatoriamente do tipo DETRAN.
- item2: Segundo registro ITEM, tambem do tipo DETRAN.
- campo: Nome do atributo do sinistro usado como criterio de comparacao.

Retorno: -1 se o primeiro valor for menor, 0 se igual, 1 se maior;
COMPARACAO_FALHOU (-2) para dados, campo ou formato nao reconhecidos.
*/
int comparar_itens_detran(ITEM *item1, ITEM *item2, const char *campo){

    if (item1 == NULL || item2 == NULL || campo == NULL ||
        item_get_tipo(item1) != TIPO_DETRAN || item_get_tipo(item2) != TIPO_DETRAN)
        return COMPARACAO_FALHOU;

    /* Campos numericos: comparacao por grandeza, nao por texto. */
    if (strcmp(campo, "id_sinistro") == 0){

        int valor1 = item_detran_get_id_sinistro(item1);
        int valor2 = item_detran_get_id_sinistro(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    /* Campos textuais: strcmp aplica ordem lexicografica. */
    if (strcmp(campo, "tipo_registro") == 0){

        const char *valor1 = item_detran_get_tipo_registro(item1);
        const char *valor2 = item_detran_get_tipo_registro(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }
    
    /* Data em DD/MM/AAAA precisa ser ordenada por ano, mes e dia. */
    if (strcmp(campo, "data_sinistro") == 0){

        const char *valor1 = item_detran_get_data_sinistro(item1);
        const char *valor2 = item_detran_get_data_sinistro(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        return comparar_data(valor1, valor2);

    }

    if (strcmp(campo, "ano_sinistro") == 0){

        int valor1 = item_detran_get_ano_sinistro(item1);
        int valor2 = item_detran_get_ano_sinistro(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "mes_sinistro") == 0){

        int valor1 = item_detran_get_mes_sinistro(item1);
        int valor2 = item_detran_get_mes_sinistro(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "dia_sinistro") == 0){

        int valor1 = item_detran_get_dia_sinistro(item1);
        int valor2 = item_detran_get_dia_sinistro(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "hora_sinistro") == 0){

        const char *valor1 = item_detran_get_hora_sinistro(item1);
        const char *valor2 = item_detran_get_hora_sinistro(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "ano_mes_sinistro") == 0){

        const char *valor1 = item_detran_get_ano_mes_sinistro(item1);
        const char *valor2 = item_detran_get_ano_mes_sinistro(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    /* Nomes dos dias seguem a ordem semanal, nao a alfabetica. */
    if (strcmp(campo, "dia_da_semana") == 0){

        const char *valor1 = item_detran_get_dia_da_semana(item1);
        const char *valor2 = item_detran_get_dia_da_semana(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int ordem1 = ordem_dia_da_semana(valor1);
        int ordem2 = ordem_dia_da_semana(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    /* Turnos sao convertidos em posicoes da sequencia do dia. */
    if (strcmp(campo, "turno") == 0){

        const char *valor1 = item_detran_get_turno(item1);
        const char *valor2 = item_detran_get_turno(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int ordem1 = ordem_turno(valor1);
        int ordem2 = ordem_turno(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "logradouro") == 0){

        const char *valor1 = item_detran_get_logradouro(item1);
        const char *valor2 = item_detran_get_logradouro(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "numero_logradouro") == 0){

        double valor1 = item_detran_get_numero_logradouro(item1);
        double valor2 = item_detran_get_numero_logradouro(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tipo_via") == 0){

        const char *valor1 = item_detran_get_tipo_via(item1);
        const char *valor2 = item_detran_get_tipo_via(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "tipo_local") == 0){

        const char *valor1 = item_detran_get_tipo_local(item1);
        const char *valor2 = item_detran_get_tipo_local(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "latitude") == 0){

        double valor1 = item_detran_get_latitude(item1);
        double valor2 = item_detran_get_latitude(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "longitude") == 0){

        double valor1 = item_detran_get_longitude(item1);
        double valor2 = item_detran_get_longitude(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "cod_ibge") == 0){

        int valor1 = item_detran_get_cod_ibge(item1);
        int valor2 = item_detran_get_cod_ibge(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "municipio") == 0){

        const char *valor1 = item_detran_get_municipio(item1);
        const char *valor2 = item_detran_get_municipio(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "regiao_administrativa") == 0){

        const char *valor1 = item_detran_get_regiao_administrativa(item1);
        const char *valor2 = item_detran_get_regiao_administrativa(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "administracao") == 0){

        const char *valor1 = item_detran_get_administracao(item1);
        const char *valor2 = item_detran_get_administracao(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "conservacao") == 0){

        const char *valor1 = item_detran_get_conservacao(item1);
        const char *valor2 = item_detran_get_conservacao(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "circunscricao") == 0){

        const char *valor1 = item_detran_get_circunscricao(item1);
        const char *valor2 = item_detran_get_circunscricao(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_primario") == 0){

        const char *valor1 = item_detran_get_tp_sinistro_primario(item1);
        const char *valor2 = item_detran_get_tp_sinistro_primario(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_pedestre") == 0){

        int valor1 = item_detran_get_qtd_pedestre(item1);
        int valor2 = item_detran_get_qtd_pedestre(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_bicicleta") == 0){

        int valor1 = item_detran_get_qtd_bicicleta(item1);
        int valor2 = item_detran_get_qtd_bicicleta(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_motocicleta") == 0){

        int valor1 = item_detran_get_qtd_motocicleta(item1);
        int valor2 = item_detran_get_qtd_motocicleta(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_automovel") == 0){

        int valor1 = item_detran_get_qtd_automovel(item1);
        int valor2 = item_detran_get_qtd_automovel(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_onibus") == 0){

        int valor1 = item_detran_get_qtd_onibus(item1);
        int valor2 = item_detran_get_qtd_onibus(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_caminhao") == 0){

        int valor1 = item_detran_get_qtd_caminhao(item1);
        int valor2 = item_detran_get_qtd_caminhao(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_veic_outros") == 0){

        int valor1 = item_detran_get_qtd_veic_outros(item1);
        int valor2 = item_detran_get_qtd_veic_outros(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_veic_nao_disponivel") == 0){

        int valor1 = item_detran_get_qtd_veic_nao_disponivel(item1);
        int valor2 = item_detran_get_qtd_veic_nao_disponivel(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    /* Gravidade tambem e comparada pelas quantidades registradas. */
    if (strcmp(campo, "qtd_gravidade_fatal") == 0){

        int valor1 = item_detran_get_qtd_gravidade_fatal(item1);
        int valor2 = item_detran_get_qtd_gravidade_fatal(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_grave") == 0){

        int valor1 = item_detran_get_qtd_gravidade_grave(item1);
        int valor2 = item_detran_get_qtd_gravidade_grave(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_leve") == 0){

        int valor1 = item_detran_get_qtd_gravidade_leve(item1);
        int valor2 = item_detran_get_qtd_gravidade_leve(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_ileso") == 0){

        int valor1 = item_detran_get_qtd_gravidade_ileso(item1);
        int valor2 = item_detran_get_qtd_gravidade_ileso(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_nao_disponivel") == 0){

        int valor1 = item_detran_get_qtd_gravidade_nao_disponivel(item1);
        int valor2 = item_detran_get_qtd_gravidade_nao_disponivel(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_pedestre") == 0){

        bool valor1 = item_detran_get_tp_sinistro_atrop_pedestre(item1);
        bool valor2 = item_detran_get_tp_sinistro_atrop_pedestre(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_vitima_fora_veic") == 0){

        bool valor1 = item_detran_get_tp_sinistro_atrop_vitima_fora_veic(item1);
        bool valor2 = item_detran_get_tp_sinistro_atrop_vitima_fora_veic(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_frontal") == 0){

        bool valor1 = item_detran_get_tp_sinistro_colisao_frontal(item1);
        bool valor2 = item_detran_get_tp_sinistro_colisao_frontal(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_traseira") == 0){

        bool valor1 = item_detran_get_tp_sinistro_colisao_traseira(item1);
        bool valor2 = item_detran_get_tp_sinistro_colisao_traseira(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_lateral") == 0){

        bool valor1 = item_detran_get_tp_sinistro_colisao_lateral(item1);
        bool valor2 = item_detran_get_tp_sinistro_colisao_lateral(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_transversal") == 0){

        bool valor1 = item_detran_get_tp_sinistro_colisao_transversal(item1);
        bool valor2 = item_detran_get_tp_sinistro_colisao_transversal(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_outros") == 0){

        bool valor1 = item_detran_get_tp_sinistro_colisao_outros(item1);
        bool valor2 = item_detran_get_tp_sinistro_colisao_outros(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_choque") == 0){

        bool valor1 = item_detran_get_tp_sinistro_choque(item1);
        bool valor2 = item_detran_get_tp_sinistro_choque(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_animal") == 0){

        bool valor1 = item_detran_get_tp_sinistro_atrop_animal(item1);
        bool valor2 = item_detran_get_tp_sinistro_atrop_animal(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_capotamento") == 0){

        bool valor1 = item_detran_get_tp_sinistro_capotamento(item1);
        bool valor2 = item_detran_get_tp_sinistro_capotamento(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_engavetamento") == 0){

        bool valor1 = item_detran_get_tp_sinistro_engavetamento(item1);
        bool valor2 = item_detran_get_tp_sinistro_engavetamento(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_tombamento") == 0){

        bool valor1 = item_detran_get_tp_sinistro_tombamento(item1);
        bool valor2 = item_detran_get_tp_sinistro_tombamento(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_outros") == 0){

        bool valor1 = item_detran_get_tp_sinistro_outros(item1);
        bool valor2 = item_detran_get_tp_sinistro_outros(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_nao_disponivel") == 0){

        bool valor1 = item_detran_get_tp_sinistro_nao_disponivel(item1);
        bool valor2 = item_detran_get_tp_sinistro_nao_disponivel(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Comparar um campo de um registro DETRAN com um valor textual
informado em uma consulta. Converte o texto conforme o tipo do campo
(inteiro, double ou booleano) e usa as regras especiais de data,
dia da semana e turno quando necessario.

Parametros:
- item: Registro ITEM do tipo DETRAN a ser consultado.
- campo: Nome do atributo do registro que sera comparado.
- valor: Texto com o valor de referencia da consulta, a ser interpretado
  segundo o tipo do atributo.

Retorno: -1 se o valor do item for menor, 0 se igual, 1 se maior;
COMPARACAO_FALHOU (-2) nos erros detectados. atoi/atof nao sinalizam
valores numericos invalidos, que podem ser convertidos para zero.
*/
int comparar_item_detran_valor(ITEM *item, const char *campo, const char *valor){

    if (item == NULL || campo == NULL || valor == NULL ||
        item_get_tipo(item) != TIPO_DETRAN)
        return COMPARACAO_FALHOU;

    /* A referencia numerica chega como texto e e convertida com atoi. */    
    if (strcmp(campo, "id_sinistro") == 0){

        int valor1 = item_detran_get_id_sinistro(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    /* Textos sao comparados diretamente com o literal da consulta. */
    if (strcmp(campo, "tipo_registro") == 0){

        const char *valor1 = item_detran_get_tipo_registro(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    /* Datas precisam de comparacao cronologica, nao strcmp simples. */
    if (strcmp(campo, "data_sinistro") == 0){

        const char *valor1 = item_detran_get_data_sinistro(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        return comparar_data(valor1, valor2);
    }

    if (strcmp(campo, "ano_sinistro") == 0){

        int valor1 = item_detran_get_ano_sinistro(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "mes_sinistro") == 0){

        int valor1 = item_detran_get_mes_sinistro(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "dia_sinistro") == 0){

        int valor1 = item_detran_get_dia_sinistro(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "hora_sinistro") == 0){

        const char *valor1 = item_detran_get_hora_sinistro(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "ano_mes_sinistro") == 0){

        const char *valor1 = item_detran_get_ano_mes_sinistro(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "dia_da_semana") == 0){

        const char *valor1 = item_detran_get_dia_da_semana(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int ordem1 = ordem_dia_da_semana(valor1);
        int ordem2 = ordem_dia_da_semana(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "turno") == 0){

        const char *valor1 = item_detran_get_turno(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int ordem1 = ordem_turno(valor1);
        int ordem2 = ordem_turno(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "logradouro") == 0){

        const char *valor1 = item_detran_get_logradouro(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "numero_logradouro") == 0){

        double valor1 = item_detran_get_numero_logradouro(item);
        double valor2 = converter_double(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tipo_via") == 0){

        const char *valor1 = item_detran_get_tipo_via(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "tipo_local") == 0){

        const char *valor1 = item_detran_get_tipo_local(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "latitude") == 0){

        double valor1 = item_detran_get_latitude(item);
        double valor2 = converter_double(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "longitude") == 0){

        double valor1 = item_detran_get_longitude(item);
        double valor2 = converter_double(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "cod_ibge") == 0){

        int valor1 = item_detran_get_cod_ibge(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "municipio") == 0){

        const char *valor1 = item_detran_get_municipio(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "regiao_administrativa") == 0){

        const char *valor1 = item_detran_get_regiao_administrativa(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "administracao") == 0){

        const char *valor1 = item_detran_get_administracao(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "conservacao") == 0){

        const char *valor1 = item_detran_get_conservacao(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "circunscricao") == 0){

        const char *valor1 = item_detran_get_circunscricao(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_primario") == 0){

        const char *valor1 = item_detran_get_tp_sinistro_primario(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_pedestre") == 0){

        int valor1 = item_detran_get_qtd_pedestre(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_bicicleta") == 0){

        int valor1 = item_detran_get_qtd_bicicleta(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_motocicleta") == 0){

        int valor1 = item_detran_get_qtd_motocicleta(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_automovel") == 0){

        int valor1 = item_detran_get_qtd_automovel(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_onibus") == 0){

        int valor1 = item_detran_get_qtd_onibus(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_caminhao") == 0){

        int valor1 = item_detran_get_qtd_caminhao(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_veic_outros") == 0){

        int valor1 = item_detran_get_qtd_veic_outros(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_veic_nao_disponivel") == 0){

        int valor1 = item_detran_get_qtd_veic_nao_disponivel(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_fatal") == 0){

        int valor1 = item_detran_get_qtd_gravidade_fatal(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_grave") == 0){

        int valor1 = item_detran_get_qtd_gravidade_grave(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_leve") == 0){

        int valor1 = item_detran_get_qtd_gravidade_leve(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_ileso") == 0){

        int valor1 = item_detran_get_qtd_gravidade_ileso(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "qtd_gravidade_nao_disponivel") == 0){

        int valor1 = item_detran_get_qtd_gravidade_nao_disponivel(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_pedestre") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_atrop_pedestre(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_vitima_fora_veic") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_atrop_vitima_fora_veic(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_frontal") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_colisao_frontal(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_traseira") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_colisao_traseira(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_lateral") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_colisao_lateral(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_transversal") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_colisao_transversal(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_colisao_outros") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_colisao_outros(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_choque") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_choque(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_atrop_animal") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_atrop_animal(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_capotamento") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_capotamento(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_engavetamento") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_engavetamento(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_tombamento") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_tombamento(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_outros") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_outros(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "tp_sinistro_nao_disponivel") == 0){

        bool valor2;
        if (!converter_bool(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_detran_get_tp_sinistro_nao_disponivel(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    return COMPARACAO_FALHOU;
}