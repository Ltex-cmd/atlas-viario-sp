#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "item.h"
#include "item_nasa.h"
#include "item_detran.h"

struct item_ {

    int tipo_dado;
    void *dado;
    char *linha_original;
    double similaridade;

};

/*
Objetivo: Identificar a base a qual pertence um registro generico, permitindo
que as demais funcoes escolham o tratamento NASA ou DETRAN.

Parametros:
- item: Registro cujo identificador de tipo sera consultado.

Retorno: Tipo armazenado (TIPO_NASA ou TIPO_DETRAN), ou 0 se item for NULL.
*/
int item_get_tipo(ITEM *item){

    if (item == NULL)
        return 0;

    return item->tipo_dado;
}

/*
Objetivo: Liberar completamente um registro, delegando a liberacao dos dados
especificos ao modulo NASA ou DETRAN e depois liberando a linha CSV e o
envoltorio ITEM. Ao final, evita deixar o ponteiro externo pendente.

Parametros:
- item: Endereco do ponteiro para o ITEM que sera destruido; recebe NULL
  apos a liberacao.
*/
void item_apagar(ITEM **item){

    if (item == NULL || *item == NULL)
        return;

    if ((*item)->tipo_dado == TIPO_NASA)
        item_apagar_nasa(&(*item)->dado);

    if ((*item)->tipo_dado == TIPO_DETRAN)
        item_apagar_detran(&(*item)->dado);

    free((*item)->linha_original);
    free(*item);
    *item = NULL;
}

/*
Objetivo: Escrever em um arquivo a linha CSV original armazenada no ITEM,
sem reconstruir seus campos ou alterar seu formato.

Parametros:
- item: Registro que contem a copia da linha CSV original.
- arquivo: Arquivo de saida ja aberto para escrita.

Retorno: true se a escrita nao apresentar erro; false para parametros
invalidos ou falhas de escrita.
*/
bool imprimir_item_arquivo(ITEM *item, FILE *arquivo){

    if (item == NULL || arquivo == NULL || item->linha_original == NULL)
        return false;

    if (fputs(item->linha_original, arquivo) == EOF)
        return false;

    if (ferror(arquivo))
        return false;

    return true;
}

/*
Objetivo: Verificar se uma determinada coluna possui um valor textual na
linha CSV original. Percorre os delimitadores fora de aspas, sem modificar
a linha nem criar copias. As aspas sao consideradas apenas para localizar os delimitadores.
E os campos com espacos ou duas aspas nao sao considerados vazios. 

Parametros:
- linha: Registro CSV original que sera examinado.
- indice: Indice da coluna desejada, contado a partir de zero.
- delimitador: Caractere separador das colunas (',' na NASA; ';' no DETRAN).

Retorno: VALOR_PRESENTE quando a coluna nao esta vazia ou VALOR_AUSENTE
quando nao e encontrada ou comeca diretamente em um delimitador/fim de linha.
*/
int validar_valor_csv(const char *linha, int indice, char delimitador){

    if (linha == NULL || indice < 0)
        return VALOR_AUSENTE;

    const char *p = linha;
    int coluna = 0;
    bool dentro_de_aspas = false; /* Delimitadores entre aspas nao encerram coluna. */

    while (coluna < indice){

        if (*p == '\0' || *p == '\r' || *p == '\n')
            return VALOR_AUSENTE;

        if (*p == '"')
            dentro_de_aspas = !dentro_de_aspas;

        if (*p == delimitador && !dentro_de_aspas) /* Ignora separadores dentro de aspas. */
            coluna++;

        p++;
    }

    /* Coluna vazia: o primeiro caractere ja e separador ou fim de linha. */
    if (*p == delimitador || *p == '\0' || *p == '\r' || *p == '\n')
        return VALOR_AUSENTE;

    return VALOR_PRESENTE;
}

