#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "query.h"

#define TAM_LINHA 1024

/* Armazena os tres componentes de uma condicao FILTER. */
typedef struct filtro_ {

    char *campo;
    char *operador;
    char *valor;

} FILTRO;

/* Cada SIM combina um campo numerico, um alvo, uma tolerancia e um peso. */
typedef struct similaridade_ {

    char *campo;
    double alvo;
    double tolerancia;
    double peso;

} SIMILARIDADE;

/* Configuracao da ordenacao solicitada pela diretiva ORDER. */
typedef struct ordem_ {

    char *criterio_ordenacao;
    char *direcao;

} ORDEM;

/* Cada entrada GROUP identifica um campo para agrupamento. */
typedef struct grupo_ {

    char *campo;

} GRUPO;

/* Campos de organizacao e quantidade de dimensoes da estrutura cruzada. */
typedef struct cross_ {

    char **campos;
    int dimensao;

} CROSS;

/* Limiar minimo de similaridade, usado por ACCEPT. */
typedef struct accept_ {

    double valor;

} ACCEPT;

/* A QUERY centraliza todas as diretivas interpretadas do arquivo. */
struct query_ {

    bool possui_filtros;
    FILTRO *filtros;
    int quantidade_filtros;

    bool possui_similaridade;
    SIMILARIDADE *similaridades;
    int quantidade_similaridades;

    bool possui_accept;
    ACCEPT accept;

    bool possui_ordenacao;
    ORDEM ordenacao;

    bool possui_grupo;
    GRUPO *grupos;
    int quantidade_grupos;

    bool possui_cross;
    CROSS cross;

};

/*
Objetivo: Alocar e inicializar uma QUERY vazia, sem filtros, similaridades, ordenacao,
  agrupamentos ou dimensoes CROSS.

Parametros:
- Nenhum.

Retorno: Ponteiro para a QUERY inicializada, ou NULL se a alocacao falhar.
*/
QUERY *query_criar(){

    QUERY *query = (QUERY *) malloc (sizeof(QUERY));
    if (query == NULL)
        return NULL;

    /* Inicialmente, nenhuma diretiva esta presente e os vetores sao vazios. */
    query->possui_filtros = false;
    query->filtros = NULL;
    query->quantidade_filtros = 0;

    query->possui_similaridade = false;
    query->similaridades = NULL;
    query->quantidade_similaridades = 0;

    query->possui_accept = false;
    query->accept.valor = 0;

    query->possui_ordenacao = false;
    query->ordenacao.criterio_ordenacao = NULL;
    query->ordenacao.direcao = NULL;

    query->possui_grupo = false;
    query->grupos = NULL;
    query->quantidade_grupos = 0;

    query->possui_cross = false;
    query->cross.campos = NULL;
    query->cross.dimensao = 0;

    return query;

}

/*
Objetivo: Criar uma copia independente do texto, para que a QUERY possa guardar valores sem
  depender da linha lida.

Parametros:
- origem: String de origem, terminada em '\0', cujo conteudo sera copiado.

Retorno: Ponteiro para a copia alocada ou NULL em caso de falha.
*/
char *copiar_string_query(const char *origem){
    
    char *destino;
    int tamanho;

    tamanho = strlen(origem);

    /* Reserva o caractere terminador adicional. */
    destino = (char *) malloc((tamanho + 1) * sizeof(char));

    if (destino == NULL)
        return NULL;

    strcpy(destino, origem);

    return destino;
}

/*
Objetivo: Extrair um campo de uma linha de consulta ate o proximo separador '|' ou fim da
  string, atualizando o ponto de leitura.

Parametros:
- char_ptr: Endereco do ponteiro de leitura; e avancado ate o proximo campo ou o fim.
- campo: Buffer de saida que recebe o texto extraido, terminado em '\0'; deve comportar
  TAM_LINHA caracteres.

Retorno: true ao extrair um campo nao vazio dentro do tamanho permitido; false em caso
  contrario.
*/
bool ler_campo(const char **char_ptr, char *campo){

    int i = 0;

    if (char_ptr == NULL || *char_ptr == NULL || campo == NULL || **char_ptr == '\0' || **char_ptr == '|')
        return false;

    /* Copia um campo e avanca o ponteiro original recebido por endereco. */
    while (**char_ptr != '|' && **char_ptr != '\0'){
        if (i >= TAM_LINHA - 1)
            return false;
        campo[i++] = **char_ptr;
        (*char_ptr)++;
    }

    campo[i] = '\0';

    /* Pula o separador para a proxima leitura. */
    if (**char_ptr == '|')
        (*char_ptr)++;

    return true;
}

