#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lista_generalizada.h"
#include "tipo_lista.h"
#include "comparador.h"

#define TIPO_ATOMO 0
#define TIPO_GRUPO 1
#define CAPACIDADE_INICIAL 32

typedef struct elemento_ ELEMENTO;
typedef struct grupo_ GRUPO;

struct elemento_ {

    int tipo;

    union {
        ITEM *atomo;
        GRUPO *grupo;
    } info;

};

struct grupo_ {

    char *campo;
    ITEM *representante;

    ELEMENTO *elementos;
    int atual;
    int tamanho;
    int capacidade_atual;

};

/*
Objetivo: Criar uma copia independente de uma string usada como criterio
  de agrupamento, para que o grupo nao dependa da memoria de origem.

Parametros:
- texto: String a copiar; deve estar terminada em '\0'.

Retorno: Ponteiro para a copia alocada ou NULL para entrada nula ou
  falha de memoria. O chamador e responsavel pela liberacao da copia.
*/
char *copiar_string_lista_generalizada(const char *texto){

    if (texto == NULL)
        return NULL;

    char *copia = malloc(strlen(texto) + 1);
    if (copia != NULL)
        strcpy(copia, texto);

    return copia;
}

/*
Objetivo: Alocar e inicializar um grupo da lista generalizada. Guarda
  uma copia do nome do campo, reserva um vetor de ELEMENTO e associa
  um ITEM representante. O grupo RAIZ e uma excecao: nao tem representante.

Parametros:
- campo: Nome nao vazio do campo que organiza o grupo; "RAIZ" identifica
  o grupo inicial e permite representante nulo.
- representante: ITEM que define o valor do grupo naquele campo; pode
  ser NULL somente quando campo e "RAIZ".

Retorno: Ponteiro para o grupo inicializado ou NULL se os argumentos
  forem invalidos ou alguma alocacao falhar.
*/
GRUPO *group_criar(const char *campo, ITEM *representante){ //item pode ser NULL pois a raiz da lista é um grupo, que nao possui nenhum representante//

    if (campo == NULL || *campo == '\0')
        return NULL;

    if (strcmp(campo, "RAIZ") != 0 && representante == NULL)
        return NULL;

    GRUPO *grupo = malloc(sizeof(GRUPO));
    if (grupo == NULL)
        return NULL;

    grupo->campo = NULL;

    if (campo != NULL){

        grupo->campo = copiar_string_lista_generalizada(campo);

        if (grupo->campo == NULL){
            free(grupo);
            return NULL;
        }
    }

    grupo->elementos = malloc(CAPACIDADE_INICIAL * sizeof(ELEMENTO));
    if (grupo->elementos == NULL){
        free(grupo->campo);
        free(grupo);
        return NULL;
    }

    grupo->representante = representante;
    grupo->atual = 0;
    grupo->tamanho = 0;
    grupo->capacidade_atual = CAPACIDADE_INICIAL;

    return grupo;
}

/*
Objetivo: Liberar recursivamente os subgrupos, o vetor de elementos,
  a string do campo e o proprio GRUPO. Os ITEMs guardados nos atomos
  so sao liberados quando apagar_itens for verdadeiro.

Parametros:
- grupo: Grupo a destruir, inclusive seus descendentes.
- apagar_itens: true para destruir tambem os ITEMs dos atomos; false
  para preservar os registros que podem estar compartilhados.

Retorno: Nao se aplica (funcao void).
*/
void group_apagar(GRUPO *grupo, bool apagar_itens){

    if (grupo == NULL)
        return;

    for (int i = 0; i < grupo->tamanho; i++){

        ELEMENTO *elemento = &grupo->elementos[i];

        if (elemento->tipo == TIPO_GRUPO)
            group_apagar(elemento->info.grupo, apagar_itens);
        else if (apagar_itens)
            item_apagar(&elemento->info.atomo);
    }

    free(grupo->elementos);
    free(grupo->campo);
    free(grupo);
}

