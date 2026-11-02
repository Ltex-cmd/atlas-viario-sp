/*
Motivacao: Centralizar os criterios definidos no arquivo de consulta.
Funcionamento geral: Le a consulta e disponibiliza parametros de suas diretivas.
*/
#ifndef QUERY_H
#define QUERY_H

#include <stdio.h>
#include <stdbool.h>

typedef struct query_ QUERY;

QUERY *query_criar();
void query_apagar(QUERY **query);
bool ler_query(FILE *arquivo, QUERY **query);

bool query_get_possui_filtros(QUERY *query);
int query_get_quantidade_filtros(QUERY *query);
const char *query_get_filtro_campo(QUERY *query, int indice);
const char *query_get_filtro_operador(QUERY *query, int indice);
const char *query_get_filtro_valor(QUERY *query, int indice);

bool query_get_possui_similaridade(QUERY *query);
int query_get_quantidade_similaridades(QUERY *query);
const char *query_get_similaridade_campo(QUERY *query, int indice);
double query_get_similaridade_alvo(QUERY *query, int indice);
double query_get_similaridade_tolerancia(QUERY *query, int indice);
double query_get_similaridade_peso(QUERY *query, int indice);

bool query_get_possui_accept(QUERY *query);
double query_get_accept(QUERY *query);

bool query_get_possui_ordenacao(QUERY *query);
const char *query_get_ordenacao_criterio(QUERY *query);
const char *query_get_ordenacao_direcao(QUERY *query);

bool query_get_possui_grupo(QUERY *query);
int query_get_quantidade_grupos(QUERY *query);
const char *query_get_grupo_campo(QUERY *query, int indice);

bool query_get_possui_cross(QUERY *query);
int query_get_dimensao_cross(QUERY *query);
const char *query_get_cross_campo(QUERY *query, int indice);

#endif
