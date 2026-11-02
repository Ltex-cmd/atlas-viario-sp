#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "reader_nasa.h"
#include "item.h"
#include "item_nasa.h"

#define NUM_CAMPOS 20
#define TAM_LINHA 4096


/*
Objetivo: Alocar uma copia de um texto lido do CSV para que ele continue valido depois que o buffer usado na leitura for reutilizado.

Parametros:
- origem: String a ser copiada, incluindo seu caractere terminador '\0'.

Retorno: Ponteiro para a copia alocada ou NULL em caso de falha de alocacao.
*/
char *copiar_string_reader_nasa(const char *origem){

    char *destino;
    int tamanho;

    tamanho = strlen(origem);

    destino = (char *) malloc((tamanho + 1) * sizeof(char));

    if (destino == NULL)
        return NULL;

    strcpy(destino, origem);

    return destino;
}

/*
Objetivo: Separar uma linha do CSV em campos delimitados por virgula, sem
considerar virgulas que estejam entre aspas. A separacao e feita no proprio
buffer, substituindo delimitadores por '\0'.

Parametros:
- linha: Buffer com o registro CSV; e modificado durante a separacao.
- campos: Vetor de ponteiros que recebe o endereco inicial de cada campo,
  dentro do proprio buffer linha (capacidade de NUM_CAMPOS posicoes).

Retorno: Numero de campos encontrados ou -1 quando ha colunas em excesso
ou aspas sem fechamento.
*/
int separar_linha(char *linha, char **campos){

    int quantidade = 0;
    bool dentro_de_aspas = false;
    char *p;

    p = linha;

    campos[0] = p;
    quantidade++;

    while (*p != '\0'){

        if (*p == '"')
            dentro_de_aspas = !dentro_de_aspas;
        
        else if (*p == ',' && dentro_de_aspas == false){

            *p = '\0';

            if (quantidade >= NUM_CAMPOS)
                return -1;

            campos[quantidade] = p + 1; /* Proximo campo comeca apos a virgula. */
            quantidade++;
        }

        p++;
    }

    if (dentro_de_aspas)
        return -1;

    return quantidade;
}

/*
Objetivo: Interpretar os indicadores booleanos do CSV da NASA.
Reconhece "True", "true" e "TRUE" como verdadeiros; os demais valores
sao tratados como falsos.

Parametros:
- campo: Texto do indicador booleano que sera interpretado.

Retorno: true para as tres grafias reconhecidas ou false caso contrario.
*/
bool ler_bool(const char *campo){

    if (strcmp (campo, "True") == 0 || strcmp(campo, "true") == 0 || strcmp(campo, "TRUE") == 0)
        return true;
    else
        return false;

}

/*
Objetivo: Converter o texto de um campo numerico do CSV para inteiro
usando atoi; valores invalidos nao sao distinguidos do valor zero.

Parametros:
- campo: String que representa o numero inteiro a converter.

Retorno: Numero inteiro convertido (atoi retorna zero para texto sem numero).
*/
int ler_int(const char *campo){

    return atoi(campo);

}

/*
Objetivo: Converter um campo numerico do CSV para double por meio de atof.
Campos vazios sao convertidos em zero; textos invalidos tambem podem
resultar em zero, pois atof nao sinaliza erros de conversao.

Parametros:
- campo: String com o numero real a ser convertido (espera ponto decimal).

Retorno: Valor convertido em double, ou zero se o campo estiver vazio.
*/
double ler_double(const char *campo){

    if (campo[0] == '\0')
        return 0;

    return atof(campo);
}

