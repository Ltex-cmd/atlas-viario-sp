#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "item_nasa.h"

/* Envoltorio comum: guarda o tipo, os dados especificos e a linha CSV. */
struct item_ {

    int tipo_dado;
    void *dado;
    char *linha_original;
    double similaridade;

};

/* Armazena os 20 atributos NASA ja convertidos para seus tipos. */
typedef struct item_nasa_ {
	
	char *asteroid_designation;
    char *asteroid_fullname;

    char *close_approach_date;
    int year;
    int month;

    double distance_au;
    double velocity_km_s;
    double absolute_magnitude;

    int days_until_approach;
    bool is_past_event;
    bool is_future_event;

    double risk_score;
    int panic_level;

    char *threat_category;
    char *panic_verdict;

    bool on_sentry_list;
    double sentry_impact_prob;
    double sentry_torino_scale;
    double sentry_palermo_scale;
    double sentry_diameter_km;

} ITEM_NASA;

/*
Objetivo: Construir um registro de asteroide a partir dos 20 campos
ja convertidos do CSV. Aloca o ITEM_NASA e seu envoltorio ITEM,
armazena a linha original e inicializa a similaridade. As strings
recebidas sao armazenadas diretamente, sem novas copias. Em caso
de sucesso, o ITEM assume a responsabilidade por libera-las.

Parametros:
- asteroid_designation: Ponteiro para identificador/designacao do asteroide.
- asteroid_fullname: Ponteiro para nome completo do asteroide.
- close_approach_date: Ponteiro para data prevista ou registrada da aproximacao.
- year: Valor que representa ano da aproximacao.
- month: Valor que representa mes da aproximacao.
- distance_au: Valor que representa distancia da aproximacao em unidades astronomicas (UA).
- velocity_km_s: Valor que representa velocidade relativa em quilometros por segundo.
- absolute_magnitude: Valor que representa magnitude absoluta do asteroide.
- days_until_approach: Valor que representa numero de dias ate a aproximacao, conforme o CSV.
- is_past_event: Valor que representa indicador de aproximacao ja ocorrida.
- is_future_event: Valor que representa indicador de aproximacao futura.
- risk_score: Valor que representa pontuacao de risco informada nos dados.
- panic_level: Valor que representa nivel de alerta ou panico informado nos dados.
- threat_category: Ponteiro para categoria textual de ameaca.
- panic_verdict: Ponteiro para descricao textual do alerta de panico.
- on_sentry_list: Valor que representa indicador de presenca na lista Sentry.
- sentry_impact_prob: Valor que representa probabilidade de impacto da lista Sentry.
- sentry_torino_scale: Valor que representa valor da escala Torino da lista Sentry.
- sentry_palermo_scale: Valor que representa valor da escala Palermo da lista Sentry.
- sentry_diameter_km: Valor que representa diametro estimado em quilometros da lista Sentry.
- linha_original: Ponteiro para linha CSV original mantida para a exportacao.

Retorno: Ponteiro para o ITEM criado ou NULL se a alocacao
falhar. Em caso de falha, as strings continuam com o chamador.
*/
ITEM *item_criar_nasa(char *asteroid_designation, char *asteroid_fullname, char *close_approach_date, int year, 
				int month, double distance_au, double velocity_km_s, double absolute_magnitude, int days_until_approach, 
				bool is_past_event, bool is_future_event, double risk_score, int panic_level, char *threat_category, 
				char *panic_verdict, bool on_sentry_list, double sentry_impact_prob, double sentry_torino_scale, 
				double sentry_palermo_scale, double sentry_diameter_km, char *linha_original){
	
	ITEM_NASA *item_nasa;
	/* Primeiro aloca a parte especifica do registro NASA. */
	item_nasa = (ITEM_NASA *) malloc (sizeof(ITEM_NASA));

	if (item_nasa == NULL)
        return NULL;

	/* Reutiliza as strings ja alocadas no leitor, sem copia-las novamente. */
	item_nasa->asteroid_designation = asteroid_designation;
	item_nasa->asteroid_fullname = asteroid_fullname;
	item_nasa->close_approach_date = close_approach_date;
	item_nasa->year = year;
	item_nasa->month = month;
	item_nasa->distance_au = distance_au;
	item_nasa->velocity_km_s = velocity_km_s;
	item_nasa->absolute_magnitude = absolute_magnitude;
	item_nasa->days_until_approach = days_until_approach;
	item_nasa->is_past_event = is_past_event;
	item_nasa->is_future_event = is_future_event;
	item_nasa->risk_score = risk_score;
	item_nasa->panic_level = panic_level;
	item_nasa->threat_category = threat_category;
	item_nasa->panic_verdict = panic_verdict;
	item_nasa->on_sentry_list = on_sentry_list;
	item_nasa->sentry_impact_prob = sentry_impact_prob;
	item_nasa->sentry_torino_scale = sentry_torino_scale;
	item_nasa->sentry_palermo_scale = sentry_palermo_scale;
	item_nasa->sentry_diameter_km = sentry_diameter_km;
	
	/* Cria o envoltorio comum, usado pelas estruturas de dados do projeto. */
	ITEM *item = (ITEM *) malloc (sizeof(ITEM));
    if (item == NULL){

        free(item_nasa); /* As strings continuam sob responsabilidade do leitor. */
        return NULL;
    }

    item->tipo_dado = TIPO_NASA; /* Permite identificar o tipo concreto. */
    item->dado = item_nasa;
    item->linha_original = linha_original; /* Reaproveitada na exportacao CSV. */
    item->similaridade = SIMILARIDADE_NAO_CALCULADA; /* Ainda nao foi calculada. */

    return item;
}

