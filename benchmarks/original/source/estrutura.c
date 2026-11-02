#include <stdlib.h>
#include <string.h>

#include "estrutura.h"
#include "pilha.h"
#include "fila.h"
#include "lista_sequencial.h"
#include "lista_encadeada.h"
#include "lista_cabeca.h"
#include "lista_ordenada.h"
#include "lista_generalizada.h"
#include "lista_cruzada.h"
#include "tipo_lista.h"

struct estrutura_ {

    int tipo;

    union {
        PILHA *pilha;
        FILA *fila;
        LISTA_SEQUENCIAL *lista_sequencial;
        LISTA_ENCADEADA *lista_encadeada;
        LISTA_CABECA *lista_cabeca;
        LISTA_ORDENADA *lista_ordenada;
        LISTA_GENERALIZADA *lista_generalizada;
        LISTA_CRUZADA *lista_cruzada;
    } dados;

};

/*
Objetivo: Interpretar o nome da estrutura, alocar o envoltorio
ESTRUTURA e registrar o tipo correspondente. A estrutura
especifica (pilha, fila ou lista) ainda nao e criada nesta etapa.

Parametros:
- tipo: Nome da estrutura solicitada (pilha, fila, lista_seq, lista_enc,
  lista_cabeca, lista_ord, lista_gen ou lista_cruz).

Retorno: Ponteiro para o envoltorio inicializado ou NULL se o nome
for invalido ou a alocacao falhar.
*/
static ESTRUTURA *criar_estrutura(const char *tipo){

    if (tipo == NULL)
        return NULL;

    int tipo_estrutura;

    if (strcmp(tipo, "pilha") == 0)
        tipo_estrutura = ESTRUTURA_PILHA;
    else if (strcmp(tipo, "fila") == 0)
        tipo_estrutura = ESTRUTURA_FILA;
    else if (strcmp(tipo, "lista_seq") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_SEQUENCIAL;
    else if (strcmp(tipo, "lista_enc") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_ENCADEADA;
    else if (strcmp(tipo, "lista_cabeca") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_CABECA;
    else if (strcmp(tipo, "lista_ord") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_ORDENADA;
    else if (strcmp(tipo, "lista_gen") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_GENERALIZADA;
    else if (strcmp(tipo, "lista_cruz") == 0)
        tipo_estrutura = ESTRUTURA_LISTA_CRUZADA;
    else
        return NULL;

    ESTRUTURA *estrutura = calloc(1, sizeof(ESTRUTURA));
    if (estrutura == NULL)
        return NULL;

    estrutura->tipo = tipo_estrutura;
    return estrutura;
}

/*
Objetivo: Criar a estrutura de dados que armazenara os registros da base.
Primeiro cria o envoltorio generico e depois inicializa a pilha, fila ou
lista correspondente. Listas ordenadas, generalizadas e cruzadas podem
receber campos que determinam sua organizacao. Em falha, libera o
envoltorio e retorna NULL.

Parametros:
- tipo: Nome da estrutura escolhida para armazenar a base.
- campos: Vetor de nomes dos campos de organizacao; usado pelas listas
  ordenada, generalizada e cruzada, podendo ser NULL para outros tipos.
- quantidade_campos: Numero de nomes de campos presentes em campos;
  a lista ordenada exige exatamente um.
- tamanho_max: Limite maximo de itens; 0 representa ausencia de limite
  fixo, e valores negativos sao rejeitados.

Retorno: Ponteiro para a ESTRUTURA criada ou NULL em caso de parametros
invalidos ou falha na criacao da estrutura interna.
*/
ESTRUTURA *estrutura_criar_base(const char *tipo, const char *const *campos, int quantidade_campos, int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    ESTRUTURA *estrutura = criar_estrutura(tipo);
    if (estrutura == NULL)
        return NULL;

    /* Cada caso instancia apenas a implementacao escolhida pelo usuario. */
    switch (estrutura->tipo){

        case ESTRUTURA_PILHA:
            estrutura->dados.pilha = pilha_criar(tamanho_max);

            if (estrutura->dados.pilha == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_FILA:
            estrutura->dados.fila = fila_criar(tamanho_max);

            if (estrutura->dados.fila == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_SEQUENCIAL:
            estrutura->dados.lista_sequencial = lista_sequencial_criar(tamanho_max);

            if (estrutura->dados.lista_sequencial == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_ENCADEADA:
            estrutura->dados.lista_encadeada = lista_encadeada_criar(tamanho_max);

            if (estrutura->dados.lista_encadeada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_CABECA:
            estrutura->dados.lista_cabeca = lista_cabeca_criar(tamanho_max);

            if (estrutura->dados.lista_cabeca == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_ORDENADA:
            if (campos == NULL || quantidade_campos != 1){
                free(estrutura);
                return NULL;
            }

            estrutura->dados.lista_ordenada = lista_ordenada_criar_base(campos[0], tamanho_max);

            if (estrutura->dados.lista_ordenada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_GENERALIZADA:
            estrutura->dados.lista_generalizada = lista_generalizada_criar_base(campos, quantidade_campos, tamanho_max);

            if (estrutura->dados.lista_generalizada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_CRUZADA:
            estrutura->dados.lista_cruzada = lista_cruzada_criar_base(campos, quantidade_campos, tamanho_max);

            if (estrutura->dados.lista_cruzada == NULL){
                free(estrutura);
                return NULL;
            }
            break;
        default:
            free(estrutura);
            return NULL;
    }

    return estrutura;
}

/*
Objetivo: Criar a estrutura destinada aos resultados da consulta.
Escolhe a implementacao pelo nome e encaminha a QUERY as estruturas
cuja criacao depende das diretivas de consulta, como ORDER, GROUP e
CROSS. Em caso de falha, libera o envoltorio alocado.

Parametros:
- tipo: Nome da estrutura escolhida para armazenar os resultados.
- query: Consulta com as configuracoes de ordenacao ou organizacao que
  podem ser usadas pelas listas ordenada, generalizada e cruzada.
- tamanho_max: Limite maximo de itens; 0 significa sem limite fixo
  e valores negativos nao sao aceitos.

Retorno: Ponteiro para a ESTRUTURA de saida inicializada ou NULL caso
a escolha seja invalida ou ocorra falha de criacao.
*/
ESTRUTURA *estrutura_criar_saida(const char *tipo, QUERY *query, int tamanho_max){

    if (tamanho_max < 0)
        return NULL;

    ESTRUTURA *estrutura = criar_estrutura(tipo);
    if (estrutura == NULL)
        return NULL;

    switch (estrutura->tipo){

        case ESTRUTURA_PILHA:
            estrutura->dados.pilha = pilha_criar(tamanho_max);

            if (estrutura->dados.pilha == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_FILA:
            estrutura->dados.fila = fila_criar(tamanho_max);

            if (estrutura->dados.fila == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_SEQUENCIAL:
            estrutura->dados.lista_sequencial = lista_sequencial_criar(tamanho_max);

            if (estrutura->dados.lista_sequencial == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_ENCADEADA:
            estrutura->dados.lista_encadeada = lista_encadeada_criar(tamanho_max);

            if (estrutura->dados.lista_encadeada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_CABECA:
            estrutura->dados.lista_cabeca = lista_cabeca_criar(tamanho_max);

            if (estrutura->dados.lista_cabeca == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_ORDENADA:
            estrutura->dados.lista_ordenada = lista_ordenada_criar_saida(query, tamanho_max);

            if (estrutura->dados.lista_ordenada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_GENERALIZADA:
            estrutura->dados.lista_generalizada = lista_generalizada_criar_saida(query, tamanho_max);

            if (estrutura->dados.lista_generalizada == NULL){
                free(estrutura);
                return NULL;
            }
            break;

        case ESTRUTURA_LISTA_CRUZADA:
            estrutura->dados.lista_cruzada = lista_cruzada_criar_saida(query, tamanho_max);

            if (estrutura->dados.lista_cruzada == NULL){
                free(estrutura);
                return NULL;
            }
            break;
        default:
            free(estrutura);
            return NULL;
    }

    return estrutura;
}

/*
Objetivo: Inserir um ITEM na estrutura generica, delegando a operacao
a implementacao concreta armazenada na union dados. As regras de
capacidade, ordem e alocacao pertencem a cada estrutura especifica.

Parametros:
- estrutura: Envoltorio da pilha, fila ou lista que recebera o item.
- item: Ponteiro para o registro a inserir; nao e copiado nesta funcao.

Retorno: true se a implementacao interna concluir a insercao; false
para parametros invalidos, tipo desconhecido ou falha de insercao.
*/
bool estrutura_inserir(ESTRUTURA *estrutura, ITEM *item){

    /* O envoltorio despacha; a estrutura especifica efetua a insercao. */
    if (estrutura == NULL || item == NULL)
        return false;

    switch (estrutura->tipo){

        case ESTRUTURA_PILHA:
            return pilha_inserir(estrutura->dados.pilha, item);

        case ESTRUTURA_FILA:
            return fila_inserir(estrutura->dados.fila, item);

        case ESTRUTURA_LISTA_SEQUENCIAL:
            return lista_sequencial_inserir(estrutura->dados.lista_sequencial, item);

        case ESTRUTURA_LISTA_ENCADEADA:
            return lista_encadeada_inserir(estrutura->dados.lista_encadeada, item);

        case ESTRUTURA_LISTA_CABECA:
            return lista_cabeca_inserir(estrutura->dados.lista_cabeca, item);

        case ESTRUTURA_LISTA_ORDENADA:
            return lista_ordenada_inserir(estrutura->dados.lista_ordenada, item);

        case ESTRUTURA_LISTA_GENERALIZADA:
            return lista_generalizada_inserir(estrutura->dados.lista_generalizada, item);

        case ESTRUTURA_LISTA_CRUZADA:
            return lista_cruzada_inserir(estrutura->dados.lista_cruzada, item);

        default:
            return false;
    }
}


/*
Objetivo: Obter o proximo registro da estrutura conforme sua ordem de
percurso. Delega a consulta a implementacao concreta; na lista cruzada,
o campo determina qual dimensao sera percorrida.

Parametros:
- estrutura: Envoltorio da estrutura que sera percorrida.
- campo: Nome do campo que seleciona a dimensao na lista cruzada;
  nao e utilizado pelas demais implementacoes.

Retorno: Ponteiro para o proximo ITEM disponivel ou NULL se a estrutura
for invalida, o percurso terminar ou nao houver item acessivel.
*/
ITEM *estrutura_percorrer(ESTRUTURA *estrutura, const char *campo){


    if (estrutura == NULL)
        return NULL;

    switch (estrutura->tipo){

        case ESTRUTURA_PILHA:
            return pilha_percorrer(estrutura->dados.pilha);

        case ESTRUTURA_FILA:
            return fila_percorrer(estrutura->dados.fila);

        case ESTRUTURA_LISTA_SEQUENCIAL:
            return lista_sequencial_percorrer(estrutura->dados.lista_sequencial);

        case ESTRUTURA_LISTA_ENCADEADA:
            return lista_encadeada_percorrer(estrutura->dados.lista_encadeada);

        case ESTRUTURA_LISTA_CABECA:
            return lista_cabeca_percorrer(estrutura->dados.lista_cabeca);

        case ESTRUTURA_LISTA_ORDENADA:
            return lista_ordenada_percorrer(estrutura->dados.lista_ordenada);

        case ESTRUTURA_LISTA_GENERALIZADA:
            return lista_generalizada_percorrer(estrutura->dados.lista_generalizada);

        case ESTRUTURA_LISTA_CRUZADA:
            return lista_cruzada_percorrer(estrutura->dados.lista_cruzada, campo);

        default:
            return NULL;
    }
}

/*
Objetivo: Destruir a implementacao concreta e liberar o envoltorio
ESTRUTURA. Permite escolher se os ITEMs armazenados tambem devem ser
liberados, evitando liberar registros compartilhados entre base e saida.
Ao concluir com um tipo valido, zera o ponteiro do chamador.

Parametros:
- estrutura: Endereco do ponteiro da estrutura a destruir; quando
  aplicavel, *estrutura passa a NULL apos a liberacao.
- apagar_itens: true para liberar tambem os ITEMs armazenados ou false
  para destruir somente a estrutura e manter os registros vivos.

Retorno: Nao se aplica (funcao void).
*/
void estrutura_apagar(ESTRUTURA **estrutura, bool apagar_itens){

    if (estrutura == NULL || *estrutura == NULL)
        return;

    switch ((*estrutura)->tipo){

        case ESTRUTURA_PILHA:
            pilha_apagar(&(*estrutura)->dados.pilha, apagar_itens);
            break;

        case ESTRUTURA_FILA:
            fila_apagar(&(*estrutura)->dados.fila, apagar_itens);
            break;

        case ESTRUTURA_LISTA_SEQUENCIAL:
            lista_sequencial_apagar(&(*estrutura)->dados.lista_sequencial, apagar_itens);
            break;

        case ESTRUTURA_LISTA_ENCADEADA:
            lista_encadeada_apagar(&(*estrutura)->dados.lista_encadeada, apagar_itens);
            break;

        case ESTRUTURA_LISTA_CABECA:
            lista_cabeca_apagar(&(*estrutura)->dados.lista_cabeca, apagar_itens);
            break;

        case ESTRUTURA_LISTA_ORDENADA:
            lista_ordenada_apagar(&(*estrutura)->dados.lista_ordenada, apagar_itens);
            break;

        case ESTRUTURA_LISTA_GENERALIZADA:
            lista_generalizada_apagar(&(*estrutura)->dados.lista_generalizada, apagar_itens);
            break;

        case ESTRUTURA_LISTA_CRUZADA:
            lista_cruzada_apagar(&(*estrutura)->dados.lista_cruzada, apagar_itens);
            break;

        default:
            return;
    }

    free(*estrutura);
    *estrutura = NULL;
}