/*
Objetivo: Ler o proximo registro nao vazio do CSV da NASA, dividir suas
20 colunas, converter os campos para seus respectivos tipos e criar um
ITEM com os dados estruturados. Guarda tambem a linha original para
permitir que o registro seja exportado sem reconstruir o CSV.

Parametros:
- arquivo: Ponteiro para o CSV ja aberto e posicionado na proxima linha
  a ser lida; sua posicao avanca a cada leitura.

Retorno: Ponteiro para o ITEM criado ou NULL no fim do arquivo e em caso
de falha de leitura, quantidade inesperada de colunas ou alocacao.
*/
ITEM *reader_ler_item_nasa(FILE *arquivo){

    char linha[TAM_LINHA]; /* Buffer temporario: reutilizado a cada registro. */
    char *campos[NUM_CAMPOS]; /* Aponta para trechos dentro de linha. */

    char *asteroid_designation;
    char *asteroid_fullname;
    char *close_approach_date;
    char *threat_category;
    char *panic_verdict;

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
    bool on_sentry_list;
    double sentry_impact_prob;
    double sentry_torino_scale;
    double sentry_palermo_scale;
    double sentry_diameter_km;

    ITEM *item;

    if (arquivo == NULL)
        return NULL;

    /* Pula linhas vazias ou compostas apenas por espacos em branco. */
    while (true){

        if (fgets(linha, TAM_LINHA, arquivo) == NULL)
            return NULL;

        int i;

        for (i = 0; linha[i] != '\0'; i++){
            if (linha[i] != ' ' && linha[i] != '\t' &&
                linha[i] != '\r' && linha[i] != '\n')
                break;
        }

        if (linha[i] != '\0')
            break;
    }

    /* Preserva a linha intacta, inclusive para exportacao posterior. */
    char *linha_original = copiar_string_reader_nasa(linha);
    if (linha_original == NULL)
        return NULL;

    linha[strcspn(linha, "\r\n")] = '\0';

    /* Cada registro valido deve conter exatamente 20 campos. */
    if (separar_linha(linha, campos) != NUM_CAMPOS){
        free(linha_original);
        return NULL;
    }

    asteroid_designation = copiar_string_reader_nasa(campos[0]);
    asteroid_fullname = copiar_string_reader_nasa(campos[1]);
    close_approach_date = copiar_string_reader_nasa(campos[2]);
    threat_category = copiar_string_reader_nasa(campos[13]);
    panic_verdict = copiar_string_reader_nasa(campos[14]);

    /* Se alguma copia de texto falhou, libera todas as que foram alocadas. */
    if (asteroid_designation == NULL ||
        asteroid_fullname == NULL ||
        close_approach_date == NULL ||
        threat_category == NULL ||
        panic_verdict == NULL){

        free(asteroid_designation);
        free(asteroid_fullname);
        free(close_approach_date);
        free(threat_category);
        free(panic_verdict);
        free(linha_original);

        return NULL;
    }

    /* Ano, mes, intervalo ate a aproximacao e nivel de panico sao inteiros. */
    year = ler_int(campos[3]);
    month = ler_int(campos[4]);

    /* Distancia, velocidade e magnitude sao informacoes numericas reais. */
    distance_au = ler_double(campos[5]);
    velocity_km_s = ler_double(campos[6]);
    absolute_magnitude = ler_double(campos[7]);

    days_until_approach = ler_int(campos[8]);

    /* Identifica se a aproximacao e um evento passado ou futuro. */
    is_past_event = ler_bool(campos[9]);
    is_future_event = ler_bool(campos[10]);

    risk_score = ler_double(campos[11]);
    panic_level = ler_int(campos[12]);

    /* Campos Sentry descrevem avaliacoes relacionadas a possivel impacto. */
    on_sentry_list = ler_bool(campos[15]);

    sentry_impact_prob = ler_double(campos[16]);
    sentry_torino_scale = ler_double(campos[17]);
    sentry_palermo_scale = ler_double(campos[18]);
    sentry_diameter_km = ler_double(campos[19]);

    /* O ITEM passa a guardar os textos alocados e a linha original. */
    item = item_criar_nasa(
        asteroid_designation,
        asteroid_fullname,
        close_approach_date,
        year,
        month,
        distance_au,
        velocity_km_s,
        absolute_magnitude,
        days_until_approach,
        is_past_event,
        is_future_event,
        risk_score,
        panic_level,
        threat_category,
        panic_verdict,
        on_sentry_list,
        sentry_impact_prob,
        sentry_torino_scale,
        sentry_palermo_scale,
        sentry_diameter_km,
        linha_original
    );

    /* Na falha da criacao, o leitor continua responsavel por liberar tudo. */
    if (item == NULL){
        free(asteroid_designation);
        free(asteroid_fullname);
        free(close_approach_date);
        free(threat_category);
        free(panic_verdict);
        free(linha_original);

        return NULL;
    }

    return item; /* Um registro pronto para ser inserido na estrutura. */
    
}