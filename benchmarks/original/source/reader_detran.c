#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "reader_detran.h"
#include "item.h"
#include "item_detran.h"
 
#define NUM_CAMPOS 50 /* O CSV do DETRAN utilizado no projeto possui 50 colunas. */
#define TAM_LINHA 4096 /* Tamanho do buffer usado para ler uma linha por vez. */


/*
Objetivo: Criar uma copia independente de uma string lida do CSV. Essa copia
permite que o texto continue valido depois que o buffer da linha for reutilizado.

Parametros:
- origem: String que sera copiada, incluindo o '\0'.
*/
char *copiar_string_reader_detran(const char *origem){

    char *destino;
    int tamanho;

    tamanho = strlen(origem);
    destino = (char *) malloc((tamanho + 1) * sizeof(char)); /* +1 por causa do '\0'. */

    if (destino == NULL)
        return NULL;

    strcpy(destino, origem); /* Os campos nao dependem mais do buffer temporario. */
    return destino;
}

/*
Objetivo: Dividir uma linha do CSV em campos separados por ponto e virgula,
ignorando separadores encontrados entre aspas. A divisao ocorre na propria
string recebida, sem alocar memoria para cada campo.

Parametros:
- linha: Buffer com a linha do CSV; os delimitadores encontrados fora de
  aspas sao substituidos por '\0'.
- campos: Vetor que recebe ponteiros para o inicio de cada campo dentro de
  linha, com capacidade para NUM_CAMPOS posicoes.
*/
int separar_linha_detran(char *linha, char **campos){

    int quantidade = 0;
    bool dentro_de_aspas = false;
    char *p = linha;

    campos[0] = p; /* O primeiro campo comeca no inicio da linha. */
    quantidade++;

    while (*p != '\0'){

        if (*p == '"')
            dentro_de_aspas = !dentro_de_aspas; /* Alterna ao entrar/sair das aspas. */

        else if (*p == ';' && dentro_de_aspas == false){

            *p = '\0'; /* Encerra o campo atual sem criar outra string. */

            if (quantidade >= NUM_CAMPOS)  /* Mais colunas que o esperado. */
                return -1; 

            campos[quantidade] = p + 1;  /* Inicio do proximo campo. */
            quantidade++;
        }

        p++;
    }

    if (dentro_de_aspas) /* Aspas abertas ao fim da linha é um formato invalido. */
        return -1;

    return quantidade;
}

/*
Objetivo: Retirar as aspas externas de um campo CSV e transformar cada par
  de aspas duplicadas em uma unica aspa. O texto original do registro ja foi
  preservado separadamente para a exportacao.

Parametro: campo dentro do buffer temporario da linha.
*/
static void decodificar_campo_detran(char *campo){

    size_t tamanho = strlen(campo);
    if (tamanho < 2 || campo[0] != '"' || campo[tamanho - 1] != '"')
        return;

    char *leitura = campo + 1;
    char *escrita = campo;
    const char *fim = campo + tamanho - 1;

    while (leitura < fim){
        if (*leitura == '"' && leitura + 1 < fim && leitura[1] == '"')
            leitura++; /* No CSV, duas aspas representam uma aspa no texto. */
        *escrita++ = *leitura++;
    }
    *escrita = '\0';
}

/*
Objetivo: Interpretar os indicadores do CSV do DETRAN como valores booleanos.
Apenas o texto "S" e considerado verdadeiro; os demais valores resultam em falso.

Parametros:
- campo: Texto do indicador a ser interpretado.
*/
bool ler_bool_detran(const char *campo){

    return strcmp(campo, "S") == 0;
}

/*
Objetivo: Converter um campo numerico do CSV para inteiro usando atoi.
A conversao nao diferencia valores invalidos de zero.

Parametros:
- campo: Texto que representa o numero inteiro a ser convertido.
*/
int ler_int_detran(const char *campo){

    return atoi(campo);
}

/*
Objetivo: Converter um campo numerico para double, adaptando a virgula decimal
usada em parte dos dados para ponto antes da conversao com atof. Campos vazios
sao tratados como zero.

Parametros:
- campo: String numerica a ser convertida; pode ser modificada durante a
  substituicao de virgulas por pontos.
*/
double ler_double_detran(char *campo){

    if (campo[0] == '\0') /* Campo ausente no CSV. */
        return 0;

    for (int i = 0; campo[i] != '\0'; i++){
        if (campo[i] == ',')
            campo[i] = '.';  /* atof espera o ponto como separador decimal. */
    }

    return atof(campo);
}