/*
Objetivo: Dobrar a capacidade do vetor dinamico de ELEMENTO de um grupo
  usando realloc, preservando os elementos ja armazenados.

Parametros:
- grupo: Grupo cujo vetor precisa de mais espaco.

Retorno: true se a realocacao ocorrer; false se grupo for NULL
  ou a alocacao falhar, sem perder o vetor anterior.
*/
bool group_expandir(GRUPO *grupo){

    if (grupo == NULL)
        return false;

    int nova_capacidade = grupo->capacidade_atual * 2;

    ELEMENTO *novo = realloc(grupo->elementos, nova_capacidade * sizeof(ELEMENTO));
    if (novo == NULL)
        return false;

    grupo->elementos = novo;
    grupo->capacidade_atual = nova_capacidade;

    return true;
}

/*
Objetivo: Acrescentar um ITEM como atomo ao final de um grupo folha.
  Impede misturar atomos com subgrupos, expande o vetor quando necessario
  e reinicia o indice de percurso apos a insercao.

Parametros:
- grupo: Grupo folha que recebera o novo atomo.
- item: Registro a guardar por ponteiro, sem copiar seus dados.

Retorno: true quando o atomo for inserido ou false para parametros
  invalidos, grupo de subgrupos ou falha ao expandir o vetor.
*/
bool group_inserir_atomo(GRUPO *grupo, ITEM *item){

    if (grupo == NULL || item == NULL)
        return false;

    if (grupo->tamanho > 0 && grupo->elementos[0].tipo != TIPO_ATOMO)
        return false;

    if (grupo->tamanho >= grupo->capacidade_atual)
        if (!group_expandir(grupo))
            return false;

    grupo->elementos[grupo->tamanho].tipo = TIPO_ATOMO;
    grupo->elementos[grupo->tamanho].info.atomo = item;
    grupo->tamanho++;
    grupo->atual = 0;

    return true;
}


/*
Objetivo: Encontrar por busca binaria a posicao de um subgrupo segundo
  o valor de seu ITEM representante no campo informado. Retorna a
  primeira posicao cujo representante nao seja menor que o item.

Parametros:
- grupo: Grupo pai contendo subgrupos ordenados pelo campo.
- campo: Nome do atributo usado para comparar os representantes.
- item: Registro com o valor de referencia para encontrar a posicao.

Retorno: Indice de insercao/consulta entre 0 e grupo->tamanho ou -1
  para argumentos invalidos, estrutura inesperada ou comparacao falha.
*/
int group_busca_binaria(GRUPO *grupo, const char *campo, ITEM *item){

    if (grupo == NULL || campo == NULL || item == NULL)
        return -1;

    int inicio = 0;
    int fim = grupo->tamanho;

    while (inicio < fim){

        int meio = inicio + (fim - inicio) / 2;
        ELEMENTO *elemento = &grupo->elementos[meio];

        if (elemento->tipo != TIPO_GRUPO || elemento->info.grupo == NULL)
            return -1;

        GRUPO *subgrupo = elemento->info.grupo;

        if (subgrupo->campo == NULL || strcmp(subgrupo->campo, campo) != 0)
            return -1;

        int comparacao = comparar_itens(subgrupo->representante, item, campo);

        if (comparacao == COMPARACAO_FALHOU)
            return -1;

        if (comparacao < 0)
            inicio = meio + 1;
        else
            fim = meio;
    }

    return inicio;
}

/*
Objetivo: Procurar um subgrupo de mesmo valor no campo indicado. Se nao
  existir, criar um novo grupo e inseri-lo na posicao ordenada,
  deslocando os subgrupos seguintes e atualizando o cursor de percurso.

Parametros:
- grupo: Grupo pai no qual procurar ou inserir um subgrupo.
- campo: Campo que caracteriza os subgrupos deste nivel.
- item: ITEM usado como representante do subgrupo, quando criado.
- criado: Ponteiro de saida: recebe true se um novo subgrupo foi criado
  e false se um existente foi reutilizado ou se a operacao falhou.

Retorno: Ponteiro para o subgrupo encontrado/criado, ou NULL caso
  os argumentos sejam invalidos ou alguma operacao falhe.
*/
GRUPO *group_buscar_ou_criar(GRUPO *grupo, const char *campo, ITEM *item, bool *criado){

    if (grupo == NULL || campo == NULL || item == NULL || criado == NULL)
        return NULL;

    *criado = false;

    int posicao = group_busca_binaria(grupo, campo, item);

    if (posicao == -1)
        return NULL;

    if (posicao < grupo->tamanho){

        GRUPO *subgrupo = grupo->elementos[posicao].info.grupo;

        int comparacao = comparar_itens(subgrupo->representante, item, campo);

        if (comparacao == COMPARACAO_FALHOU)
            return NULL;

        if (comparacao == 0)
            return subgrupo;
    }

    if (grupo->tamanho >= grupo->capacidade_atual)
        if (!group_expandir(grupo))
            return NULL;

    GRUPO *subgrupo = group_criar(campo, item);

    if (subgrupo == NULL)
        return NULL;

    for (int i = grupo->tamanho; i > posicao; i--)
        grupo->elementos[i] = grupo->elementos[i - 1];

    grupo->elementos[posicao].tipo = TIPO_GRUPO;
    grupo->elementos[posicao].info.grupo = subgrupo;

    grupo->tamanho++;
    grupo->atual = 0;
    *criado = true;

    return subgrupo;
}