/*
Objetivo: Verificar se uma sequencia de parametros separados por '|' possui exatamente a
  quantidade esperada, sem campos vazios.

Parametros:
- string: Texto que contem os parametros separados por '|'.
- quantidade_esperada: Numero exato de campos exigido pelo comando.

Retorno: true se o formato e a quantidade de campos estiverem corretos; false caso
  contrario.
*/
bool quantidade_campos_query(const char *string, int quantidade_esperada){

    if (string == NULL || *string == '\0' || *string == '|' || quantidade_esperada <= 0)
        return false;

    /* Uma string nao vazia possui ao menos um campo. */
    int campos = 1;
    for (const char *p = string; *p != '\0'; p++){
        if (*p == '|'){
            if (*(p + 1) == '\0' || *(p + 1) == '|') //verifica se um campo e vazio//
                return false;
            campos++;
        }
    }

    if (campos == quantidade_esperada)
        return true;

    return false;
    
}

/*
Objetivo: Converter um parametro textual de consulta para double com strtod, rejeitando
  texto vazio e caracteres extras.

Parametros:
- string: Texto completo que deve representar um numero.
- valor: Ponteiro de saida que recebe o numero somente se a conversao for valida.

Retorno: true se a conversao for valida; false caso contrario, sem escrever em *valor.
*/
bool converter_double_query(const char *string, double *valor){

    if (string == NULL || valor == NULL || *string == '\0')
        return false;

    char *fim;

    /* O ponteiro fim permite rejeitar caracteres que sobraram apos o numero. */
    double numero = strtod(string, &fim);

    if (fim == string || *fim != '\0')
        return false;
    
    *valor = numero;

    return true;
}

/*
Objetivo: Verificar se o operador informado esta entre os operadores relacionais aceitos
  pelos filtros.

Parametros:
- operador: Texto do operador (=, ==, !=, <, <=, > ou >=).

Retorno: true para operador aceito; false para NULL ou texto desconhecido.
*/
bool operador_valido_query(const char *operador){

    if (operador == NULL)
        return false;

    return strcmp(operador, "=") == 0 || strcmp(operador, "==") == 0 ||
           strcmp(operador, "!=") == 0 || strcmp(operador, "<") == 0 ||
           strcmp(operador, "<=") == 0 || strcmp(operador, ">") == 0 ||
           strcmp(operador, ">=") == 0;
}

/*
Objetivo: Interpretar uma diretiva FILTER com campo, operador e valor, criando um novo
  filtro na QUERY.

Parametros:
- query: Consulta que recebera mais um filtro.
- string: Parametros no formato campo|operador|valor.

Retorno: true apos inserir o filtro; false para formato invalido ou falha de memoria.
*/
bool ler_query_filtro(QUERY *query, const char *string){

    if (query == NULL || !quantidade_campos_query(string, 3))
        return false;

    const char *char_ptr = string;

    char campo[TAM_LINHA];
    char operador[TAM_LINHA];
    char valor[TAM_LINHA];

    if (ler_campo(&char_ptr, campo) == false)
        return false;

    if (ler_campo(&char_ptr, operador) == false)
        return false;

    if (ler_campo(&char_ptr, valor) == false)
        return false;

    if (!operador_valido_query(operador))
        return false;

    /* Os buffers locais serao reutilizados; a QUERY guarda copias proprias. */
    char *novo_campo = copiar_string_query(campo);
    if (novo_campo == NULL)
        return false;

    char *novo_operador = copiar_string_query(operador);
    if (novo_operador == NULL){
        free(novo_campo);
        return false;
    }

    char *novo_valor = copiar_string_query(valor);
    if (novo_valor == NULL){
        free(novo_campo);
        free(novo_operador);
        return false;
    }

    /* Aumenta dinamicamente o vetor em uma posicao; em erro, libera as copias. */
    FILTRO *novo = realloc(query->filtros, (query->quantidade_filtros + 1) * sizeof(FILTRO));

    if (novo == NULL){
        free(novo_campo);
        free(novo_operador);
        free(novo_valor);
        return false;
    }

    int posicao_nova = query->quantidade_filtros;

    novo[posicao_nova].campo = novo_campo;
    novo[posicao_nova].operador = novo_operador;
    novo[posicao_nova].valor = novo_valor;

    query->filtros = novo;
    query->possui_filtros = true;
    query->quantidade_filtros++;

    return true;
}

