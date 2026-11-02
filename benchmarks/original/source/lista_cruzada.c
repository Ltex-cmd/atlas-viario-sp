#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lista_cruzada.h"
#include "item.h"
#include "comparador.h"

#define CAPACIDADE_INICIAL 256

struct lista_cruzada_ {

    ITEM ***listas;
    int *atuais;

    int tipo_lista;
    int dimensao;

    union {
        char **campos;
        QUERY *query;
    } criterio;

    int tamanho;
    int capacidade_atual;
    int tamanho_max;

};

/*
Objetivo: Criar uma copia dinamica do texto usado como nome de campo,
permitindo que a lista de base mantenha seus criterios de ordenacao.

Parametros:
- texto: String a copiar, terminada em '\0'; pode ser NULL.

Retorno: Ponteiro para a copia alocada ou NULL se a entrada for nula
ou a alocacao falhar. O chamador fica responsavel por liberar a copia.
*/
char *copiar_string_lista_cruzada(const char *texto){

    if (texto == NULL)
        return NULL;

    char *copia = malloc(strlen(texto) + 1);
    if (copia != NULL)
        strcpy(copia, texto);

    return copia;
}

/*
Objetivo: Obter o nome do campo que define a ordenacao de uma dimensao
na lista cruzada. Em listas de base, consulta o vetor proprio; em listas
de saida, consulta a diretiva CROSS da QUERY associada.

Parametros:
- lista: Lista cruzada que contem a configuracao das dimensoes.
- indice: Posicao da dimensao, de zero ate dimensao - 1.

Retorno: Nome do campo correspondente ou NULL se os parametros, o indice
ou o tipo da lista forem invalidos. Nao transfere posse da string.
*/
const char *obter_campo(LISTA_CRUZADA *lista, int indice){

    if (lista == NULL || indice < 0 || indice >= lista->dimensao)
        return NULL;

    if (lista->tipo_lista == LISTA_BASE)
        return lista->criterio.campos[indice];

    if (lista->tipo_lista == LISTA_SAIDA)
        return query_get_cross_campo(lista->criterio.query, indice);

    return NULL;
}


/*
Objetivo: Comparar dois registros usando o campo definido em uma dimensao
CROSS da consulta. Valida a existencia da diretiva e do indice antes de
encaminhar a comparacao para a funcao generica comparar_itens.

Parametros:
- item1: Primeiro registro a comparar.
- item2: Segundo registro a comparar.
- query: Consulta que possui os campos da diretiva CROSS.
- indice: Indice da dimensao cujo campo sera usado na comparacao.

Retorno: -1, 0 ou 1 segundo a ordem dos registros; retorna
COMPARACAO_FALHOU (-2) para parametros, campo ou comparacao invalidos.
*/
int comparar_itens_cross(ITEM *item1, ITEM *item2, QUERY *query, int indice){

    if (item1 == NULL || item2 == NULL || query == NULL ||
        !query_get_possui_cross(query) ||
        indice < 0 || indice >= query_get_dimensao_cross(query))
        return COMPARACAO_FALHOU;

    const char *campo = query_get_cross_campo(query, indice);

    if (campo == NULL)
        return COMPARACAO_FALHOU;

    return comparar_itens(item1, item2, campo);
}


/*
Objetivo: Reservar o descritor e os vetores de uma lista cruzada com uma
sequencia ordenada independente para cada dimensao. Inicializa os cursores,
os limites e a capacidade inicial, respeitando o tamanho maximo informado.
Libera as alocacoes anteriores se alguma etapa falhar.

Parametros:
- tipo_lista: LISTA_BASE ou LISTA_SAIDA, indicando o uso da estrutura.
- dimensao: Quantidade de sequencias/campos de organizacao, maior que zero.
- tamanho_max: Limite de registros; zero significa sem limite definido.

Retorno: Ponteiro para a lista inicializada ou NULL para parametros
invalidos ou falha de alocacao. Os criterios de campo sao definidos depois.
*/
LISTA_CRUZADA *lista_cruzada_alocar(int tipo_lista, int dimensao, int tamanho_max){

    if (dimensao <= 0 || tamanho_max < 0 ||
        (tipo_lista != LISTA_BASE && tipo_lista != LISTA_SAIDA))
        return NULL;

    LISTA_CRUZADA *lista = malloc(sizeof(LISTA_CRUZADA));
    if (lista == NULL)
        return NULL;

    lista->listas = malloc(dimensao * sizeof(ITEM **));
    if (lista->listas == NULL){
        free(lista);
        return NULL;
    }

    lista->atuais = malloc(dimensao * sizeof(int));
    if (lista->atuais == NULL){
        free(lista->listas);
        free(lista);
        return NULL;
    }

    int capacidade = CAPACIDADE_INICIAL;

    if (tamanho_max != 0 && capacidade > tamanho_max)
        capacidade = tamanho_max;

    for (int i = 0; i < dimensao; i++){

        lista->listas[i] = malloc(capacidade * sizeof(ITEM *));

        if (lista->listas[i] == NULL){

            for (int j = 0; j < i; j++)
                free(lista->listas[j]);

            free(lista->atuais);
            free(lista->listas);
            free(lista);
            return NULL;
        }

        lista->atuais[i] = 0;
    }

    lista->tipo_lista = tipo_lista;
    lista->dimensao = dimensao;
    lista->criterio.campos = NULL;
    lista->tamanho = 0;
    lista->capacidade_atual = capacidade;
    lista->tamanho_max = tamanho_max;

    return lista;
}

