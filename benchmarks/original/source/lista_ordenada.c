#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lista_ordenada.h"
#include "comparador.h"
#include "processamento_query.h"

#define CAPACIDADE_INICIAL 256

struct lista_ordenada_ {

    ITEM **itens;

    int atual;
    int tamanho;
    int capacidade_atual;
    int tamanho_max;

    int tipo_lista;
    union{
        char *campo;
        QUERY *query;
    } criterio;

};

/*
Objetivo: Criar uma copia independente do nome do campo usado como chave
na lista ordenada da base. A copia permite que o criterio continue valido
mesmo que a string de origem deixe de existir.

Parametros:
- texto: Nome do campo a copiar, terminado em '\0'; pode ser NULL.

Retorno: Ponteiro para a nova string, ou NULL se texto for nulo ou a
alocacao de memoria falhar. O chamador deve liberar a copia.
*/
char *copiar_string_lista_ordenada(const char *texto){

    if (texto == NULL)
        return NULL;

    char *copia = malloc(strlen(texto) + 1);
    if (copia != NULL)
        strcpy(copia, texto);

    return copia;
}

/*
Objetivo: Criar uma lista ordenada para armazenar os registros da base,
utilizando um campo fixo como criterio de comparacao. Reserva o vetor
inicial de ponteiros e guarda uma copia propria do nome do campo.

Parametros:
- campo: Nome nao vazio do atributo usado para manter a ordenacao.
- tamanho_max: Limite de registros; 0 significa sem limite fixo e
  valores negativos sao rejeitados.

Retorno: Ponteiro para a lista criada ou NULL em caso de parametros
invalidos ou falha em alguma alocacao.
*/
LISTA_ORDENADA *lista_ordenada_criar_base(const char *campo, int tamanho_max){

    if (campo == NULL || *campo == '\0' || tamanho_max < 0)
        return NULL;

    LISTA_ORDENADA *lista = (LISTA_ORDENADA *) malloc(sizeof(LISTA_ORDENADA));
    if (lista == NULL)
        return NULL;

    lista->itens = (ITEM **) malloc (sizeof(ITEM *) * CAPACIDADE_INICIAL);
    if (lista->itens == NULL){
        free(lista);
        return NULL;
    }

    lista->atual = 0;
    lista->tamanho = 0;
    lista->capacidade_atual = CAPACIDADE_INICIAL;
    lista->tamanho_max = tamanho_max;
    lista->tipo_lista = LISTA_BASE;
    lista->criterio.campo = copiar_string_lista_ordenada(campo);

    if (lista->criterio.campo == NULL){
        free(lista->itens);
        free(lista);
        return NULL;
    }

    return lista;
}

/*
Objetivo: Criar uma lista ordenada para os resultados de uma consulta.
Usa as configuracoes ORDER da QUERY para determinar a posicao de cada
registro, sem criar uma copia da propria consulta.

Parametros:
- query: Consulta valida que ja possui a diretiva ORDER; deve continuar
  existindo enquanto a lista precisar consultar seu criterio.
- tamanho_max: Limite de registros; 0 significa sem limite fixo e
  valores negativos sao rejeitados.

Retorno: Ponteiro para a lista de saida criada, ou NULL se faltar ORDER,
os parametros forem invalidos ou alguma alocacao falhar.
*/
LISTA_ORDENADA *lista_ordenada_criar_saida(QUERY *query, int tamanho_max){

    if (query == NULL || !query_get_possui_ordenacao(query) || tamanho_max < 0)
        return NULL;

    LISTA_ORDENADA *lista = (LISTA_ORDENADA *) malloc(sizeof(LISTA_ORDENADA));
    if (lista == NULL)
        return NULL;

    lista->itens = (ITEM **) malloc (sizeof(ITEM *) * CAPACIDADE_INICIAL);
    if (lista->itens == NULL){
        free(lista);
        return NULL;
    }

    lista->atual = 0;
    lista->tamanho = 0;
    lista->capacidade_atual = CAPACIDADE_INICIAL;
    lista->tamanho_max = tamanho_max;
    lista->tipo_lista = LISTA_SAIDA;
    lista->criterio.query = query;

    return lista;
}