/*
Objetivo: Remover um subgrupo recem-criado que permaneceu vazio apos
  uma falha de insercao. Desloca os elementos seguintes, reinicia
  o cursor e libera o subgrupo sem destruir nenhum ITEM.

Parametros:
- grupo: Grupo pai que contem o subgrupo a remover.
- subgrupo: Subgrupo vazio a procurar e remover.

Retorno: true quando o subgrupo vazio for removido; false se os
  argumentos forem invalidos, ele nao estiver vazio ou nao for encontrado.
*/
bool group_remover_vazio(GRUPO *grupo, GRUPO *subgrupo){ //usado para remover subgrupo vazio caso a inserção falhe//

    if (grupo == NULL || subgrupo == NULL || subgrupo->tamanho != 0)
        return false;

    for (int i = 0; i < grupo->tamanho; i++){

        if (grupo->elementos[i].tipo == TIPO_GRUPO &&
            grupo->elementos[i].info.grupo == subgrupo){

            for (int j = i; j < grupo->tamanho - 1; j++)
                grupo->elementos[j] = grupo->elementos[j + 1];

            grupo->tamanho--;
            grupo->atual = 0;
            group_apagar(subgrupo, false);
            return true;
        }
    }

    return false;
}

/*
Objetivo: Percorrer recursivamente os elementos na ordem armazenada.
  Em grupos folha, retorna os atomos (ITEMs); em grupos intermediarios,
  visita os subgrupos antes de avancar ao proximo elemento.

Parametros:
- grupo: Grupo a percorrer usando seu indice interno atual.

Retorno: Proximo ITEM disponivel ou NULL quando o grupo for invalido
  ou todos os seus elementos e subgrupos ja tiverem sido percorridos.
*/
ITEM *group_percorrer(GRUPO *grupo){

    if (grupo == NULL)
        return NULL;

    while (grupo->atual < grupo->tamanho){

        ELEMENTO *elemento = &grupo->elementos[grupo->atual];

        if (elemento->tipo == TIPO_ATOMO){

            grupo->atual++;

            if (elemento->info.atomo != NULL)
                return elemento->info.atomo;
        }
        else {

            ITEM *item = group_percorrer(elemento->info.grupo);

            if (item != NULL)
                return item;

            grupo->atual++;
        }
    }

    return NULL;
}

/*
Objetivo: Descer pelos niveis de agrupamento, localizando ou criando
  um subgrupo para cada campo. Ao atingir a profundidade solicitada,
  inserir o ITEM como atomo no grupo folha. Em caso de falha, desfaz
  a criacao de um subgrupo que ainda esteja vazio.

Parametros:
- grupo: Grupo do nivel atual da recursao.
- item: Registro a inserir no grupo folha.
- campos: Vetor de campos definido para uma lista de base; usado
  somente quando query for NULL.
- profundidade: Quantidade total de niveis de agrupamento.
- query: Consulta com campos GROUP para listas de saida, ou NULL
  para empregar o vetor campos.
- indice: Nivel atual do percurso recursivo, iniciado em zero.

Retorno: true se o registro foi inserido no ultimo nivel; false
  caso algum campo, subgrupo, comparacao ou alocacao falhe.
*/
bool group_inserir_recursivo(GRUPO *grupo, ITEM *item, const char *const *campos, int profundidade, QUERY *query, int indice){

    if (indice == profundidade)
        return group_inserir_atomo(grupo, item);

    const char *campo;

    if (query == NULL)
        campo = campos[indice];
    else
        campo = query_get_grupo_campo(query, indice);

    if (campo == NULL || *campo == '\0')
        return false;

    bool criado = false;
    GRUPO *subgrupo = group_buscar_ou_criar(grupo, campo, item, &criado);
    if (subgrupo == NULL)
        return false;

    if (group_inserir_recursivo(subgrupo, item, campos, profundidade, query, indice + 1))
        return true;

    if (criado)
        group_remover_vazio(grupo, subgrupo);

    return false;
}

