#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "item_detran.h"

struct item_ {

    int tipo_dado;
    void *dado;
    char *linha_original;
    double similaridade;

};

/* Representa os 50 campos de um registro de sinistro ja convertidos do CSV. */
typedef struct item_detran_ {

    int id_sinistro;
    char *tipo_registro;
    char *data_sinistro;
    int ano_sinistro;
    int mes_sinistro;
    int dia_sinistro;
    char *hora_sinistro;
    char *ano_mes_sinistro;
    char *dia_da_semana;
    char *turno;
    char *logradouro;
    double numero_logradouro;
    char *tipo_via;
    char *tipo_local;
    double latitude;
    double longitude;
    int cod_ibge;
    char *municipio;
    char *regiao_administrativa;
    char *administracao;
    char *conservacao;
    char *circunscricao;
    char *tp_sinistro_primario;
    int qtd_pedestre;
    int qtd_bicicleta;
    int qtd_motocicleta;
    int qtd_automovel;
    int qtd_onibus;
    int qtd_caminhao;
    int qtd_veic_outros;
    int qtd_veic_nao_disponivel;
    int qtd_gravidade_fatal;
    int qtd_gravidade_grave;
    int qtd_gravidade_leve;
    int qtd_gravidade_ileso;
    int qtd_gravidade_nao_disponivel;
    bool tp_sinistro_atrop_pedestre;
    bool tp_sinistro_atrop_vitima_fora_veic;
    bool tp_sinistro_colisao_frontal;
    bool tp_sinistro_colisao_traseira;
    bool tp_sinistro_colisao_lateral;
    bool tp_sinistro_colisao_transversal;
    bool tp_sinistro_colisao_outros;
    bool tp_sinistro_choque;
    bool tp_sinistro_atrop_animal;
    bool tp_sinistro_capotamento;
    bool tp_sinistro_engavetamento;
    bool tp_sinistro_tombamento;
    bool tp_sinistro_outros;
    bool tp_sinistro_nao_disponivel;

} ITEM_DETRAN;