/*
Objetivo: Interpretar uma diretiva SIM e armazenar campo, alvo, tolerancia e peso para o
  calculo de similaridade.

Parametros:
- query: Consulta que recebera o criterio de similaridade.
- string: Parametros no formato campo|alvo|tolerancia|peso.

Retorno: true ao inserir um criterio valido; false para erro de formato, de faixa numerica
  ou de memoria.
*/
bool ler_query_similaridade(QUERY *query, const char *string){

    if (query == NULL || !quantidade_campos_query(string, 4))
        return false;

    const char *char_ptr = string;

    char campo[TAM_LINHA];
    char alvo[TAM_LINHA];
    char tolerancia[TAM_LINHA];
    char peso[TAM_LINHA];

    if (ler_campo(&char_ptr, campo) == false)
        return false;

    if (ler_campo(&char_ptr, alvo) == false)
        return false;

    if (ler_campo(&char_ptr, tolerancia) == false)
        return false;

    if (ler_campo(&char_ptr, peso) == false)
        return false;

    double novo_alvo, nova_tolerancia, novo_peso;
    if (!converter_double_query(alvo, &novo_alvo) ||
        !converter_double_query(tolerancia, &nova_tolerancia) ||
        !converter_double_query(peso, &novo_peso) ||
        nova_tolerancia <= 0 || novo_peso < 0)
        return false;

    char *novo_campo = copiar_string_query(campo);
    if (novo_campo == NULL)
        return false;


    /* Acrescenta um criterio SIM ao vetor, preservando os anteriores. */
    SIMILARIDADE *novo = realloc(query->similaridades,(query->quantidade_similaridades + 1) * sizeof(SIMILARIDADE));

    if (novo == NULL){
        free(novo_campo);
        return false;
    }

    int posicao_nova = query->quantidade_similaridades;

    novo[posicao_nova].campo = novo_campo;
    novo[posicao_nova].alvo = novo_alvo;
    novo[posicao_nova].tolerancia = nova_tolerancia;
    novo[posicao_nova].peso = novo_peso;

    query->similaridades = novo;
    query->possui_similaridade = true;
    query->quantidade_similaridades++;

    return true;
}

/*
Objetivo: Interpretar uma diretiva ACCEPT que define o limiar minimo de similaridade aceito
  (entre 0 e 1).

Parametros:
- query: Consulta que recebera o limiar, desde que ainda nao possua ACCEPT.
- string: Texto contendo o unico valor numerico exigido pela diretiva.

Retorno: true quando o limiar e armazenado; false para repeticao ou valor invalido.
*/
bool ler_query_accept(QUERY *query, const char *string){

    if (query == NULL || !quantidade_campos_query(string, 1) || query->possui_accept == true)
        return false;

    const char *char_ptr = string;

    char valor[TAM_LINHA];

    if (ler_campo(&char_ptr, valor) == false)
        return false;

    double numero;
    if (!converter_double_query(valor, &numero) || numero < 0 || numero > 1)
        return false;

    /* O limiar ACCEPT representa uma nota de similaridade entre 0 e 1. */
    query->accept.valor = numero;
    query->possui_accept = true;

    return true;
}

