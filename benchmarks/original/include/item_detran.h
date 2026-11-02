/*
Motivacao: Representar os dados de sinistros de transito como registros.
Funcionamento geral: Cria ITEMs DETRAN e disponibiliza acesso aos seus atributos.
*/

#ifndef ITEM_DETRAN_H
#define ITEM_DETRAN_H

#include <stdbool.h>
#include <stdio.h>

#include "item.h"

#define TIPO_DETRAN 2


ITEM *item_criar_detran(int id_sinistro, char *tipo_registro, char *data_sinistro, int ano_sinistro,
    int mes_sinistro, int dia_sinistro, char *hora_sinistro, char *ano_mes_sinistro, char *dia_da_semana,
    char *turno, char *logradouro, double numero_logradouro, char *tipo_via, char *tipo_local, double latitude,
    double longitude, int cod_ibge, char *municipio, char *regiao_administrativa, char *administracao, char *conservacao,
    char *circunscricao, char *tp_sinistro_primario, int qtd_pedestre, int qtd_bicicleta, int qtd_motocicleta, int qtd_automovel,
    int qtd_onibus, int qtd_caminhao, int qtd_veic_outros, int qtd_veic_nao_disponivel, int qtd_gravidade_fatal,int qtd_gravidade_grave,
    int qtd_gravidade_leve, int qtd_gravidade_ileso, int qtd_gravidade_nao_disponivel, bool tp_sinistro_atrop_pedestre,
    bool tp_sinistro_atrop_vitima_fora_veic, bool tp_sinistro_colisao_frontal, bool tp_sinistro_colisao_traseira,
    bool tp_sinistro_colisao_lateral, bool tp_sinistro_colisao_transversal, bool tp_sinistro_colisao_outros,
    bool tp_sinistro_choque, bool tp_sinistro_atrop_animal, bool tp_sinistro_capotamento, bool tp_sinistro_engavetamento,
    bool tp_sinistro_tombamento, bool tp_sinistro_outros, bool tp_sinistro_nao_disponivel,char *linha_original);
    
void item_apagar_detran(void **dado);

int item_detran_get_id_sinistro(ITEM *item);
const char *item_detran_get_tipo_registro(ITEM *item);
const char *item_detran_get_data_sinistro(ITEM *item);
int item_detran_get_ano_sinistro(ITEM *item);
int item_detran_get_mes_sinistro(ITEM *item);
int item_detran_get_dia_sinistro(ITEM *item);
const char *item_detran_get_hora_sinistro(ITEM *item);
const char *item_detran_get_ano_mes_sinistro(ITEM *item);
const char *item_detran_get_dia_da_semana(ITEM *item);
const char *item_detran_get_turno(ITEM *item);

const char *item_detran_get_logradouro(ITEM *item);
double item_detran_get_numero_logradouro(ITEM *item);
const char *item_detran_get_tipo_via(ITEM *item);
const char *item_detran_get_tipo_local(ITEM *item);
double item_detran_get_latitude(ITEM *item);
double item_detran_get_longitude(ITEM *item);
int item_detran_get_cod_ibge(ITEM *item);
const char *item_detran_get_municipio(ITEM *item);
const char *item_detran_get_regiao_administrativa(ITEM *item);
const char *item_detran_get_administracao(ITEM *item);
const char *item_detran_get_conservacao(ITEM *item);
const char *item_detran_get_circunscricao(ITEM *item);

const char *item_detran_get_tp_sinistro_primario(ITEM *item);

int item_detran_get_qtd_pedestre(ITEM *item);
int item_detran_get_qtd_bicicleta(ITEM *item);
int item_detran_get_qtd_motocicleta(ITEM *item);
int item_detran_get_qtd_automovel(ITEM *item);
int item_detran_get_qtd_onibus(ITEM *item);
int item_detran_get_qtd_caminhao(ITEM *item);
int item_detran_get_qtd_veic_outros(ITEM *item);
int item_detran_get_qtd_veic_nao_disponivel(ITEM *item);
int item_detran_get_qtd_gravidade_fatal(ITEM *item);
int item_detran_get_qtd_gravidade_grave(ITEM *item);
int item_detran_get_qtd_gravidade_leve(ITEM *item);
int item_detran_get_qtd_gravidade_ileso(ITEM *item);
int item_detran_get_qtd_gravidade_nao_disponivel(ITEM *item);

bool item_detran_get_tp_sinistro_atrop_pedestre(ITEM *item);
bool item_detran_get_tp_sinistro_atrop_vitima_fora_veic(ITEM *item);
bool item_detran_get_tp_sinistro_colisao_frontal(ITEM *item);
bool item_detran_get_tp_sinistro_colisao_traseira(ITEM *item);
bool item_detran_get_tp_sinistro_colisao_lateral(ITEM *item);
bool item_detran_get_tp_sinistro_colisao_transversal(ITEM *item);
bool item_detran_get_tp_sinistro_colisao_outros(ITEM *item);
bool item_detran_get_tp_sinistro_choque(ITEM *item);
bool item_detran_get_tp_sinistro_atrop_animal(ITEM *item);
bool item_detran_get_tp_sinistro_capotamento(ITEM *item);
bool item_detran_get_tp_sinistro_engavetamento(ITEM *item);
bool item_detran_get_tp_sinistro_tombamento(ITEM *item);
bool item_detran_get_tp_sinistro_outros(ITEM *item);
bool item_detran_get_tp_sinistro_nao_disponivel(ITEM *item);

bool obter_valor_numerico_detran(ITEM *item, const char *campo, double *valor);

#endif