/*
Objetivo: Construir um registro DETRAN a partir dos 50 campos ja
convertidos do CSV. Aloca o ITEM_DETRAN e o ITEM que o encapsula,
associa a linha original para exportacao e inicializa a similaridade.
Os ponteiros de texto recebidos sao armazenados, sem novas copias;
apos o sucesso, sua liberacao passa a ser responsabilidade do ITEM.

Parametros:
- id_sinistro: Valor que representa o identificador do sinistro.
- tipo_registro: Ponteiro para a classificacao textual do registro.
- data_sinistro: Ponteiro para a data do sinistro em formato textual.
- ano_sinistro: Valor que representa o ano em que ocorreu o sinistro.
- mes_sinistro: Valor que representa o mes em que ocorreu o sinistro.
- dia_sinistro: Valor que representa o dia do mes em que ocorreu o sinistro.
- hora_sinistro: Ponteiro para o horario registrado para o sinistro.
- ano_mes_sinistro: Ponteiro para a referencia de ano e mes registrada no CSV.
- dia_da_semana: Ponteiro para o dia da semana do sinistro.
- turno: Ponteiro para o turno associado a ocorrencia.
- logradouro: Ponteiro para o nome do logradouro da ocorrencia.
- numero_logradouro: Valor que representa o numero do logradouro da ocorrencia.
- tipo_via: Ponteiro para a classificacao da via da ocorrencia.
- tipo_local: Ponteiro para a classificacao do local da ocorrencia.
- latitude: Valor que representa a coordenada geografica de latitude.
- longitude: Valor que representa a coordenada geografica de longitude.
- cod_ibge: Valor que representa o codigo IBGE do municipio.
- municipio: Ponteiro para o municipio onde ocorreu o sinistro.
- regiao_administrativa: Ponteiro para a regiao administrativa associada a ocorrencia.
- administracao: Ponteiro para a administracao associada a via ou local.
- conservacao: Ponteiro para a informacao de conservacao da via ou local.
- circunscricao: Ponteiro para a circunscricao associada a ocorrencia.
- tp_sinistro_primario: Ponteiro para o tipo principal de sinistro.
- qtd_pedestre: Valor que representa a quantidade de pedestres registrada.
- qtd_bicicleta: Valor que representa a quantidade de bicicletas registrada.
- qtd_motocicleta: Valor que representa a quantidade de motocicletas registrada.
- qtd_automovel: Valor que representa a quantidade de automoveis registrada.
- qtd_onibus: Valor que representa a quantidade de onibus registrada.
- qtd_caminhao: Valor que representa a quantidade de caminhoes registrada.
- qtd_veic_outros: Valor que representa a quantidade de veiculos de outras categorias.
- qtd_veic_nao_disponivel: Valor que representa a quantidade registrada na categoria de veiculos nao disponiveis.
- qtd_gravidade_fatal: Valor que representa a quantidade de vitimas com gravidade fatal.
- qtd_gravidade_grave: Valor que representa a quantidade de vitimas com gravidade grave.
- qtd_gravidade_leve: Valor que representa a quantidade de vitimas com gravidade leve.
- qtd_gravidade_ileso: Valor que representa a quantidade de envolvidos classificados como ilesos.
- qtd_gravidade_nao_disponivel: Valor que representa a quantidade com classificacao de gravidade nao disponivel.
- tp_sinistro_atrop_pedestre: Indicador booleano de atropelamento de pedestre.
- tp_sinistro_atrop_vitima_fora_veic: Indicador booleano de atropelamento de vitima fora do veiculo.
- tp_sinistro_colisao_frontal: Indicador booleano de colisao frontal.
- tp_sinistro_colisao_traseira: Indicador booleano de colisao traseira.
- tp_sinistro_colisao_lateral: Indicador booleano de colisao lateral.
- tp_sinistro_colisao_transversal: Indicador booleano de colisao transversal.
- tp_sinistro_colisao_outros: Indicador booleano de outras formas de colisao.
- tp_sinistro_choque: Indicador booleano de choque.
- tp_sinistro_atrop_animal: Indicador booleano de atropelamento de animal.
- tp_sinistro_capotamento: Indicador booleano de capotamento.
- tp_sinistro_engavetamento: Indicador booleano de engavetamento.
- tp_sinistro_tombamento: Indicador booleano de tombamento.
- tp_sinistro_outros: Indicador booleano de outros tipos de sinistro.
- tp_sinistro_nao_disponivel: Indicador booleano de tipo de sinistro nao disponivel.
- linha_original: Ponteiro para a copia da linha original do CSV, mantida para exportacao.

Retorno: ITEM criado ou NULL se alguma alocacao falhar.
Em caso de falha, os ponteiros recebidos permanecem com o chamador.
*/
ITEM *item_criar_detran(int id_sinistro, char *tipo_registro, char *data_sinistro, int ano_sinistro,
    int mes_sinistro, int dia_sinistro, char *hora_sinistro, char *ano_mes_sinistro, char *dia_da_semana,
    char *turno, char *logradouro, double numero_logradouro, char *tipo_via, char *tipo_local, double latitude,
    double longitude, int cod_ibge, char *municipio, char *regiao_administrativa, char *administracao, char *conservacao,
    char *circunscricao, char *tp_sinistro_primario, int qtd_pedestre, int qtd_bicicleta, int qtd_motocicleta, int qtd_automovel,
    int qtd_onibus, int qtd_caminhao, int qtd_veic_outros, int qtd_veic_nao_disponivel, int qtd_gravidade_fatal,int qtd_gravidade_grave,
    int qtd_gravidade_leve, int qtd_gravidade_ileso, int qtd_gravidade_nao_disponivel, bool tp_sinistro_atrop_pedestre,
    bool tp_sinistro_atrop_vitima_fora_veic, bool tp_sinistro_colisao_frontal, bool tp_sinistro_colisao_traseira,
    bool tp_sinistro_colisao_lateral, bool tp_sinistro_colisao_transversal, bool tp_sinistro_colisao_outros,
    bool tp_sinistro_choque, bool tp_sinistro_atrop_animal, bool tp_sinistro_capotamento, bool tp_sinistro_engavetamento,
    bool tp_sinistro_tombamento, bool tp_sinistro_outros, bool tp_sinistro_nao_disponivel,char *linha_original){

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) malloc(sizeof(ITEM_DETRAN));

    if (item_detran == NULL)
        return NULL;

    item_detran->id_sinistro = id_sinistro;
    /* Guarda os ponteiros ja alocados pelo leitor; nao duplica as strings. */
    item_detran->tipo_registro = tipo_registro;
    item_detran->data_sinistro = data_sinistro;
    item_detran->ano_sinistro = ano_sinistro;
    item_detran->mes_sinistro = mes_sinistro;
    item_detran->dia_sinistro = dia_sinistro;
    item_detran->hora_sinistro = hora_sinistro;
    item_detran->ano_mes_sinistro = ano_mes_sinistro;
    item_detran->dia_da_semana = dia_da_semana;
    item_detran->turno = turno;
    item_detran->logradouro = logradouro;
    item_detran->numero_logradouro = numero_logradouro;
    item_detran->tipo_via = tipo_via;
    item_detran->tipo_local = tipo_local;
    item_detran->latitude = latitude;
    item_detran->longitude = longitude;
    item_detran->cod_ibge = cod_ibge;
    item_detran->municipio = municipio;
    item_detran->regiao_administrativa = regiao_administrativa;
    item_detran->administracao = administracao;
    item_detran->conservacao = conservacao;
    item_detran->circunscricao = circunscricao;
    item_detran->tp_sinistro_primario = tp_sinistro_primario;
    item_detran->qtd_pedestre = qtd_pedestre;
    item_detran->qtd_bicicleta = qtd_bicicleta;
    item_detran->qtd_motocicleta = qtd_motocicleta;
    item_detran->qtd_automovel = qtd_automovel;
    item_detran->qtd_onibus = qtd_onibus;
    item_detran->qtd_caminhao = qtd_caminhao;
    item_detran->qtd_veic_outros = qtd_veic_outros;
    item_detran->qtd_veic_nao_disponivel = qtd_veic_nao_disponivel;
    item_detran->qtd_gravidade_fatal = qtd_gravidade_fatal;
    item_detran->qtd_gravidade_grave = qtd_gravidade_grave;
    item_detran->qtd_gravidade_leve = qtd_gravidade_leve;
    item_detran->qtd_gravidade_ileso = qtd_gravidade_ileso;
    item_detran->qtd_gravidade_nao_disponivel = qtd_gravidade_nao_disponivel;
    item_detran->tp_sinistro_atrop_pedestre = tp_sinistro_atrop_pedestre;
    item_detran->tp_sinistro_atrop_vitima_fora_veic = tp_sinistro_atrop_vitima_fora_veic;
    item_detran->tp_sinistro_colisao_frontal = tp_sinistro_colisao_frontal;
    item_detran->tp_sinistro_colisao_traseira = tp_sinistro_colisao_traseira;
    item_detran->tp_sinistro_colisao_lateral = tp_sinistro_colisao_lateral;
    item_detran->tp_sinistro_colisao_transversal = tp_sinistro_colisao_transversal;
    item_detran->tp_sinistro_colisao_outros = tp_sinistro_colisao_outros;
    item_detran->tp_sinistro_choque = tp_sinistro_choque;
    item_detran->tp_sinistro_atrop_animal = tp_sinistro_atrop_animal;
    item_detran->tp_sinistro_capotamento = tp_sinistro_capotamento;
    item_detran->tp_sinistro_engavetamento = tp_sinistro_engavetamento;
    item_detran->tp_sinistro_tombamento = tp_sinistro_tombamento;
    item_detran->tp_sinistro_outros = tp_sinistro_outros;
    item_detran->tp_sinistro_nao_disponivel = tp_sinistro_nao_disponivel;

    /* O ITEM generico permite armazenar NASA e DETRAN na mesma interface. */
    ITEM *item = (ITEM *) malloc(sizeof(ITEM));
    if (item == NULL){

        free(item_detran); /* Os textos ainda pertencem ao chamador. */
        return NULL;
    }

    item->tipo_dado = TIPO_DETRAN; /* Identifica qual conjunto de dados ele contem. */
    item->dado = item_detran;
    item->linha_original = linha_original; /* Preserva a linha para exportar sem remontar o CSV. */
    item->similaridade = SIMILARIDADE_NAO_CALCULADA; /* Consultas podem calcular esse valor depois. */

    return item;
}