/*
Objetivo: Validar os parametros e iniciar a insercao recursiva de um
  registro na hierarquia, usando os campos da base ou os campos GROUP
  fornecidos pela consulta da saida.

Parametros:
- raiz: Grupo raiz da lista generalizada.
- item: Registro a ser inserido.
- campos: Vetor de campos da lista de base; pode ser NULL se query existir.
- profundidade: Numero de niveis de grupos a percorrer, maior que zero.
- query: Consulta com diretiva GROUP, ou NULL para usar campos.

Retorno: true se a insercao recursiva for concluida; false quando
  a configuracao for inconsistente ou a insercao falhar.
*/
bool group_inserir(GRUPO *raiz, ITEM *item, const char *const *campos, int profundidade, QUERY *query){

    if (raiz == NULL || item == NULL || profundidade <= 0)
        return false;

    if (query == NULL && campos == NULL)
        return false;

    if (query != NULL &&
        (!query_get_possui_grupo(query) ||
         query_get_quantidade_grupos(query) != profundidade))
        return false;

    return group_inserir_recursivo(raiz, item, campos, profundidade, query, 0);
}

struct lista_generalizada_ {

    int tipo_lista;
    int profundidade;

    union {
        char **campos;
        QUERY *query;
    } criterio;

    GRUPO *raiz;

    int tamanho;
    int tamanho_max;

};

/*
Objetivo: Criar uma lista generalizada para registros originais.
  Inicializa uma raiz sentinela e armazena copias independentes dos
  campos que definem cada nivel de agrupamento da base.

Parametros:
- campos: Vetor de nomes nao vazios, um por nivel da hierarquia.
- profundidade: Quantidade de campos/niveis; deve ser positiva.
- tamanho_max: Limite de registros, com zero indicando ausencia
  de limite e valores negativos rejeitados.

Retorno: Ponteiro para a lista criada ou NULL em caso de parametros
  invalidos ou falha na alocacao dos seus componentes.
*/
LISTA_GENERALIZADA *lista_generalizada_criar_base(const char *const *campos, int profundidade, int tamanho_max){

    if (campos == NULL || profundidade <= 0 || tamanho_max < 0)
        return NULL;

    LISTA_GENERALIZADA *lista = malloc(sizeof(LISTA_GENERALIZADA));
    if (lista == NULL)
        return NULL;

    lista->tipo_lista = LISTA_BASE;
    lista->profundidade = profundidade;
    lista->criterio.campos = NULL;
    lista->raiz = group_criar("RAIZ", NULL);

    if (lista->raiz == NULL){
        free(lista);
        return NULL;
    }

    lista->criterio.campos = malloc(profundidade * sizeof(char *));
    if (lista->criterio.campos == NULL){
        lista_generalizada_apagar(&lista, false);
        return NULL;
    }

    for (int i = 0; i < profundidade; i++)
        lista->criterio.campos[i] = NULL;

    for (int i = 0; i < profundidade; i++){

        if (campos[i] == NULL || *campos[i] == '\0'){
            lista_generalizada_apagar(&lista, false);
            return NULL;
        }

        lista->criterio.campos[i] = copiar_string_lista_generalizada(campos[i]);

        if (lista->criterio.campos[i] == NULL){
            lista_generalizada_apagar(&lista, false);
            return NULL;
        }
    }

    lista->tamanho = 0;
    lista->tamanho_max = tamanho_max;

    return lista;
}