/*
Objetivo: Liberar as strings armazenadas e a estrutura ITEM_NASA.
Nao libera o ITEM externo nem a linha_original, que sao
liberados pela funcao item_apagar().

Parametros:
- dado: Endereco do ponteiro para os dados NASA.
  Ao final, o ponteiro apontado e definido como NULL.
*/
void item_apagar_nasa(void **dado){

    if (dado == NULL || *dado == NULL)
        return;

    ITEM_NASA *item_nasa;

    item_nasa = (ITEM_NASA *) *dado;

    /* Os campos textuais foram alocados separadamente pelo leitor NASA. */
    free(item_nasa->asteroid_designation);
    free(item_nasa->asteroid_fullname);
    free(item_nasa->close_approach_date);
    free(item_nasa->threat_category);
    free(item_nasa->panic_verdict);
    free(item_nasa);

    *dado = NULL; /* Evita manter uma referencia pendente apos a liberacao. */
}

/*
Objetivo: Consultar identificador/designacao do asteroide no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Texto associado ao campo ou NULL para um item invalido
ou de outro tipo. O texto pertence ao ITEM e nao deve ser liberado
pela funcao que o consulta.
*/
const char *item_nasa_get_asteroid_designation(ITEM *item){

    /* Nao interpreta memoria de outro tipo de registro como ITEM_NASA. */
    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return NULL;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->asteroid_designation;
}

/*
Objetivo: Consultar nome completo do asteroide no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Texto associado ao campo ou NULL para um item invalido
ou de outro tipo. O texto pertence ao ITEM e nao deve ser liberado
pela funcao que o consulta.
*/
const char *item_nasa_get_asteroid_fullname(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return NULL;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->asteroid_fullname;
}



/*
Objetivo: Consultar data prevista ou registrada da aproximacao no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Texto associado ao campo ou NULL para um item invalido
ou de outro tipo. O texto pertence ao ITEM e nao deve ser liberado
pela funcao que o consulta.
*/
const char *item_nasa_get_close_approach_date(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return NULL;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->close_approach_date;
}

/*
Objetivo: Consultar ano da aproximacao no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
int item_nasa_get_year(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->year;
}

/*
Objetivo: Consultar mes da aproximacao no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
int item_nasa_get_month(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->month;
}

/*
Objetivo: Consultar distancia da aproximacao em unidades astronomicas (UA) no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
double item_nasa_get_distance_au(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->distance_au;
}

/*
Objetivo: Consultar velocidade relativa em quilometros por segundo no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
double item_nasa_get_velocity_km_s(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->velocity_km_s;
}

/*
Objetivo: Consultar magnitude absoluta do asteroide no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
double item_nasa_get_absolute_magnitude(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->absolute_magnitude;
}

/*
Objetivo: Consultar numero de dias ate a aproximacao, conforme o CSV no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
int item_nasa_get_days_until_approach(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->days_until_approach;
}

/*
Objetivo: Consultar indicador de aproximacao ja ocorrida no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Indicador armazenado ou false para um item invalido
ou de outro tipo (false tambem pode ser um valor legitimo).
*/
bool item_nasa_get_is_past_event(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return false;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->is_past_event;
}

/*
Objetivo: Consultar indicador de aproximacao futura no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Indicador armazenado ou false para um item invalido
ou de outro tipo (false tambem pode ser um valor legitimo).
*/
bool item_nasa_get_is_future_event(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return false;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->is_future_event;
}

/*
Objetivo: Consultar pontuacao de risco informada nos dados no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
double item_nasa_get_risk_score(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->risk_score;
}

/*
Objetivo: Consultar nivel de alerta ou panico informado nos dados no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
*/
int item_nasa_get_panic_level(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->panic_level;
}

/*
Objetivo: Consultar categoria textual de ameaca no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Texto associado ao campo ou NULL para um item invalido
ou de outro tipo. O texto pertence ao ITEM e nao deve ser liberado
pela funcao que o consulta.
*/
const char *item_nasa_get_threat_category(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return NULL;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->threat_category;
}

/*
Objetivo: Consultar descricao textual do alerta de panico no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Texto associado ao campo ou NULL para um item invalido
ou de outro tipo. O texto pertence ao ITEM e nao deve ser liberado
pela funcao que o consulta.
*/
const char *item_nasa_get_panic_verdict(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return NULL;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->panic_verdict;
}

/*
Objetivo: Consultar indicador de presenca na lista Sentry no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Indicador armazenado ou false para um item invalido
ou de outro tipo (false tambem pode ser um valor legitimo).
*/
bool item_nasa_get_on_sentry_list(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return false;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    return item_nasa->on_sentry_list;
}