/*
Objetivo: Criar uma lista cruzada para registros da base, preparando os
vetores das dimensoes e armazenando copias independentes dos nomes dos
campos utilizados para ordenar cada uma delas.

Parametros:
- campos: Vetor contendo um nome nao vazio para cada dimensao.
- dimensao: Numero de campos/dimensoes, necessariamente positivo.
- tamanho_max: Limite de registros aceitos; zero indica sem limite fixo.

Retorno: Ponteiro para a lista criada ou NULL caso os parametros sejam
invalidos ou alguma alocacao falhe.
*/
LISTA_CRUZADA *lista_cruzada_criar_base(const char *const *campos, int dimensao, int tamanho_max){

    if (campos == NULL || dimensao <= 0 || tamanho_max < 0)
        return NULL;

    for (int i = 0; i < dimensao; i++){ 
        if (campos[i] == NULL || *campos[i] == '\0')
            return NULL;
    }

    LISTA_CRUZADA *lista = lista_cruzada_alocar(LISTA_BASE, dimensao, tamanho_max);
    if (lista == NULL)
        return NULL;

    lista->criterio.campos = malloc(dimensao * sizeof(char *));
    if (lista->criterio.campos == NULL){
        lista_cruzada_apagar(&lista, false);
        return NULL;
    }

    for (int i = 0; i < dimensao; i++)
        lista->criterio.campos[i] = NULL;

    for (int i = 0; i < dimensao; i++){

        lista->criterio.campos[i] = copiar_string_lista_cruzada(campos[i]);

        if (lista->criterio.campos[i] == NULL){
            lista_cruzada_apagar(&lista, false);
            return NULL;
        }
    }

    return lista;
}

/*
Objetivo: Criar a lista cruzada para os resultados da consulta, usando
as dimensoes definidas por CROSS. Guarda uma referencia a QUERY, que
deve continuar valida enquanto a lista precisar acessar seus campos.

Parametros:
- query: Consulta contendo a diretiva CROSS e campos validos.
- tamanho_max: Limite maximo de registros, sendo zero ausencia de limite.

Retorno: Ponteiro para a lista de saida criada ou NULL para configuracao
invalida ou falha de memoria.
*/
LISTA_CRUZADA *lista_cruzada_criar_saida(QUERY *query, int tamanho_max){

    if (query == NULL || !query_get_possui_cross(query) || tamanho_max < 0)
        return NULL;

    int dimensao = query_get_dimensao_cross(query);

    if (dimensao <= 0)
        return NULL;

    for (int i = 0; i < dimensao; i++){

        const char *campo = query_get_cross_campo(query, i);

        if (campo == NULL || *campo == '\0')
            return NULL;
    }

    LISTA_CRUZADA *lista = lista_cruzada_alocar(LISTA_SAIDA, dimensao, tamanho_max);
    if (lista == NULL)
        return NULL;

    lista->criterio.query = query;

    return lista;
}

/*
Objetivo: Aumentar a capacidade dos vetores de todas as dimensoes quando
necessario, normalmente dobrando o tamanho e respeitando tamanho_max.
Cada vetor e realocado separadamente; se algum realloc falhar, a funcao
interrompe a expansao e retorna false, mantendo os ponteiros validos.

Parametros:
- lista: Lista cruzada cujos vetores devem ser ampliados.

Retorno: true se todos os vetores forem realocados e a capacidade
registrada for atualizada; false para lista invalida, limite atingido
ou falha de alocacao.
*/
bool lista_cruzada_expandir(LISTA_CRUZADA *lista){

    if (lista == NULL)
        return false;

    if (lista->tamanho_max != 0 && lista->tamanho >= lista->tamanho_max)
        return false;

    int nova_capacidade = lista->capacidade_atual * 2;

    if (lista->tamanho_max != 0 && nova_capacidade > lista->tamanho_max)
        nova_capacidade = lista->tamanho_max;

    for (int i = 0; i < lista->dimensao; i++){

        ITEM **novo = realloc(lista->listas[i], nova_capacidade * sizeof(ITEM *));

        if (novo == NULL)
            return false;

        lista->listas[i] = novo;
    }

    lista->capacidade_atual = nova_capacidade;

    return true;
}