/*
Objetivo: Ler o proximo registro nao vazio do CSV de sinistros do DETRAN,
separar suas 50 colunas, converter cada campo ao tipo correspondente e criar
um ITEM com os dados estruturados e a linha original para futura exportacao.
Retorna NULL se o arquivo terminar ou se ocorrer erro de leitura/conversao
estrutural/alocacao.

Parametros:
- arquivo: Arquivo CSV ja aberto, posicionado no proximo registro a ler;
  o ponteiro avanca conforme as linhas sao consumidas.
*/
ITEM *reader_ler_item_detran(FILE *arquivo){

    char linha[TAM_LINHA]; /* Buffer temporario para uma linha do CSV. */
    char *campos[NUM_CAMPOS]; /* Ponteiros para campos dentro do proprio buffer. */

    /* Variaveis correspondentes as colunas do CSV, agrupadas por tipo. */
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

    ITEM *item;

    if (arquivo == NULL)
        return NULL;
    /* Ignora linhas vazias ou compostas somente por espacos. */
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

    /* Guarda a linha intacta para exportar o mesmo registro depois. */
    char *linha_original = copiar_string_reader_detran(linha);
    if (linha_original == NULL)
        return NULL;

    linha[strcspn(linha, "\r\n")] = '\0';

    /* Cada registro precisa fornecer exatamente as 50 colunas esperadas. */
    if (separar_linha_detran(linha, campos) != NUM_CAMPOS){
        free(linha_original);
        return NULL;
    }

    /* Remove a codificacao CSV antes de comparar ou converter cada valor. */
    for (int i = 0; i < NUM_CAMPOS; i++)
        decodificar_campo_detran(campos[i]);

    /* Copia os campos de texto: campos[] aponta para o buffer local "linha". */
    tipo_registro = copiar_string_reader_detran(campos[1]);
    data_sinistro = copiar_string_reader_detran(campos[2]);
    hora_sinistro = copiar_string_reader_detran(campos[6]);
    ano_mes_sinistro = copiar_string_reader_detran(campos[7]);
    dia_da_semana = copiar_string_reader_detran(campos[8]);
    turno = copiar_string_reader_detran(campos[9]);
    logradouro = copiar_string_reader_detran(campos[10]);
    tipo_via = copiar_string_reader_detran(campos[12]);
    tipo_local = copiar_string_reader_detran(campos[13]);
    municipio = copiar_string_reader_detran(campos[17]);
    regiao_administrativa = copiar_string_reader_detran(campos[18]);
    administracao = copiar_string_reader_detran(campos[19]);
    conservacao = copiar_string_reader_detran(campos[20]);
    circunscricao = copiar_string_reader_detran(campos[21]);
    tp_sinistro_primario = copiar_string_reader_detran(campos[22]);

    /* Se qualquer copia falhou, desfaz todas as alocacoes ja realizadas. */
    if (tipo_registro == NULL ||
        data_sinistro == NULL ||
        hora_sinistro == NULL ||
        ano_mes_sinistro == NULL ||
        dia_da_semana == NULL ||
        turno == NULL ||
        logradouro == NULL ||
        tipo_via == NULL ||
        tipo_local == NULL ||
        municipio == NULL ||
        regiao_administrativa == NULL ||
        administracao == NULL ||
        conservacao == NULL ||
        circunscricao == NULL ||
        tp_sinistro_primario == NULL){

        free(tipo_registro);
        free(data_sinistro);
        free(hora_sinistro);
        free(ano_mes_sinistro);
        free(dia_da_semana);
        free(turno);
        free(logradouro);
        free(tipo_via);
        free(tipo_local);
        free(municipio);
        free(regiao_administrativa);
        free(administracao);
        free(conservacao);
        free(circunscricao);
        free(tp_sinistro_primario);
        free(linha_original);

        return NULL;
    }

    /* Converte os identificadores, datas numericas e contagens para int. */
    id_sinistro = ler_int_detran(campos[0]);
    ano_sinistro = ler_int_detran(campos[3]);
    mes_sinistro = ler_int_detran(campos[4]);
    dia_sinistro = ler_int_detran(campos[5]);
    /* Campos numericos que podem conter virgula como separador decimal. */
    numero_logradouro = ler_double_detran(campos[11]);
    latitude = ler_double_detran(campos[14]);
    longitude = ler_double_detran(campos[15]);
    cod_ibge = ler_int_detran(campos[16]);
    qtd_pedestre = ler_int_detran(campos[23]);
    qtd_bicicleta = ler_int_detran(campos[24]);
    qtd_motocicleta = ler_int_detran(campos[25]);
    qtd_automovel = ler_int_detran(campos[26]);
    qtd_onibus = ler_int_detran(campos[27]);
    qtd_caminhao = ler_int_detran(campos[28]);
    qtd_veic_outros = ler_int_detran(campos[29]);
    qtd_veic_nao_disponivel = ler_int_detran(campos[30]);
    qtd_gravidade_fatal = ler_int_detran(campos[31]);
    qtd_gravidade_grave = ler_int_detran(campos[32]);
    qtd_gravidade_leve = ler_int_detran(campos[33]);
    qtd_gravidade_ileso = ler_int_detran(campos[34]);
    qtd_gravidade_nao_disponivel = ler_int_detran(campos[35]);
    /* Indicadores de tipos de sinistro: "S" vira true; outros valores, false. */
    tp_sinistro_atrop_pedestre = ler_bool_detran(campos[36]);
    tp_sinistro_atrop_vitima_fora_veic = ler_bool_detran(campos[37]);
    tp_sinistro_colisao_frontal = ler_bool_detran(campos[38]);
    tp_sinistro_colisao_traseira = ler_bool_detran(campos[39]);
    tp_sinistro_colisao_lateral = ler_bool_detran(campos[40]);
    tp_sinistro_colisao_transversal = ler_bool_detran(campos[41]);
    tp_sinistro_colisao_outros = ler_bool_detran(campos[42]);
    tp_sinistro_choque = ler_bool_detran(campos[43]);
    tp_sinistro_atrop_animal = ler_bool_detran(campos[44]);
    tp_sinistro_capotamento = ler_bool_detran(campos[45]);
    tp_sinistro_engavetamento = ler_bool_detran(campos[46]);
    tp_sinistro_tombamento = ler_bool_detran(campos[47]);
    tp_sinistro_outros = ler_bool_detran(campos[48]);
    tp_sinistro_nao_disponivel = ler_bool_detran(campos[49]);

    /* Monta o ITEM com os valores convertidos. Se der certo, o ITEM passa a ser responsavel pelas strings alocadas e pela linha original. */
    item = item_criar_detran(
        id_sinistro,
        tipo_registro,
        data_sinistro,
        ano_sinistro,
        mes_sinistro,
        dia_sinistro,
        hora_sinistro,
        ano_mes_sinistro,
        dia_da_semana,
        turno,
        logradouro,
        numero_logradouro,
        tipo_via,
        tipo_local,
        latitude,
        longitude,
        cod_ibge,
        municipio,
        regiao_administrativa,
        administracao,
        conservacao,
        circunscricao,
        tp_sinistro_primario,
        qtd_pedestre,
        qtd_bicicleta,
        qtd_motocicleta,
        qtd_automovel,
        qtd_onibus,
        qtd_caminhao,
        qtd_veic_outros,
        qtd_veic_nao_disponivel,
        qtd_gravidade_fatal,
        qtd_gravidade_grave,
        qtd_gravidade_leve,
        qtd_gravidade_ileso,
        qtd_gravidade_nao_disponivel,
        tp_sinistro_atrop_pedestre,
        tp_sinistro_atrop_vitima_fora_veic,
        tp_sinistro_colisao_frontal,
        tp_sinistro_colisao_traseira,
        tp_sinistro_colisao_lateral,
        tp_sinistro_colisao_transversal,
        tp_sinistro_colisao_outros,
        tp_sinistro_choque,
        tp_sinistro_atrop_animal,
        tp_sinistro_capotamento,
        tp_sinistro_engavetamento,
        tp_sinistro_tombamento,
        tp_sinistro_outros,
        tp_sinistro_nao_disponivel,
        linha_original
    );

    //Se houver falha
    if (item == NULL){
        free(tipo_registro);
        free(data_sinistro);
        free(hora_sinistro);
        free(ano_mes_sinistro);
        free(dia_da_semana);
        free(turno);
        free(logradouro);
        free(tipo_via);
        free(tipo_local);
        free(municipio);
        free(regiao_administrativa);
        free(administracao);
        free(conservacao);
        free(circunscricao);
        free(tp_sinistro_primario);
        free(linha_original);

        return NULL;
    }

    return item; /* Um registro pronto para ser inserido na base. */
}