/*
Objetivo: Localizar, por busca binaria, a posicao em que um registro
deve ser inserido para manter o vetor ordenado. Para a base, utiliza
comparar_itens; para a saida, utiliza comparar_itens_order. Em caso
de empate, escolhe a posicao apos os registros com a mesma chave.

Parametros:
- lista: Lista ordenada cujo vetor sera pesquisado.
- item: Registro que sera comparado com os elementos existentes.

Retorno: Indice da posicao de insercao, de 0 ate o tamanho atual,
ou -1 se houver argumentos, tipo ou comparacoes invalidos.
*/
int busca_binaria(LISTA_ORDENADA *lista, ITEM *item){

    if (lista == NULL || item == NULL)
        return -1;

    int validacao;

    if (lista->tipo_lista == LISTA_BASE)
        validacao = comparar_itens(item, item, lista->criterio.campo);
    else if (lista->tipo_lista == LISTA_SAIDA)
        validacao = comparar_itens_order(item, item, lista->criterio.query);
    else
        return -1;

    if (validacao == COMPARACAO_FALHOU)
        return -1;

    int inicio = 0;
    int fim = lista->tamanho;

    while (inicio < fim){

        int meio = inicio + (fim - inicio) / 2;
        int comparacao;

        if (lista->tipo_lista == LISTA_BASE)
            comparacao = comparar_itens(
                lista->itens[meio], item, lista->criterio.campo
            );
        else
            comparacao = comparar_itens_order(
                lista->itens[meio], item, lista->criterio.query
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
Objetivo: Inserir um ITEM na posicao indicada pela busca binaria, sem
perder a ordem dos registros. Se preciso, amplia o vetor com realloc
e desloca os elementos posteriores uma posicao para a direita.

Parametros:
- lista: Lista ordenada em que sera armazenado o ponteiro do registro.
- item: Registro a inserir; os dados do ITEM nao sao copiados.

Retorno: true se a insercao for concluida; false em caso de argumentos
invalidos, limite atingido, comparacao impossivel ou falha de memoria.
*/
bool lista_ordenada_inserir(LISTA_ORDENADA *lista, ITEM *item){

    if (lista == NULL || item == NULL ||
        (lista->tamanho_max != 0 &&
         lista->tamanho >= lista->tamanho_max))
        return false;

    int posicao = busca_binaria(lista, item);

    if (posicao == -1)
        return false;

    if (lista->capacidade_atual == lista->tamanho){

        int nova_capacidade = lista->capacidade_atual * 2;

        if (lista->tamanho_max != 0 && nova_capacidade > lista->tamanho_max)
            nova_capacidade = lista->tamanho_max;

        ITEM **novo = realloc(
            lista->itens, sizeof(ITEM *) * nova_capacidade
        );

        if (novo == NULL)
            return false;

        lista->itens = novo;
        lista->capacidade_atual = nova_capacidade;
    }

    for (int i = lista->tamanho; i > posicao; i--)
        lista->itens[i] = lista->itens[i - 1];

    lista->itens[posicao] = item;
    lista->tamanho++;

    return true;
}

/*
Objetivo: Liberar os recursos da lista ordenada e zerar o ponteiro do
chamador. Libera opcionalmente os ITEMs armazenados. A lista da base
possui uma copia do campo de ordenacao, que tambem e liberada;
a lista de saida apenas referencia a QUERY, sem libera-la.

Parametros:
- lista: Endereco do ponteiro da lista a destruir; recebe NULL ao final.
- apagar_itens: true para liberar tambem os ITEMs; false para preservar
  os registros que possam estar compartilhados com outras estruturas.

Retorno: Nao se aplica (funcao void).
*/
void lista_ordenada_apagar(LISTA_ORDENADA **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    if (apagar_itens){
        for (int i = 0; i < (*lista)->tamanho; i++)
            item_apagar(&(*lista)->itens[i]);
    }

    if ((*lista)->tipo_lista == LISTA_BASE)
        free((*lista)->criterio.campo);

    free((*lista)->itens);
    free(*lista);
    *lista = NULL;
}

/*
Objetivo: Percorrer sequencialmente o vetor ja ordenado, a partir do
indice atual. Avanca o cursor a cada posicao e ignora ponteiros NULL.

Parametros:
- lista: Lista ordenada cujo proximo registro sera consultado.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL se a lista for
invalida ou nao restarem registros a percorrer.
*/
ITEM *lista_ordenada_percorrer(LISTA_ORDENADA *lista){

    if (lista == NULL)
        return NULL;

    while (lista->atual < lista->tamanho){

        ITEM *item = lista->itens[lista->atual];
        lista->atual++;

        if (item != NULL)
            return item;
    }

    return NULL;
}