/*
Objetivo: Encontrar a posicao de insercao em uma dimensao da lista
cruzada por busca binaria. Os itens sao comparados pelo campo associado
as dimensoes; em caso de empate, a nova entrada fica apos as existentes.

Parametros:
- lista: Lista cruzada que possui o vetor ordenado a pesquisar.
- item: Registro cuja posicao de insercao sera calculada.
- indice: Indice da dimensao que determina o campo de comparacao.

Retorno: Posicao de insercao entre zero e tamanho ou -1 para argumento,
campo ou comparacao invalidos.
*/
int lista_cruzada_busca_binaria(LISTA_CRUZADA *lista, ITEM *item, int indice){

    if (lista == NULL || item == NULL)
        return -1;

    const char *campo = obter_campo(lista, indice);

    if (campo == NULL || comparar_itens(item, item, campo) == COMPARACAO_FALHOU)
        return -1;

    int inicio = 0;
    int fim = lista->tamanho;

    while (inicio < fim){

        int meio = inicio + (fim - inicio) / 2;

        int comparacao = comparar_itens(
            lista->listas[indice][meio], item, campo
        );

        if (comparacao == COMPARACAO_FALHOU)
            return -1;

        if (comparacao <= 0)
            inicio = meio + 1;
        else
            fim = meio;
    }

    return inicio;
}

/*
Objetivo: Inserir o mesmo ponteiro de ITEM nas sequencias de todas as
dimensoes, cada uma ordenada pelo seu proprio campo. Antes de modificar
os vetores, verifica a capacidade e calcula todas as posicoes de insercao.
Em seguida desloca os elementos e atualiza o tamanho compartilhado.

Parametros:
- lista: Lista cruzada que armazenara as referencias ao registro.
- item: Ponteiro para o ITEM a inserir; seus dados nao sao copiados.

Retorno: true se o item for inserido em todas as dimensoes ou false
para argumentos invalidos, limite atingido, comparacao impossivel
ou falha de memoria.
*/
bool lista_cruzada_inserir(LISTA_CRUZADA *lista, ITEM *item){

    if (lista == NULL || item == NULL ||
        (lista->tamanho_max != 0 &&
         lista->tamanho >= lista->tamanho_max))
        return false;

    if (lista->tamanho >= lista->capacidade_atual)
        if (!lista_cruzada_expandir(lista))
            return false;

    int *posicoes = malloc(lista->dimensao * sizeof(int));
    if (posicoes == NULL)
        return false;

    for (int i = 0; i < lista->dimensao; i++){

        posicoes[i] = lista_cruzada_busca_binaria(lista, item, i);

        if (posicoes[i] == -1){
            free(posicoes);
            return false;
        }
    }

    for (int i = 0; i < lista->dimensao; i++){

        int posicao = posicoes[i];

        for (int j = lista->tamanho; j > posicao; j--)
            lista->listas[i][j] = lista->listas[i][j - 1];

        lista->listas[i][posicao] = item;
    }

    lista->tamanho++;
    free(posicoes);

    return true;
}

/*
Objetivo: Recuperar o proximo ITEM do percurso ordenado por um campo
especifico da lista cruzada. Cada dimensao possui seu proprio cursor,
que avanca independentemente das demais.

Parametros:
- lista: Lista cruzada que sera percorrida.
- campo: Nome do campo que identifica a dimensao desejada.

Retorno: Ponteiro para o proximo ITEM nao nulo da dimensao escolhida
ou NULL para parametros invalidos, campo nao encontrado ou fim do percurso.
*/
ITEM *lista_cruzada_percorrer(LISTA_CRUZADA *lista, const char *campo){

    if (lista == NULL || campo == NULL)
        return NULL;

    for (int i = 0; i < lista->dimensao; i++){

        const char *campo_lista = obter_campo(lista, i);

        if (campo_lista != NULL && strcmp(campo_lista, campo) == 0){

            while (lista->atuais[i] < lista->tamanho){

                ITEM *item = lista->listas[i][lista->atuais[i]];
                lista->atuais[i]++;

                if (item != NULL)
                    return item;
            }

            return NULL;
        }
    }

    return NULL;
}

/*
Objetivo: Destruir todos os vetores e o descritor da lista cruzada.
Como o mesmo ITEM aparece nas varias dimensoes, libera cada registro
somente pelo primeiro vetor quando apagar_itens for true. Para listas
de base, libera tambem as copias dos nomes dos campos; nao libera a QUERY.

Parametros:
- lista: Endereco do ponteiro para a lista; recebe NULL apos a destruicao.
- apagar_itens: true para liberar os ITEMs armazenados, false para
preservar registros que possam pertencer a outra estrutura.

Retorno: Nao se aplica (funcao void).
*/
void lista_cruzada_apagar(LISTA_CRUZADA **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    if (apagar_itens){
        for (int i = 0; i < (*lista)->tamanho; i++)
            item_apagar(&(*lista)->listas[0][i]);
    }

    if ((*lista)->tipo_lista == LISTA_BASE && (*lista)->criterio.campos != NULL){

        for (int i = 0; i < (*lista)->dimensao; i++)
            free((*lista)->criterio.campos[i]);

        free((*lista)->criterio.campos);
    }

    for (int i = 0; i < (*lista)->dimensao; i++)
        free((*lista)->listas[i]);

    free((*lista)->listas);
    free((*lista)->atuais);
    free(*lista);
    *lista = NULL;
}