/*
Objetivo: Liberar os campos textuais alocados e a estrutura
especifica do DETRAN. Nao libera o ITEM externo nem sua
linha_original, cuja liberacao pertence a item_apagar().

Parametros:
- dado: Endereco do ponteiro para ITEM_DETRAN. Apos liberar
  a memoria, a funcao define esse ponteiro como NULL.
*/
void item_apagar_detran(void **dado){

    if (dado == NULL || *dado == NULL)
        return;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) *dado;

    /* Os campos de texto foram alocados individualmente pelo leitor CSV. */
    free(item_detran->tipo_registro);
    free(item_detran->data_sinistro);
    free(item_detran->hora_sinistro);
    free(item_detran->ano_mes_sinistro);
    free(item_detran->dia_da_semana);
    free(item_detran->turno);
    free(item_detran->logradouro);
    free(item_detran->tipo_via);
    free(item_detran->tipo_local);
    free(item_detran->municipio);
    free(item_detran->regiao_administrativa);
    free(item_detran->administracao);
    free(item_detran->conservacao);
    free(item_detran->circunscricao);
    free(item_detran->tp_sinistro_primario);
    free(item_detran); /* A linha_original e o ITEM externo ficam para item_apagar(). */

    *dado = NULL;
}

/*
Objetivo: Consultar o identificador do sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_id_sinistro(ITEM *item){

    /* Evita acessar dados de um registro NASA como se fossem DETRAN. */
    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->id_sinistro;
}

