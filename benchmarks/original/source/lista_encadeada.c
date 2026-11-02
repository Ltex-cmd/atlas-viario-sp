#include <stdlib.h>
#include <stdbool.h>

#include "lista_encadeada.h"
#include "item.h"

typedef struct no_lista_encadeada_ NO_LISTA_ENCADEADA;

struct lista_encadeada_ {

    NO_LISTA_ENCADEADA *inicio;
    NO_LISTA_ENCADEADA *fim;
    NO_LISTA_ENCADEADA *atual;

    int tamanho;
    int tamanho_max;

};

struct no_lista_encadeada_ {

    ITEM *item;
    NO_LISTA_ENCADEADA *proximo;

};

/*
Objetivo: Criar uma lista simplesmente encadeada vazia. Aloca o descritor
LISTA_ENCADEADA e inicializa os ponteiros de inicio, fim e percurso como
NULL, sem alocar nos antecipadamente.

Parametros:
- tamanho_max: Quantidade maxima de itens permitida na lista; 0 indica
  ausencia de limite e valores negativos sao rejeitados.

Retorno: Ponteiro para a lista criada ou NULL se o limite for invalido
ou se a alocacao do descritor falhar.
*/
LISTA_ENCADEADA *lista_encadeada_criar(int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    LISTA_ENCADEADA *lista = (LISTA_ENCADEADA *) malloc(sizeof(LISTA_ENCADEADA));
    if (lista == NULL)
        return NULL;

    lista->inicio = NULL;
    lista->fim = NULL;
    lista->atual = NULL;

    lista->tamanho = 0;
    lista->tamanho_max = tamanho_max;
    
    return lista;
}

/*Objetivo: Acrescentar um novo no ao final da lista encadeada, contendo
o ponteiro para o ITEM recebido. Respeita o limite configurado e atualiza
a ligacao do ultimo no. Se for a primeira insercao, inicia tambem o
ponteiro de percurso; insercoes posteriores nao reiniciam o percurso.

Parametros:
- lista: Lista encadeada que recebera o registro.
- item: Ponteiro para o registro que sera armazenado, sem copia dos dados.

Retorno: true quando o novo no e inserido; false se a lista ou o ITEM
for invalido, o limite tiver sido atingido ou a alocacao falhar.
*/
bool lista_encadeada_inserir(LISTA_ENCADEADA *lista, ITEM *item){

    if (lista == NULL || item == NULL || (lista->tamanho_max && lista->tamanho >= lista->tamanho_max))
        return false;

    NO_LISTA_ENCADEADA *novo = (NO_LISTA_ENCADEADA *) malloc(sizeof(NO_LISTA_ENCADEADA));
    if (novo == NULL)
        return false;

    novo->item = item;
    novo->proximo = NULL;

    if (lista->fim == NULL){
        lista->inicio = novo;
        lista->atual = novo;
    }
    else
        lista->fim->proximo = novo;

    lista->fim = novo;
    
    lista->tamanho++;

    return true;
}

/*
Objetivo: Destruir todos os nos e o descritor da lista. Percorre as
ligacoes a partir do inicio, guardando o proximo no antes de liberar
o atual. Permite liberar os registros armazenados ou preservar seus
ITEMs para evitar dupla liberacao em estruturas que compartilham dados.

Parametros:
- lista: Endereco do ponteiro para a lista que sera destruida;
  ao final, *lista recebe NULL.
- apagar_itens: true para liberar tambem cada ITEM por item_apagar;
  false para liberar apenas os nos e o descritor da lista.

Retorno: Nao se aplica (funcao void).
*/
void lista_encadeada_apagar(LISTA_ENCADEADA **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    NO_LISTA_ENCADEADA *atual = (*lista)->inicio;

    while (atual != NULL){
        NO_LISTA_ENCADEADA *proximo = atual->proximo;

        if (apagar_itens)
            item_apagar(&atual->item);

        free(atual);
        atual = proximo;
    }

    free(*lista);
    *lista = NULL;
}


/*
Objetivo: Retornar o proximo ITEM disponivel no percurso da lista,
acompanhando as ligacoes entre os nos. Avanca o cursor a cada no e
ignora nos cujo ponteiro para ITEM seja NULL. Ao atingir o fim,
o cursor permanece em NULL.

Parametros:
- lista: Lista encadeada que possui o cursor de percurso atual.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL quando a lista
for invalida ou todos os nos do percurso ja tiverem sido visitados.
*/
ITEM *lista_encadeada_percorrer(LISTA_ENCADEADA *lista){

    if (lista == NULL)
        return NULL;

    while (lista->atual != NULL){
        ITEM *item = lista->atual->item;
        lista->atual = lista->atual->proximo;

        if (item != NULL)
            return item;
    }

    return NULL;
}