/*
Objetivo: Localizar o nome de um campo no esquema NASA ou DETRAN e verificar
se o registro possui valor para essa coluna no CSV original. Campos Sentry
na NASA exigem participacao na lista; campos de quantidade do DETRAN
(com prefixo qtd_) sao considerados presentes mesmo quando vazios.

Parametros:
- item: Registro NASA ou DETRAN cujo campo sera consultado.
- campo: Nome da coluna a verificar.

Retorno: VALOR_PRESENTE (0), VALOR_AUSENTE (1) ou CAMPO_INVALIDO (2),
conforme o campo seja valido e preenchido, valido e ausente ou desconhecido.
*/
int validar_valor(ITEM *item, const char *campo){

    if (item == NULL || campo == NULL || campo[0] == '\0')
        return CAMPO_INVALIDO;

    if (item_get_tipo(item) == TIPO_NASA){

        const char *campos[] = {
            "asteroid_designation",
            "asteroid_fullname",
            "close_approach_date",
            "year",
            "month",
            "distance_au",
            "velocity_km_s",
            "absolute_magnitude",
            "days_until_approach",
            "is_past_event",
            "is_future_event",
            "risk_score",
            "panic_level",
            "threat_category",
            "panic_verdict",
            "on_sentry_list",
            "sentry_impact_prob",
            "sentry_torino_scale",
            "sentry_palermo_scale",
            "sentry_diameter_km"
        };

        for (int i = 0; i < 20; i++){

            if (strcmp(campo, campos[i]) == 0){

                if (i >= 16 && !item_nasa_get_on_sentry_list(item))
                    return VALOR_AUSENTE;

                return validar_valor_csv(item->linha_original, i, ',');
            }
        }
    }
    else if (item_get_tipo(item) == TIPO_DETRAN){

        const char *campos[] = {
            "id_sinistro",
            "tipo_registro",
            "data_sinistro",
            "ano_sinistro",
            "mes_sinistro",
            "dia_sinistro",
            "hora_sinistro",
            "ano_mes_sinistro",
            "dia_da_semana",
            "turno",
            "logradouro",
            "numero_logradouro",
            "tipo_via",
            "tipo_local",
            "latitude",
            "longitude",
            "cod_ibge",
            "municipio",
            "regiao_administrativa",
            "administracao",
            "conservacao",
            "circunscricao",
            "tp_sinistro_primario",
            "qtd_pedestre",
            "qtd_bicicleta",
            "qtd_motocicleta",
            "qtd_automovel",
            "qtd_onibus",
            "qtd_caminhao",
            "qtd_veic_outros",
            "qtd_veic_nao_disponivel",
            "qtd_gravidade_fatal",
            "qtd_gravidade_grave",
            "qtd_gravidade_leve",
            "qtd_gravidade_ileso",
            "qtd_gravidade_nao_disponivel",
            "tp_sinistro_atrop_pedestre",
            "tp_sinistro_atrop_vitima_fora_veic",
            "tp_sinistro_colisao_frontal",
            "tp_sinistro_colisao_traseira",
            "tp_sinistro_colisao_lateral",
            "tp_sinistro_colisao_transversal",
            "tp_sinistro_colisao_outros",
            "tp_sinistro_choque",
            "tp_sinistro_atrop_animal",
            "tp_sinistro_capotamento",
            "tp_sinistro_engavetamento",
            "tp_sinistro_tombamento",
            "tp_sinistro_outros",
            "tp_sinistro_nao_disponivel"
        };

        for (int i = 0; i < 50; i++){

            if (strcmp(campo, campos[i]) == 0){

                /* Quantidade vazia no DETRAN significa zero. */
                if (strncmp(campo, "qtd_", 4) == 0)
                    return VALOR_PRESENTE;

                return validar_valor_csv(item->linha_original, i, ';');
            }
        }
    }

    return CAMPO_INVALIDO;
}

/*
Objetivo: Encaminhar a consulta de um campo numerico para a implementacao
correspondente ao tipo do registro (NASA ou DETRAN).

Parametros:
- item: Registro que contem o campo numerico solicitado.
- campo: Nome do atributo a ser consultado.
- valor: Ponteiro de saida que recebe o numero em double quando a leitura
  e realizada com sucesso.

Retorno: true quando foi possivel obter o campo; false para parametros
invalidos, tipo desconhecido ou falha no leitor especifico.
*/
bool obter_valor_numerico_item(ITEM *item, const char *campo, double *valor){

    if (item == NULL || campo == NULL || valor == NULL)
        return false;

    if (item_get_tipo(item) == TIPO_NASA)
        return obter_valor_numerico_nasa(item, campo, valor);

    if (item_get_tipo(item) == TIPO_DETRAN)
        return obter_valor_numerico_detran(item, campo, valor);

    return false;
}