/*
Objetivo: Consultar a classificacao textual do registro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_tipo_registro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tipo_registro;
}

/*
Objetivo: Consultar a data do sinistro em formato textual armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_data_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->data_sinistro;
}

/*
Objetivo: Consultar o ano em que ocorreu o sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo. 0 se o item for invalido ou nao for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_ano_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->ano_sinistro;
}

/*
Objetivo: Consultar o mes em que ocorreu o sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo. 0 se o item for invalido ou nao for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_mes_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->mes_sinistro;
}

/*
Objetivo: Consultar o dia do mes em que ocorreu o sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_dia_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->dia_sinistro;
}

/*
Objetivo: Consultar o horario registrado para o sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_hora_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->hora_sinistro;
}

/*
Objetivo: Consultar a referencia de ano e mes registrada no CSV armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_ano_mes_sinistro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->ano_mes_sinistro;
}

/*
Objetivo: Consultar o dia da semana do sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_dia_da_semana(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->dia_da_semana;
}

/*
Objetivo: Consultar o turno associado a ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_turno(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->turno;
}

/*
Objetivo: Consultar o nome do logradouro da ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_logradouro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->logradouro;
}


/*
Objetivo: Consultar o numero do logradouro da ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
double item_detran_get_numero_logradouro(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->numero_logradouro;
}

/*
Objetivo: Consultar a classificacao da via da ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_tipo_via(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tipo_via;
}

/*
Objetivo: Consultar a classificacao do local da ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_tipo_local(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tipo_local;
}
/*
Objetivo: Consultar a coordenada geografica de latitude armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
double item_detran_get_latitude(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->latitude;
}

/*
Objetivo: Consultar a coordenada geografica de longitude armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
double item_detran_get_longitude(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->longitude;
}

/*
Objetivo: Consultar o codigo IBGE do municipio armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_cod_ibge(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->cod_ibge;
}

/*
Objetivo: Consultar o municipio onde ocorreu o sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_municipio(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->municipio;
}

/*
Objetivo: Consultar a regiao administrativa associada a ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_regiao_administrativa(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->regiao_administrativa;
}

/*
Objetivo: Consultar a administracao associada a via ou local armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_administracao(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->administracao;
}

/*
Objetivo: Consultar a informacao de conservacao da via ou local armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_conservacao(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->conservacao;
}

/*
Objetivo: Consultar a circunscricao associada a ocorrencia armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_circunscricao(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->circunscricao;
}

/*
Objetivo: Consultar o tipo principal de sinistro armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Ponteiro para o texto do campo ou NULL se o item for
invalido ou nao for do tipo DETRAN. O texto nao deve ser liberado
nem alterado pela funcao que o consulta.
*/
const char *item_detran_get_tp_sinistro_primario(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return NULL;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_primario;
}