/*
Objetivo: Interpretar uma diretiva ORDER e guardar o criterio e a direcao da ordenacao na
  QUERY.

Parametros:
- query: Consulta que recebera a configuracao ORDER, se ainda nao existir.
- string: Parametros no formato criterio|direcao.

Retorno: true se os campos foram armazenados; false em caso de repeticao, formato ou
  alocacao invalidos. A direcao e validada depois por ler_query.
*/
bool ler_query_order(QUERY *query, const char *string){

    if (query == NULL || !quantidade_campos_query(string, 2) || query->possui_ordenacao == true)
        return false;

    const char *char_ptr = string;

    char criterio_ordenacao[TAM_LINHA];
    char direcao[TAM_LINHA];

    if (ler_campo(&char_ptr, criterio_ordenacao) == false)
        return false;

    if (ler_campo(&char_ptr, direcao) == false)
        return false;

    /* Guarda o campo e a direcao em memoria propria da consulta. */
    query->ordenacao.criterio_ordenacao = copiar_string_query(criterio_ordenacao);
    if (query->ordenacao.criterio_ordenacao == NULL)
        return false;
    
    query->ordenacao.direcao = copiar_string_query(direcao);
    if (query->ordenacao.direcao == NULL){

        free(query->ordenacao.criterio_ordenacao);
        query->ordenacao.criterio_ordenacao = NULL;
        return false;

    }
    
    query->possui_ordenacao = true;

    return true;

}

/*
Objetivo: Interpretar uma diretiva GROUP com um ou mais campos e registra-los dinamicamente
  na QUERY.

Parametros:
- query: Consulta que recebera a lista de campos de agrupamento.
- string: Lista nao vazia de nomes de campos separados por '|'.

Retorno: true ao concluir a leitura; false para formatacao incorreta, repeticao ou falha de
  memoria.
*/
bool ler_query_grupo(QUERY *query, const char *string){

    if (query == NULL || string == NULL || *string == '\0' || query->possui_grupo == true)
        return false;

    if (string[strlen(string) - 1] == '|')
        return false;

    const char *char_ptr = string;

    char campo[TAM_LINHA];

    while (*char_ptr != '\0'){

        if (ler_campo(&char_ptr, campo) == false)
            return false;

        char *novo_campo = copiar_string_query(campo);

        if (novo_campo == NULL)
            return false;

        /* Cada campo GROUP acrescenta uma entrada no vetor de grupos. */
        GRUPO *novo = realloc(query->grupos, (query->quantidade_grupos + 1) * sizeof(GRUPO));

        if (novo == NULL){
            free(novo_campo);
            return false;
        }

        int posicao_nova = query->quantidade_grupos;

        novo[posicao_nova].campo = novo_campo;

        query->grupos = novo;
        query->quantidade_grupos++;
    }

    query->possui_grupo = true;
    return true;
}

/*
Objetivo: Interpretar uma diretiva CROSS e montar um vetor de campos com no minimo duas
  dimensoes.

Parametros:
- query: Consulta que recebera a configuracao multidimensional.
- string: Nomes dos campos separados por '|', com ao menos dois campos.

Retorno: true se o CROSS e armazenado; false para formato invalido, repeticao, menos de duas
  dimensoes ou falha de memoria.
*/
bool ler_query_cross(QUERY *query, const char *string){

    if (query == NULL || string == NULL || *string == '\0' ||
        query->possui_cross == true)
        return false;

    if (string[strlen(string) - 1] == '|') //ve se o campo e vazio//
        return false;

    const char *char_ptr = string;
    char campo[TAM_LINHA];

    /* Constroi a configuracao CROSS temporariamente, antes de atribui-la a QUERY. */
    char **campos = NULL;
    int dimensao = 0;

    while (*char_ptr != '\0'){

        if (ler_campo(&char_ptr, campo) == false){
            for (int i = 0; i < dimensao; i++)
                free(campos[i]);

            free(campos);
            return false;
        }

        char *novo_campo = copiar_string_query(campo);

        if (novo_campo == NULL){
            for (int i = 0; i < dimensao; i++)
                free(campos[i]);

            free(campos);
            return false;
        }

        /* O vetor de ponteiros cresce conforme cada dimensao e lida. */
        char **novo = realloc(campos, (dimensao + 1) * sizeof(char *));

        if (novo == NULL){
            free(novo_campo);
            for (int i = 0; i < dimensao; i++)
                free(campos[i]);

            free(campos);
            return false;
        }

        campos = novo;
        campos[dimensao] = novo_campo;
        dimensao++;
    }

    /* CROSS exige ao menos duas dimensoes para ser aceito. */
    if (dimensao < 2){
        for (int i = 0; i < dimensao; i++)
                free(campos[i]);

            free(campos);
        return false;
    }

    query->cross.campos = campos;
    query->cross.dimensao = dimensao;
    query->possui_cross = true;

    return true;
}