/*
Objetivo: Calcular a similaridade entre o valor numerico de um campo e um
valor-alvo. A nota e 1 quando sao iguais e diminui linearmente conforme
a distancia aumenta, chegando a 0 fora da tolerancia. Usa a expressao
max(0, 1 - |valor - alvo| / tolerancia).

Parametros:
- item: Registro do qual sera extraido o numero a comparar.
- campo: Nome do campo numerico usado na comparacao.
- alvo: Valor de referencia para a similaridade maxima.
- tolerancia: Distancia positiva que determina quando a nota chega a zero.

Retorno: Similaridade entre 0 e 1 ou SIMILARIDADE_FALHOU quando nao foi
possivel calcular a nota.
*/
double calcular_similaridade(ITEM *item, const char *campo, double alvo, double tolerancia){

    if (item == NULL || campo == NULL || !(tolerancia > 0))
        return SIMILARIDADE_FALHOU;

    double valor;

    if (!obter_valor_numerico_item(item, campo, &valor)){
        item->similaridade = SIMILARIDADE_FALHOU;
        return SIMILARIDADE_FALHOU;
    }
    
    double diferenca = valor - alvo;

    if (diferenca < 0)
        diferenca = -diferenca;

    /* Nota igual a 1 no alvo e decrescimo linear ate a tolerancia. */
    double similaridade = 1.0 - diferenca / tolerancia;

    if (similaridade < 0) /* Distancias maiores que a tolerancia ficam em zero. */
        similaridade = 0;

    return similaridade;
}

/*
Objetivo: Calcular a similaridade final de um registro com uma consulta,
combinando as notas de todos os criterios por media ponderada. Valida
campos, tolerancias e pesos e guarda o resultado dentro do proprio ITEM.

Parametros:
- item: Registro que tera o atributo similaridade atualizado.
- query: Consulta contendo campos, alvos, tolerancias e pesos para cada
  criterio de similaridade.

Retorno: true se a media ponderada foi calculada e armazenada; false se
os dados forem invalidos, algum criterio falhar ou a soma dos pesos
nao for positiva. Nas falhas tratadas, grava SIMILARIDADE_FALHOU.
*/
bool item_calcular_similaridade(ITEM *item, QUERY *query){

    if (item == NULL)
        return false;

    if (query == NULL ||
        (item->tipo_dado != TIPO_NASA &&
         item->tipo_dado != TIPO_DETRAN)){
        item->similaridade = SIMILARIDADE_FALHOU;
        return false;
    }

    if (!query_get_possui_similaridade(query)){
        item->similaridade = SIMILARIDADE_FALHOU;
        return false;
    }

    /* Uma consulta pode combinar varios criterios de semelhanca. */
    int quantidade = query_get_quantidade_similaridades(query);

    if (quantidade <= 0){
        item->similaridade = SIMILARIDADE_FALHOU;
        return false;
    }

    double soma_similaridades = 0;
    double soma_pesos = 0;

    for (int i = 0; i < quantidade; i++){

        const char *campo = query_get_similaridade_campo(query, i);
        double alvo = query_get_similaridade_alvo(query, i);
        double tolerancia = query_get_similaridade_tolerancia(query, i);
        double peso = query_get_similaridade_peso(query, i);

        if (campo == NULL || *campo == '\0' || (tolerancia <= 0) || (peso < 0)){
            item->similaridade = SIMILARIDADE_FALHOU;
            return false;
        }

        double similaridade = calcular_similaridade(item, campo, alvo, tolerancia);

        if (!(similaridade >= 0 && similaridade <= 1)){
            item->similaridade = SIMILARIDADE_FALHOU;
            return false;
        }

        soma_similaridades += similaridade * peso;
        soma_pesos += peso;
    }

    if (!(soma_pesos > 0)){
        item->similaridade = SIMILARIDADE_FALHOU;
        return false;
    }

    double resultado = soma_similaridades / soma_pesos;

    if (!(resultado >= 0 && resultado <= 1)){
        item->similaridade = SIMILARIDADE_FALHOU;
        return false;
    }
    
    item->similaridade = resultado;

    return true;
}

double item_get_similaridade(ITEM *item){

    if (item == NULL)
        return SIMILARIDADE_FALHOU;
    
    return item->similaridade;
}
