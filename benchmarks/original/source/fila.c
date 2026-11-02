#include <stdlib.h>
#include <stdbool.h>

#include "fila.h"
#include "item.h"

typedef struct no_fila_ NO_FILA;

struct fila_ {

    NO_FILA *inicio;
    NO_FILA *fim;
    NO_FILA *atual; //usado para conseguir percorrer a fila para impressao //

    int tamanho;
    int tamanho_max;

};

struct no_fila_ {

    ITEM *item;
    NO_FILA *proximo;

};

/*
Objetivo: Criar uma fila encadeada inicialmente vazia, alocando seu descritor
 e inicializando os ponteiros de inicio, fim e percurso como NULL. Armazena
 tambem o limite de registros permitido.

Parametros:
- tamanho_max: Numero maximo de ITEMs aceitos na fila; zero indica ausencia
  de limite e valores negativos nao sao aceitos.

Retorno: Ponteiro para a fila inicializada ou NULL quando o limite for
 invalido ou a alocacao de memoria falhar.
*/
FILA *fila_criar(int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    FILA *fila = (FILA *) malloc(sizeof(FILA));
    if (fila == NULL)
        return NULL;

    fila->inicio = NULL;
    fila->fim = NULL;
    fila->atual = NULL;
    
    fila->tamanho = 0;
    fila->tamanho_max = tamanho_max;

    return fila;
}


/*
Objetivo: Enfileirar um registro ao final da lista de nos, preservando a
 ordem FIFO (primeiro a entrar, primeiro a sair). Aloca um novo no, liga-o
 ao antigo fim e, na primeira insercao, inicializa inicio e cursor de
 percurso. Nao copia o conteudo do ITEM nem reinicia o cursor nas
 insercoes seguintes.

Parametros:
- fila: Fila que recebera o novo registro.
- item: Ponteiro para o ITEM a armazenar, sem copiar seus dados.

Retorno: true quando a insercao for concluida; false para ponteiros nulos,
 limite de registros atingido ou falha na alocacao do novo no.
*/
bool fila_inserir(FILA *fila, ITEM *item){

    if (fila == NULL || item == NULL || (fila->tamanho_max != 0 && fila->tamanho >= fila->tamanho_max))
        return false;

    NO_FILA *novo = (NO_FILA *) malloc(sizeof(NO_FILA));
    if (novo == NULL)
        return false;

    novo->item = item;
    novo->proximo = NULL;

    if (fila->fim == NULL){
        fila->inicio = novo;
        fila->atual = novo;
    }
    else
        fila->fim->proximo = novo;

    fila->fim = novo;

    fila->tamanho++;
    return true;
}

/*
Objetivo: Destruir os nos encadeados desde o inicio da fila e liberar seu
 descritor. Permite escolher se os ITEMs tambem serao destruidos, evitando
 a liberacao dupla de registros compartilhados com outras estruturas.
 Ao final, define como NULL o ponteiro da fila no chamador.

Parametros:
- fila: Endereco do ponteiro para a fila a ser destruida; quando valido,
  *fila passa a NULL apos a liberacao.
- apagar_itens: true para destruir tambem os ITEMs armazenados; false para
  liberar somente os nos e o descritor, preservando os registros.

Retorno: Nao se aplica (funcao void).
*/
void fila_apagar(FILA **fila, bool apagar_itens){

    if (fila == NULL || *fila == NULL)
        return;

    NO_FILA *atual = (*fila)->inicio;

    while (atual != NULL){
        NO_FILA *proximo = atual->proximo;

        if (apagar_itens)
            item_apagar(&atual->item);

        free(atual);
        atual = proximo;
    }

    free(*fila);
    *fila = NULL;
}

/*
Objetivo: Retornar os registros na ordem em que foram inseridos, avancando
 pelo ponteiro proximo a partir do cursor interno. Ignora nos com ITEM nulo
 e nao remove elementos da fila. O cursor fica em NULL ao terminar o
 percurso.

Parametros:
- fila: Fila cujo proximo registro sera consultado pelo cursor atual.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL caso a fila seja
 invalida ou nao restem registros no percurso.
*/
ITEM *fila_percorrer(FILA *fila){

    if (fila == NULL)
        return NULL;

    while (fila->atual != NULL){
        ITEM *item = fila->atual->item;
        fila->atual = fila->atual->proximo;

        if (item != NULL)
            return item;
    }

    return NULL;
}