/*
Objetivo: Liberar todos os textos, vetores dinamicos e a estrutura da consulta, evitando
  vazamentos de memoria.

Parametros:
- query: Endereco do ponteiro para QUERY; ao final, o ponteiro apontado e definido como
  NULL.
*/
void query_apagar(QUERY **query){

    if (query == NULL || *query == NULL)
        return;

    /* Libera primeiro cada string interna e depois os vetores que as contêm. */
    for (int i = 0; i < (*query)->quantidade_filtros; i++){
        free((*query)->filtros[i].campo);
        free((*query)->filtros[i].operador);
        free((*query)->filtros[i].valor);
    }
    free((*query)->filtros);

    for (int i = 0; i < (*query)->quantidade_similaridades; i++)
        free((*query)->similaridades[i].campo);
    free((*query)->similaridades);

    free((*query)->ordenacao.criterio_ordenacao);
    free((*query)->ordenacao.direcao);

    for (int i = 0; i < (*query)->quantidade_grupos; i++)
        free((*query)->grupos[i].campo);
    free((*query)->grupos);

    for (int i = 0; i < (*query)->cross.dimensao; i++)
        free((*query)->cross.campos[i]);

    free((*query)->cross.campos);

    free(*query);
    *query = NULL; /* Evita deixar o chamador apontando para memoria liberada. */
}

/*
Objetivo: Ler um arquivo de consultas linha por linha, identificar as diretivas e montar a QUERY; verificar regras globais apos a leitura.

Parametros:
- arquivo: Arquivo de texto ja aberto contendo diretivas FILTER, SIM, ACCEPT, ORDER, GROUP e
  CROSS.
- query: Endereco do ponteiro para uma QUERY ja criada; sera liberada e definida como NULL
  caso a leitura seja invalida.

Retorno: true se todas as diretivas e validacoes forem aceitas; false se ocorrer erro,
  liberando a QUERY.
*/
bool ler_query(FILE *arquivo, QUERY **query){

    if (query == NULL || *query == NULL)
        return false;

    if (arquivo == NULL){
        query_apagar(query);
        return false;
    }

    bool valida = true;
    char linha[TAM_LINHA];

    /* Cada linha nao vazia representa uma diretiva da consulta. */
    while (fgets(linha, TAM_LINHA, arquivo) != NULL){

        /* Rejeita linhas maiores que o buffer para nao interpretar instrucoes truncadas. */
        size_t tamanho = strlen(linha);
        if (tamanho > 0 && linha[tamanho - 1] != '\n' && !feof(arquivo)){
            if (tamanho == TAM_LINHA - 1){
                valida = false;
                break;
            }
        }

        linha[strcspn(linha, "\r\n")] = '\0';

        if (linha[0] == '\0')
            continue;

        /* Os comandos aceitos possuem no maximo seis caracteres. */
        char comando[7];
        char *char_ptr = linha;
        int i = 0;

        while (*char_ptr != '|' && *char_ptr != '\0'){

            if (i >= 6){
                valida = false;
                break;
            }

            comando[i] = *char_ptr;
            char_ptr++;
            i++;
        }

        comando[i] = '\0';

        if (valida == false || *char_ptr == '\0'){
            valida = false;
            break;
        }

        /* Despacha os parametros para o leitor proprio de cada diretiva. */
        if (strcmp(comando, "FILTER") == 0)
            valida = ler_query_filtro(*query, char_ptr + 1);

        else if (strcmp(comando, "SIM") == 0)
            valida = ler_query_similaridade(*query, char_ptr + 1);

        else if (strcmp(comando, "ACCEPT") == 0)
            valida = ler_query_accept(*query, char_ptr + 1);

        else if (strcmp(comando, "ORDER") == 0)
            valida = ler_query_order(*query, char_ptr + 1);

        else if (strcmp(comando, "GROUP") == 0)
            valida = ler_query_grupo(*query, char_ptr + 1);

        else if (strcmp(comando, "CROSS") == 0)
            valida = ler_query_cross(*query, char_ptr + 1);

        else
            valida = false;

        if (valida == false)
            break;
    }

    if (ferror(arquivo) != 0)
        valida = false;

    /* ACCEPT so faz sentido se houver SIM e um limiar no intervalo permitido. */
    if ((*query)->possui_accept == true){
        if ((*query)->possui_similaridade == false ||
            !((*query)->accept.valor >= 0 && (*query)->accept.valor <= 1))
            valida = false;
    }

    /* Deve existir pelo menos um peso estritamente positivo para a media ponderada. */
    if ((*query)->possui_similaridade){
        bool peso_positivo = false;
        for (int i = 0; i < (*query)->quantidade_similaridades; i++){
            if ((*query)->similaridades[i].peso > 0)
                peso_positivo = true;
        }
        if (!peso_positivo)
            valida = false;
    }

    /* Verifica a direcao e impede ORDER por similarity sem criterios SIM. */
    if ((*query)->possui_ordenacao == true){

        if (strcmp((*query)->ordenacao.direcao, "ASC") != 0 &&
            strcmp((*query)->ordenacao.direcao, "DESC") != 0)
            valida = false;

        if (strcmp((*query)->ordenacao.criterio_ordenacao, "similarity") == 0 &&
            (*query)->possui_similaridade == false)
            valida = false;
    }

    if (valida == false)
        query_apagar(query); /* Descarta consulta incompleta ou invalida. */

    return valida;
}

