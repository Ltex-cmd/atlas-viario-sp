#include <stdlib.h>
#include <stdbool.h>

#include "pilha.h"
#include "item.h"

typedef struct no_pilha_ NO_PILHA;

struct pilha_ {

    NO_PILHA *topo;
    NO_PILHA *atual;
    
    int tamanho;
    int tamanho_max;

};

struct no_pilha_ {

    ITEM *item;
    NO_PILHA *anterior;

};


/*
Objetivo: Criar uma pilha encadeada inicialmente vazia. Aloca o descritor,
inicializa os ponteiros de topo e de percurso como NULL e registra o limite
de elementos permitido.

Parametros:
- tamanho_max: Numero maximo de registros na pilha; zero indica que nao ha
  limite fixo, enquanto valores negativos sao considerados invalidos.

Retorno: Ponteiro para a pilha criada ou NULL se o limite for invalido ou
a alocacao do descritor falhar.
*/
PILHA *pilha_criar(int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    PILHA *pilha = (PILHA *) malloc(sizeof(PILHA));
    if (pilha == NULL)
        return NULL;

    pilha->topo = NULL;
    pilha->atual = NULL;

    pilha->tamanho = 0;
    pilha->tamanho_max = tamanho_max;

    return pilha;
}


/*
Objetivo: Empilhar um novo registro, criando um no ligado ao topo anterior.
O elemento recem-inserido passa a ser o novo topo, seguindo a regra LIFO
(ultimo a entrar, primeiro a sair). Reinicia o cursor de percurso no topo.

Parametros:
- pilha: Pilha que recebera o novo registro.
- item: Ponteiro para o ITEM a armazenar; seu conteudo nao e copiado.

Retorno: true se o registro for inserido; false se os parametros forem
invalidos, o limite de capacidade for atingido ou a alocacao falhar.
*/
bool pilha_inserir(PILHA *pilha, ITEM *item){

    if (pilha == NULL || item == NULL || (pilha->tamanho_max != 0 && pilha->tamanho >= pilha->tamanho_max))
        return false;

    NO_PILHA *novo = (NO_PILHA *) malloc(sizeof(NO_PILHA));
    if (novo == NULL)
        return false;

    novo->item = item;
    novo->anterior = pilha->topo;

    pilha->topo = novo;
    pilha->atual = novo;

    pilha->tamanho++;

    return true;
}

/*
Objetivo: Destruir a pilha percorrendo seus nos do topo ate o mais antigo.
Libera sempre os nos e o descritor, podendo liberar tambem os ITEMs quando
eles pertencerem a esta estrutura. Zera o ponteiro da pilha no chamador.

Parametros:
- pilha: Endereco do ponteiro para a pilha; apos a liberacao, *pilha
  recebe NULL. Ponteiro nulo ou pilha inexistente nao causam alteracoes.
- apagar_itens: true para liberar cada ITEM armazenado; false para liberar
  apenas os nos, preservando os registros compartilhados por outra estrutura.

Retorno: Nao se aplica (funcao void).
*/
void pilha_apagar(PILHA **pilha, bool apagar_itens){

    if (pilha == NULL || *pilha == NULL)
        return;

    NO_PILHA *atual = (*pilha)->topo;

    while (atual != NULL){

        NO_PILHA *anterior = atual->anterior;

        if (apagar_itens)
            item_apagar(&atual->item);

        free(atual);
        atual = anterior;
    }

    free(*pilha);
    *pilha = NULL;
}


/*
Objetivo: Recuperar o proximo registro no percurso da pilha, comecando
pelo topo e seguindo os ponteiros anterior. Avanca o cursor a cada no
visitado e ignora nos que nao armazenam ITEM. O percurso nao remove nos.

Parametros:
- pilha: Pilha cujo cursor interno de percurso sera avancado.

Retorno: Ponteiro para o proximo ITEM nao nulo ou NULL quando a pilha
for invalida ou o percurso tiver terminado.
*/
ITEM *pilha_percorrer(PILHA *pilha){

    if (pilha == NULL)
        return NULL;

    while (pilha->atual != NULL){

        ITEM *item = pilha->atual->item;
        pilha->atual = pilha->atual->anterior;

        if (item != NULL)
            return item;
    }

    return NULL;
}