/*
Objetivo: Criar uma lista generalizada para os registros de resultado.
  Obtem a profundidade da diretiva GROUP e guarda uma referencia a
  QUERY, sem copiar nem assumir propriedade dessa consulta.

Parametros:
- query: Consulta valida contendo ao menos um campo GROUP.
- tamanho_max: Limite maximo de registros; zero significa sem limite
  fixo, e valores negativos sao rejeitados.

Retorno: Ponteiro para a lista de saida ou NULL para parametros
  invalidos ou falhas de alocacao.
*/
LISTA_GENERALIZADA *lista_generalizada_criar_saida(QUERY *query, int tamanho_max){

    if (query == NULL || !query_get_possui_grupo(query) ||
        query_get_quantidade_grupos(query) <= 0 || tamanho_max < 0)
        return NULL;

    LISTA_GENERALIZADA *lista = malloc(sizeof(LISTA_GENERALIZADA));
    if (lista == NULL)
        return NULL;

    lista->tipo_lista = LISTA_SAIDA;
    lista->profundidade = query_get_quantidade_grupos(query);
    lista->criterio.query = query;
    lista->raiz = group_criar("RAIZ", 0);

    if (lista->raiz == NULL){
        free(lista);
        return NULL;
    }

    lista->tamanho = 0;
    lista->tamanho_max = tamanho_max;

    return lista;
}

/*
Objetivo: Inserir um ITEM na hierarquia de grupos da lista, respeitando
  o limite de tamanho. Usa os campos da base ou a QUERY da saida
  conforme o tipo da lista e incrementa o tamanho so em caso de sucesso.

Parametros:
- lista: Lista generalizada que recebera o registro.
- item: Registro a inserir por ponteiro, sem copia dos dados.

Retorno: true se o ITEM foi inserido ou false para parametros invalidos,
  limite atingido ou falha na insercao dos grupos/atomo.
*/
bool lista_generalizada_inserir(LISTA_GENERALIZADA *lista, ITEM *item){

    if (lista == NULL || item == NULL || (lista->tamanho_max != 0 && lista->tamanho >= lista->tamanho_max))
        return false;

    bool inserido;

    if (lista->tipo_lista == LISTA_BASE)
        inserido = group_inserir(lista->raiz, item, (const char *const *) lista->criterio.campos,
            lista->profundidade, NULL);
    else
        inserido = group_inserir(lista->raiz, item, NULL, lista->profundidade, lista->criterio.query);

    if (inserido)
        lista->tamanho++;

    return inserido;
}

/*
Objetivo: Recuperar o proximo ITEM na travessia recursiva da hierarquia
  de grupos, delegando o percurso ao grupo raiz.

Parametros:
- lista: Lista generalizada cujo cursor de percurso sera avancado.

Retorno: Proximo registro do percurso ou NULL se lista for nula
  ou todos os registros ja tiverem sido visitados.
*/
ITEM *lista_generalizada_percorrer(LISTA_GENERALIZADA *lista){

    if (lista == NULL)
        return NULL;

    return group_percorrer(lista->raiz);
}

/*
Objetivo: Liberar a raiz e todos os subgrupos da lista, inclusive
  seus vetores e campos. Em listas de base, libera tambem as copias
  dos nomes dos campos; em listas de saida, preserva a QUERY referenciada.
  Opcionalmente libera os ITEMs e zera o ponteiro da lista no chamador.

Parametros:
- lista: Endereco do ponteiro da lista a destruir; recebe NULL ao final.
- apagar_itens: true para liberar os ITEMs dos grupos folha, ou false
  para preservar registros compartilhados com outras estruturas.

Retorno: Nao se aplica (funcao void).
*/
void lista_generalizada_apagar(LISTA_GENERALIZADA **lista, bool apagar_itens){

    if (lista == NULL || *lista == NULL)
        return;

    group_apagar((*lista)->raiz, apagar_itens);

    if ((*lista)->tipo_lista == LISTA_BASE){
        if ((*lista)->criterio.campos != NULL){
            for (int i = 0; i < (*lista)->profundidade; i++)
                free((*lista)->criterio.campos[i]);
            free((*lista)->criterio.campos);
        }
    }

    free(*lista);
    *lista = NULL;
}
