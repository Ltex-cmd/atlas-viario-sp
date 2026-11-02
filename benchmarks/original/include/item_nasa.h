/*
Motivacao: Representar os dados dos asteroides como registros.
Funcionamento geral: Cria ITEMs NASA e disponibiliza acesso aos seus atributos.
*/

#ifndef ITEM_NASA_H
#define ITEM_NASA_H

#include <stdbool.h>
#include <stdio.h>
#include "item.h"

#define TIPO_NASA 1

ITEM *item_criar_nasa(char *asteroid_designation, char *asteroid_fullname, char *close_approach_date, int year,
                      int month, double distance_au, double velocity_km_s, double absolute_magnitude, int days_until_approach,
                      bool is_past_event, bool is_future_event, double risk_score, int panic_level, char *threat_category,
                      char *panic_verdict, bool on_sentry_list, double sentry_impact_prob, double sentry_torino_scale,
                      double sentry_palermo_scale, double sentry_diameter_km, char *linha_original);

void item_apagar_nasa(void **dado);

const char *item_nasa_get_asteroid_designation(ITEM *item);
const char *item_nasa_get_asteroid_fullname(ITEM *item);
const char *item_nasa_get_close_approach_date(ITEM *item);

int item_nasa_get_year(ITEM *item);
int item_nasa_get_month(ITEM *item);

double item_nasa_get_distance_au(ITEM *item);
double item_nasa_get_velocity_km_s(ITEM *item);
double item_nasa_get_absolute_magnitude(ITEM *item);

int item_nasa_get_days_until_approach(ITEM *item);

bool item_nasa_get_is_past_event(ITEM *item);
bool item_nasa_get_is_future_event(ITEM *item);

double item_nasa_get_risk_score(ITEM *item);

int item_nasa_get_panic_level(ITEM *item);

const char *item_nasa_get_threat_category(ITEM *item);
const char *item_nasa_get_panic_verdict(ITEM *item);

bool item_nasa_get_on_sentry_list(ITEM *item);

double item_nasa_get_sentry_impact_prob(ITEM *item);
double item_nasa_get_sentry_torino_scale(ITEM *item);
double item_nasa_get_sentry_palermo_scale(ITEM *item);
double item_nasa_get_sentry_diameter_km(ITEM *item);

bool obter_valor_numerico_nasa(ITEM *item, const char *campo, double *valor);

#endif