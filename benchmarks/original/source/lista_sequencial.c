#include <stdlib.h>
#include <stdbool.h>

#include "lista_sequencial.h"
#include "item.h"

#define CAPACIDADE_INICIAL 256

struct lista_sequencial_ {

    ITEM **itens;

    int atual;

    int tamanho;
    int capacidade_atual;
    int tamanho_max;

};

/*
Objetivo: Criar uma lista sequencial que armazena ponteiros para ITEM em
um vetor dinamico. Reserva inicialmente CAPACIDADE_INICIAL posicoes e
inicializa o cursor de percurso, a quantidade de elementos e o limite
logico de insercoes. O vetor pode ter capacidade fisica maior que o
limite solicitado.

Parametros:
- tamanho_max: Quantidade maxima de registros permitida; 0 indica que
  nao ha limite logico, e valores negativos sao rejeitados.

Retorno: Ponteiro para a LISTA_SEQUENCIAL criada ou NULL se o limite for
invalido ou alguma alocacao de memoria falhar.
*/
LISTA_SEQUENCIAL *lista_sequencial_criar(int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    LISTA_SEQUENCIAL *lista = (LISTA_SEQUENCIAL *) malloc(sizeof(LISTA_SEQUENCIAL));
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

    return lista;
}


/*
Objetivo: Inserir um ponteiro para ITEM ao final da lista sequencial.
Recusa itens nulos e insercoes acima do limite configurado. Quando o
vetor fica cheio, tenta aumentar sua capacidade com realloc, dobrando-a
ou respeitando tamanho_max. A insercao reinicia o cursor de percurso.

Parametros:
- lista: Lista sequencial que recebera o registro.
- item: Ponteiro para o ITEM a armazenar; o registro nao e copiado.

Retorno: true se o item foi inserido; false para parametros invalidos,
limite maximo atingido ou falha na realocacao.
*/
bool lista_sequencial_inserir(LISTA_SEQUENCIAL *lista, ITEM *item){

    if (lista == NULL || item == NULL || (lista->tamanho_max != 0 && lista->tamanho >= lista->tamanho_max))
        return false;

    if (lista->capacidade_atual == lista->tamanho){

        int nova_capacidade = lista->capacidade_atual * 2;
        if (nova_capacidade > lista->tamanho_max && lista->tamanho_max != 0)
            nova_capacidade = lista->tamanho_max;

        ITEM **novo = (ITEM **) realloc (lista->itens, sizeof(ITEM *) * nova_capacidade);
        if (novo == NULL)
            return false;

        lista->capacidade_atual = nova_capacidade;
        lista->itens = novo;

    }


    lista->itens[lista->tamanho] = item;
    lista->tamanho++;

    lista->atual = 0;

    return true;
}

/*
Objetivo: Destruir a lista sequencial, liberando o vetor de ponteiros
e a estrutura da lista. Opcionalmente libera tambem os ITEMs apontados
pelo vetor. Ao final, define o ponteiro do chamador como NULL.

Parametros:
- lista: Endereco do ponteiro para a lista a ser destruida; ao final,
  *lista recebe NULL, se a lista existia.
- apagar_itens: true para chamar item_apagar em cada registro armazenado;
  false para manter os ITEMs vivos e liberar apenas a lista e seu vetor.

Retorno: Nao se aplica (funcao void).
*/
void lista_sequencial_apagar(LISTA_SEQUENCIAL **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    if (apagar_itens){

        for (int i = 0; i < (*lista)->tamanho; i++)
            item_apagar(&(*lista)->itens[i]);
    
    }

    free((*lista)->itens);
    free(*lista);
    *lista = NULL;
}


/*
Objetivo: Retornar o proximo registro disponivel na ordem do vetor.
Utiliza o indice atual como cursor, avancando a cada posicao visitada
e ignorando ponteiros NULL. Uma nova insercao reinicia o cursor.

Parametros:
- lista: Lista sequencial cujo proximo registro sera consultado.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL se a lista for
invalida ou o percurso ja tiver chegado ao final.
*/
ITEM *lista_sequencial_percorrer(LISTA_SEQUENCIAL *lista){

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