/*
Objetivo: Consultar a quantidade de pedestres registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_pedestre(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_pedestre;
}


/*
Objetivo: Consultar a quantidade de bicicletas registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_bicicleta(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_bicicleta;
}

/*
Objetivo: Consultar a quantidade de motocicletas registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_motocicleta(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_motocicleta;
}

/*
Objetivo: Consultar a quantidade de automoveis registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_automovel(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_automovel;
}

/*
Objetivo: Consultar a quantidade de onibus registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_onibus(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_onibus;
}

/*
Objetivo: Consultar a quantidade de caminhoes registrada armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_caminhao(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_caminhao;
}

/*
Objetivo: Consultar a quantidade de veiculos de outras categorias armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_veic_outros(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_veic_outros;
}

/*
Objetivo: Consultar a quantidade registrada na categoria de veiculos nao disponiveis armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_veic_nao_disponivel(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_veic_nao_disponivel;
}

/*
Objetivo: Consultar a quantidade de vitimas com gravidade fatal armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_gravidade_fatal(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_gravidade_fatal;
}

/*
Objetivo: Consultar a quantidade de vitimas com gravidade grave armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_gravidade_grave(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_gravidade_grave;
}

/*
Objetivo: Consultar a quantidade de vitimas com gravidade leve armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_gravidade_leve(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_gravidade_leve;
}

/*
Objetivo: Consultar a quantidade de envolvidos classificados como ilesos armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_gravidade_ileso(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_gravidade_ileso;
}

/*
Objetivo: Consultar a quantidade com classificacao de gravidade nao disponivel armazenado no registro de sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Valor do campo; 0 se o item for invalido ou nao
for do tipo DETRAN (o valor 0 tambem pode ser legitimo).
*/
int item_detran_get_qtd_gravidade_nao_disponivel(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return 0;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->qtd_gravidade_nao_disponivel;
}

/*
Objetivo: Consultar o indicador de atropelamento de pedestre armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_atrop_pedestre(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_atrop_pedestre;
}

/*
Objetivo: Consultar o indicador de atropelamento de vitima fora do veiculo armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_atrop_vitima_fora_veic(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_atrop_vitima_fora_veic;
}

/*
Objetivo: Consultar o indicador de colisao frontal armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_colisao_frontal(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_colisao_frontal;
}

/*
Objetivo: Consultar o indicador de colisao traseira armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_colisao_traseira(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_colisao_traseira;
}

/*
Objetivo: Consultar o indicador de colisao lateral armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_colisao_lateral(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_colisao_lateral;
}

/*
Objetivo: Consultar o indicador de colisao transversal armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_colisao_transversal(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_colisao_transversal;
}

/*
Objetivo: Consultar o indicador de outras formas de colisao armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_colisao_outros(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_colisao_outros;
}

/*
Objetivo: Consultar o indicador de choque armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_choque(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_choque;
}

/*
Objetivo: Consultar o indicador de atropelamento de animal armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_atrop_animal(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_atrop_animal;
}

/*
Objetivo: Consultar o indicador de capotamento armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_capotamento(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_capotamento;
}

/*
Objetivo: Consultar o indicador de engavetamento armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_engavetamento(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_engavetamento;
}

/*
Objetivo: Consultar o indicador de tombamento armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_tombamento(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_tombamento;
}

/*
Objetivo: Consultar o indicador de outros tipos de sinistro armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_outros(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_outros;
}

/*
Objetivo: Consultar o indicador de tipo de sinistro nao disponivel armazenado no sinistro.

Parametros:
- item: Registro generico a ser consultado; deve conter dados DETRAN.

Retorno: Indicador armazenado; false se o item for invalido
ou nao for do tipo DETRAN.
*/
bool item_detran_get_tp_sinistro_nao_disponivel(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_DETRAN || item->dado == NULL)
        return false;

    ITEM_DETRAN *item_detran;
    item_detran = (ITEM_DETRAN *) item->dado;

    return item_detran->tp_sinistro_nao_disponivel;
}