/*
Objetivo: Consultar probabilidade de impacto da lista Sentry no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
Tambem retorna 0 quando o asteroide nao consta da lista Sentry.
*/
double item_nasa_get_sentry_impact_prob(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    /* Campos Sentry nao se aplicam a asteroides fora dessa lista. */
    if (item_nasa->on_sentry_list == false)
        return 0;
    
    return item_nasa->sentry_impact_prob;
}

/*
Objetivo: Consultar valor da escala Torino da lista Sentry no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
Tambem retorna 0 quando o asteroide nao consta da lista Sentry.
*/
double item_nasa_get_sentry_torino_scale(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    if (item_nasa->on_sentry_list == false)
        return 0;

    return item_nasa->sentry_torino_scale;
}

/*
Objetivo: Consultar valor da escala Palermo da lista Sentry no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
Tambem retorna 0 quando o asteroide nao consta da lista Sentry.
*/
double item_nasa_get_sentry_palermo_scale(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    if (item_nasa->on_sentry_list == false)
        return 0;

    return item_nasa->sentry_palermo_scale;
}

/*
Objetivo: Consultar diametro estimado em quilometros da lista Sentry no registro NASA.

Parametros:
- item: Registro generico consultado; deve conter dados NASA.

Retorno: Valor numerico do campo ou 0 para um item invalido
ou de outro tipo (zero tambem pode ser um valor legitimo).
Tambem retorna 0 quando o asteroide nao consta da lista Sentry.
*/
double item_nasa_get_sentry_diameter_km(ITEM *item){

    if (item == NULL || item->tipo_dado != TIPO_NASA || item->dado == NULL)
        return 0;

    ITEM_NASA *item_nasa;
    item_nasa = (ITEM_NASA *) item->dado;

    if (item_nasa->on_sentry_list == false)
        return 0;

    return item_nasa->sentry_diameter_km;
}

/*
Objetivo: Consultar um campo numerico de um registro NASA pelo
nome, permitindo que filtros e comparacoes usem uma interface
generica. Verifica se o campo existe e possui valor no CSV;
dados da lista Sentry so podem ser lidos quando aplicaveis.

Parametros:
- item: Registro NASA do qual sera obtido o campo.
- campo: Nome do atributo numerico que sera consultado.
- valor: Ponteiro de saida que recebe o numero convertido
  para double apenas quando a consulta tiver sucesso.

Retorno: true quando o campo numerico e reconhecido e presente;
false caso contrario, sem modificar o conteudo de *valor.
*/
bool obter_valor_numerico_nasa(ITEM *item, const char *campo, double *valor){

    if (item == NULL ||
        item_get_tipo(item) != TIPO_NASA || campo == NULL || valor == NULL)
        return false;

    /* Impede que valores vazios do CSV sejam confundidos com zero. */
    if (validar_valor(item, campo) != 0)
        return false;

    /* Mapeia o nome textual da coluna para o getter correspondente. */
    if (strcmp(campo, "year") == 0){
        *valor = (double) item_nasa_get_year(item); /* Normaliza int para double. */
        return true;
    }

    if (strcmp(campo, "month") == 0){
        *valor = (double) item_nasa_get_month(item);
        return true;
    }

    if (strcmp(campo, "distance_au") == 0){
        *valor = item_nasa_get_distance_au(item);
        return true;
    }
    if (strcmp(campo, "velocity_km_s") == 0){
        *valor = item_nasa_get_velocity_km_s(item);
        return true;
    }

    if (strcmp(campo, "absolute_magnitude") == 0){
        *valor = item_nasa_get_absolute_magnitude(item);
        return true;
    }

    if (strcmp(campo, "days_until_approach") == 0){
        *valor = (double) item_nasa_get_days_until_approach(item);
        return true;
    }

    if (strcmp(campo, "risk_score") == 0){
        *valor = item_nasa_get_risk_score(item);
        return true;
    }

    if (strcmp(campo, "panic_level") == 0){
        *valor = (double) item_nasa_get_panic_level(item);
        return true;
    }

    if (strcmp(campo, "sentry_impact_prob") == 0){
        /* Atributos Sentry so sao validos para asteroides dessa lista. */
        if (item_nasa_get_on_sentry_list(item) == false)
            return false;

        *valor = item_nasa_get_sentry_impact_prob(item);
        return true;
    }

    if (strcmp(campo, "sentry_torino_scale") == 0){
        if (item_nasa_get_on_sentry_list(item) == false)
            return false;

        *valor = item_nasa_get_sentry_torino_scale(item);
        return true;
    }

    if (strcmp(campo, "sentry_palermo_scale") == 0){
        if (item_nasa_get_on_sentry_list(item) == false)
            return false;

        *valor = item_nasa_get_sentry_palermo_scale(item);
        return true;
    }

    if (strcmp(campo, "sentry_diameter_km") == 0){
        if (item_nasa_get_on_sentry_list(item) == false)
            return false;

        *valor = item_nasa_get_sentry_diameter_km(item);
        return true;
    }

    return false;
}