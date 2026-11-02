
#include <stdlib.h>
#include <stdbool.h>

#include "lista_cabeca.h"
#include "item.h"

typedef struct no_lista_cabeca_ NO_LISTA_CABECA;

struct no_lista_cabeca_ {

    ITEM *item;
    NO_LISTA_CABECA *proximo;

};

struct lista_cabeca_ {

    NO_LISTA_CABECA *cabeca;
    NO_LISTA_CABECA *fim;
    NO_LISTA_CABECA *atual;

    int tamanho;
    int tamanho_max;

};


/*
Objetivo: Criar uma lista simplesmente encadeada com no-cabeca sentinela.
Aloca o descritor e o no-cabeca, inicializa os ponteiros e os contadores,
deixando a lista vazia e pronta para receber itens.

Parametros:
- tamanho_max: Quantidade maxima permitida de itens reais; zero indica
  ausencia de limite e valores negativos sao rejeitados.

Retorno: Ponteiro para a lista inicializada ou NULL se o limite for
invalido ou alguma das alocacoes falhar.
*/
LISTA_CABECA *lista_cabeca_criar(int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    LISTA_CABECA *lista = (LISTA_CABECA *) malloc(sizeof(LISTA_CABECA));
    if (lista == NULL)
        return NULL;

    NO_LISTA_CABECA *cabeca = (NO_LISTA_CABECA *) malloc(sizeof(NO_LISTA_CABECA));
    if (cabeca == NULL){
        free(lista);
        return NULL;
    }

    /* O no-cabeca e sentinela: nao representa um registro da base. */
    cabeca->item = NULL;
    cabeca->proximo = NULL;

    lista->cabeca = cabeca;

    /* Em uma lista vazia, o ultimo no e a propria sentinela. */
    lista->fim = cabeca;
    lista->atual = NULL;

    lista->tamanho = 0;
    lista->tamanho_max = tamanho_max;

    return lista;
}

/*
Objetivo: Inserir um ITEM no fim da lista com no-cabeca, alocando um
novo no e ligando-o apos o ultimo. Respeita o limite configurado e
inicializa o cursor de percurso quando a lista recebe seu primeiro item.

Parametros:
- lista: Lista com no-cabeca que recebera o novo registro.
- item: Ponteiro para o ITEM a armazenar; o conteudo nao e copiado.

Retorno: true quando a insercao e concluida; false se algum parametro
for invalido, o limite tiver sido atingido ou a alocacao falhar.
*/
bool lista_cabeca_inserir(LISTA_CABECA *lista, ITEM *item){

    if (lista == NULL || item == NULL || (lista->tamanho_max != 0 && lista->tamanho >= lista->tamanho_max))
        return false;

    NO_LISTA_CABECA *novo = (NO_LISTA_CABECA *) malloc(sizeof(NO_LISTA_CABECA));
    if (novo == NULL)
        return false;

    novo->item = item;
    novo->proximo = NULL;

    lista->fim->proximo = novo;
    lista->fim = novo;

    if (lista->cabeca->proximo == novo)
        lista->atual = novo;

    lista->tamanho++;

    return true;
}

/*
Objetivo: Retornar o proximo ITEM no percurso da lista, avancando
pelos ponteiros proximo e ignorando nos com item nulo. O no-cabeca nao
faz parte do percurso; ao chegar ao final, o cursor permanece nulo.

Parametros:
- lista: Lista que contem o cursor do percurso atual.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL quando a lista
for invalida ou nao houver mais registros no percurso.
*/
ITEM *lista_cabeca_percorrer(LISTA_CABECA *lista){

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

/*
Objetivo: Destruir todos os nos da lista, incluindo o no-cabeca, e
liberar seu descritor. Conforme apagar_itens, tambem libera os ITEMs
armazenados, permitindo compartilhar registros sem dupla liberacao.
Ao final, define o ponteiro da lista como NULL.

Parametros:
- lista: Endereco do ponteiro para a lista a destruir; ao final,
  *lista e definido como NULL, caso tenha sido liberada.
- apagar_itens: true para liberar os ITEMs; false para liberar apenas
  os nos e o descritor, preservando os registros apontados.

Retorno: Nao se aplica (funcao void).
*/
void lista_cabeca_apagar(LISTA_CABECA **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    NO_LISTA_CABECA *atual = (*lista)->cabeca;

    while (atual != NULL){

        NO_LISTA_CABECA *proximo = atual->proximo;

        if (apagar_itens)
            item_apagar(&atual->item);

        free(atual);
        atual = proximo;
    }

    free(*lista);
    *lista = NULL;
}