/*
Objetivo: Informar se a consulta possui ao menos um filtro.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_filtros.
*/
bool query_get_possui_filtros(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_filtros;

}

/*
Objetivo: Obter quantos filtros foram armazenados na consulta.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Quantidade de filtros ou 0 quando a QUERY for NULL.
*/
int query_get_quantidade_filtros(QUERY *query){

    if (query == NULL)
        return 0;

    return query->quantidade_filtros;
}

/*
Objetivo: Obter o nome do campo usado por um filtro da consulta.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do filtro, entre 0 e quantidade_filtros - 1.

Retorno: Ponteiro para o texto do campo ou NULL para indice/QUERY invalidos. O texto
  pertence a QUERY e nao deve ser liberado pelo chamador.
*/
const char *query_get_filtro_campo(QUERY *query, int indice){

    /* A verificacao do indice evita acessar posicoes fora do vetor. */
    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_filtros)
        return NULL;

    return query->filtros[indice].campo;
}

/*
Objetivo: Obter o operador relacional de um filtro da consulta.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do filtro, entre 0 e quantidade_filtros - 1.

Retorno: Ponteiro para o texto do operador ou NULL para indice/QUERY invalidos. O texto
  pertence a QUERY e nao deve ser liberado pelo chamador.
*/
const char *query_get_filtro_operador(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_filtros)
        return NULL;

    return query->filtros[indice].operador;
}

/*
Objetivo: Obter o valor textual de referencia de um filtro.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do filtro, entre 0 e quantidade_filtros - 1.

Retorno: Ponteiro para o valor textual ou NULL para indice/QUERY invalidos. O texto pertence
  a QUERY e nao deve ser liberado pelo chamador.
*/
const char *query_get_filtro_valor(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_filtros)
        return NULL;

    return query->filtros[indice].valor;
}

/*
Objetivo: Informar se a consulta possui criterios SIM.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_similaridade.
*/
bool query_get_possui_similaridade(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_similaridade;
}

/*
Objetivo: Obter a quantidade de criterios de similaridade.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Quantidade armazenada ou 0 quando a QUERY for NULL.
*/
int query_get_quantidade_similaridades(QUERY *query){

    if (query == NULL)
        return 0;

    return query->quantidade_similaridades;
}

/*
Objetivo: Obter o campo utilizado pelo criterio de similaridade indicado.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do criterio SIM, entre 0 e quantidade_similaridades - 1.

Retorno: Ponteiro para o nome do campo ou NULL para indice/QUERY invalidos. O texto pertence
  a QUERY e nao deve ser liberado pelo chamador.
*/
const char *query_get_similaridade_campo(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_similaridades)
        return NULL;

    return query->similaridades[indice].campo;
}

/*
Objetivo: Obter o valor-alvo de um criterio de similaridade.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do criterio SIM, entre 0 e quantidade_similaridades - 1.

Retorno: Alvo do criterio ou 0 para indice/QUERY invalidos.
*/
double query_get_similaridade_alvo(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_similaridades)
        return 0;

    return query->similaridades[indice].alvo;
}