/*
Objetivo: Obter o valor de um campo numerico do DETRAN pelo nome,
para permitir comparacoes e consultas genericas. A funcao verifica
o tipo do item, a presenca do valor e se o campo e reconhecido.

Parametros:
- item: Registro DETRAN que contem o valor a consultar.
- campo: Nome da coluna numerica a ser procurada.
- valor: Ponteiro que recebe o numero convertido para double
  somente quando a consulta ao campo tem sucesso.

Retorno: true se encontrou o campo numerico e preencheu *valor;
false em caso contrario, sem modificar *valor.
*/
bool obter_valor_numerico_detran(ITEM *item, const char *campo, double *valor){

    if (item == NULL || campo == NULL || valor == NULL ||
        item_get_tipo(item) != TIPO_DETRAN)
        return false;

    if (validar_valor(item, campo) != VALOR_PRESENTE)
        return false;

    if (strcmp(campo, "id_sinistro") == 0)
        *valor = item_detran_get_id_sinistro(item);

    else if (strcmp(campo, "ano_sinistro") == 0)
        *valor = item_detran_get_ano_sinistro(item);

    else if (strcmp(campo, "mes_sinistro") == 0)
        *valor = item_detran_get_mes_sinistro(item);

    else if (strcmp(campo, "dia_sinistro") == 0)
        *valor = item_detran_get_dia_sinistro(item);

    else if (strcmp(campo, "numero_logradouro") == 0)
        *valor = item_detran_get_numero_logradouro(item);

    else if (strcmp(campo, "latitude") == 0)
        *valor = item_detran_get_latitude(item);

    else if (strcmp(campo, "longitude") == 0)
        *valor = item_detran_get_longitude(item);

    else if (strcmp(campo, "cod_ibge") == 0)
        *valor = item_detran_get_cod_ibge(item);

    else if (strcmp(campo, "qtd_pedestre") == 0)
        *valor = item_detran_get_qtd_pedestre(item);

    else if (strcmp(campo, "qtd_bicicleta") == 0)
        *valor = item_detran_get_qtd_bicicleta(item);

    else if (strcmp(campo, "qtd_motocicleta") == 0)
        *valor = item_detran_get_qtd_motocicleta(item);

    else if (strcmp(campo, "qtd_automovel") == 0)
        *valor = item_detran_get_qtd_automovel(item);

    else if (strcmp(campo, "qtd_onibus") == 0)
        *valor = item_detran_get_qtd_onibus(item);

    else if (strcmp(campo, "qtd_caminhao") == 0)
        *valor = item_detran_get_qtd_caminhao(item);

    else if (strcmp(campo, "qtd_veic_outros") == 0)
        *valor = item_detran_get_qtd_veic_outros(item);

    else if (strcmp(campo, "qtd_veic_nao_disponivel") == 0)
        *valor = item_detran_get_qtd_veic_nao_disponivel(item);

    else if (strcmp(campo, "qtd_gravidade_fatal") == 0)
        *valor = item_detran_get_qtd_gravidade_fatal(item);

    else if (strcmp(campo, "qtd_gravidade_grave") == 0)
        *valor = item_detran_get_qtd_gravidade_grave(item);

    else if (strcmp(campo, "qtd_gravidade_leve") == 0)
        *valor = item_detran_get_qtd_gravidade_leve(item);

    else if (strcmp(campo, "qtd_gravidade_ileso") == 0)
        *valor = item_detran_get_qtd_gravidade_ileso(item);

    else if (strcmp(campo, "qtd_gravidade_nao_disponivel") == 0)
        *valor = item_detran_get_qtd_gravidade_nao_disponivel(item);

    else
        return false;

    return true;
}