/*
Objetivo: Obter a tolerancia de um criterio de similaridade.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do criterio SIM, entre 0 e quantidade_similaridades - 1.

Retorno: Tolerancia do criterio ou 0 para indice/QUERY invalidos.
*/
double query_get_similaridade_tolerancia(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_similaridades)
        return 0;

    return query->similaridades[indice].tolerancia;
}

/*
Objetivo: Obter o peso de um criterio de similaridade.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do criterio SIM, entre 0 e quantidade_similaridades - 1.

Retorno: Peso do criterio ou 0 para indice/QUERY invalidos.
*/
double query_get_similaridade_peso(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_similaridades)
        return 0;

    return query->similaridades[indice].peso;
}

/*
Objetivo: Informar se existe um limiar ACCEPT definido.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_accept.
*/
bool query_get_possui_accept(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_accept;
}

/*
Objetivo: Obter o limiar de similaridade registrado em ACCEPT.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Limiar armazenado ou 0 quando a QUERY for NULL.
*/
double query_get_accept(QUERY *query){

    if (query == NULL)
        return 0;

    return query->accept.valor;
}

/*
Objetivo: Informar se a consulta definiu uma diretiva ORDER.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_ordenacao.
*/
bool query_get_possui_ordenacao(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_ordenacao;
}

/*
Objetivo: Obter o nome do campo ou criterio definido para ordenacao.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Texto do criterio ou NULL quando a QUERY for invalida. O texto pertence a QUERY e
  nao deve ser liberado pelo chamador.
*/
const char *query_get_ordenacao_criterio(QUERY *query){

    if (query == NULL)
        return NULL;

    return query->ordenacao.criterio_ordenacao;
}

/*
Objetivo: Obter a direcao configurada para ordenacao.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Texto da direcao ou NULL quando a QUERY for invalida. O texto pertence a QUERY e
  nao deve ser liberado pelo chamador.
*/
const char *query_get_ordenacao_direcao(QUERY *query){

    if (query == NULL)
        return NULL;

    return query->ordenacao.direcao;
}

/*
Objetivo: Informar se a consulta possui uma diretiva GROUP.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_grupo.
*/
bool query_get_possui_grupo(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_grupo;
}

/*
Objetivo: Obter o numero de campos de agrupamento cadastrados.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Quantidade de campos ou 0 quando a QUERY for NULL.
*/
int query_get_quantidade_grupos(QUERY *query){

    if (query == NULL)
        return 0;

    return query->quantidade_grupos;
}

/*
Objetivo: Obter o nome de um campo usado no agrupamento.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao do campo GROUP, entre 0 e quantidade_grupos - 1.

Retorno: Texto do campo ou NULL para indice/QUERY invalidos. O texto pertence a QUERY e nao
  deve ser liberado pelo chamador.
*/
const char *query_get_grupo_campo(QUERY *query, int indice){

    if (query == NULL ||
        indice < 0 ||
        indice >= query->quantidade_grupos)
        return NULL;

    return query->grupos[indice].campo;
}

/*
Objetivo: Informar se a consulta possui uma diretiva CROSS.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: false quando a QUERY for NULL; caso contrario, valor da flag possui_cross.
*/
bool query_get_possui_cross(QUERY *query){

    if (query == NULL)
        return false;

    return query->possui_cross;
}

/*
Objetivo: Obter o numero de dimensoes da configuracao CROSS.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.

Retorno: Numero de dimensoes ou 0 quando nao existir CROSS ou a QUERY for invalida.
*/
int query_get_dimensao_cross(QUERY *query){

    if (query == NULL || query->possui_cross == false)
        return 0;

    return query->cross.dimensao;
}

/*
Objetivo: Obter o campo correspondente a uma dimensao do CROSS.

Parametros:
- query: Estrutura de consulta da qual os dados serao recuperados.
- indice: Posicao da dimensao CROSS, entre 0 e dimensao - 1.

Retorno: Texto do campo ou NULL para indice/QUERY invalidos ou quando nao existir CROSS. O
  texto pertence a QUERY e nao deve ser liberado pelo chamador.
*/
const char *query_get_cross_campo(QUERY *query, int indice){

    /* Um indice CROSS so existe quando a consulta possui esta configuracao. */
    if (query == NULL || query->possui_cross == false ||
        indice < 0 || indice >= query->cross.dimensao)
        return NULL;

    return query->cross.campos[indice];
}