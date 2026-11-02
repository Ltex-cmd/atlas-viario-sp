#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "comparador_nasa.h"
#include "item.h"
#include "item_nasa.h"

/* Os resultados validos sao -1, 0 e 1. */

/*
Objetivo: Atribuir uma prioridade numerica as categorias de ameaca da
base NASA. A ordem definida e SAFE < MONITOR < CONCERN < OH_NO,
permitindo compara-las sem depender da ordem alfabetica.

Parametros:
- valor: Categoria textual que sera convertida para sua posicao na ordem.

Retorno: Valor de 0 a 3 para categorias reconhecidas ou
COMPARACAO_FALHOU (-2) para entrada nula ou desconhecida.
*/
int ordem_threat_category(const char *valor){

    if (valor == NULL)
        return COMPARACAO_FALHOU;

    if (strcmp(valor, "SAFE") == 0) return 0;
    if (strcmp(valor, "MONITOR") == 0) return 1;
    if (strcmp(valor, "CONCERN") == 0) return 2;
    if (strcmp(valor, "OH_NO") == 0) return 3;

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Atribuir uma prioridade numerica aos textos do veredito de panico,
seguindo a ordem de gravidade definida para os registros NASA.

Parametros:
- valor: Texto do veredito a ser localizado na ordem predefinida.

Retorno: Valor de 0 a 3 quando o texto e reconhecido ou
COMPARACAO_FALHOU (-2) para valor nulo ou nao previsto.
*/
int ordem_panic_verdict(const char *valor){

    if (valor == NULL)
        return COMPARACAO_FALHOU;

    if (strcmp(valor, "You're fine. Go touch grass.") == 0) return 0;
    if (strcmp(valor, "\"Worth a tweet, not worth a bunker.\"") == 0) return 1;
    if (strcmp(valor, "Time to learn survival skills?") == 0) return 2;
    if (strcmp(valor, "Did you backup your data?") == 0) return 3;

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Interpretar os textos booleanos utilizados nas consultas NASA.
Reconhece apenas "true", "True", "false" e "False".

Parametros:
- texto: String a ser interpretada como valor logico.
- resultado: Ponteiro de saida que recebe true ou false quando o texto
  e valido; nao e modificado em caso de falha.

Retorno: true se houve conversao valida ou false se a entrada e invalida.
*/
bool converter_bool_nasa(const char *texto, bool *resultado){

    if (texto == NULL || resultado == NULL)
        return false;

    if (strcmp(texto, "true") == 0 || strcmp(texto, "True") == 0)
        *resultado = true;

    else if (strcmp(texto, "false") == 0 || strcmp(texto, "False") == 0)
        *resultado = false;

    else
        return false;

    return true;
}

/*
Objetivo: Comparar o mesmo campo entre dois registros de asteroides NASA.
Utiliza strcmp nos textos e datas, comparacao numerica para int e double,
ordens explicitas para categorias e comparacao logica para indicadores.
Campos Sentry exigem que ambos os registros estejam na lista Sentry.

Parametros:
- item1: Primeiro registro NASA a ser comparado.
- item2: Segundo registro NASA a ser comparado.
- campo: Nome da coluna cujos valores determinam a comparacao.

Retorno: -1 quando o primeiro valor e menor, 0 quando sao iguais, 1 quando
e maior ou COMPARACAO_FALHOU (-2) para campo ou dados invalidos.
*/
int comparar_itens_nasa(ITEM *item1, ITEM *item2, const char *campo){

    if (item1 == NULL || item2 == NULL || campo == NULL ||
        item_get_tipo(item1) != TIPO_NASA || item_get_tipo(item2) != TIPO_NASA)
        return COMPARACAO_FALHOU;

    if (strcmp(campo, "asteroid_designation") == 0){

        const char *valor1 = item_nasa_get_asteroid_designation(item1);
        const char *valor2 = item_nasa_get_asteroid_designation(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "asteroid_fullname") == 0){

        const char *valor1 = item_nasa_get_asteroid_fullname(item1);
        const char *valor2 = item_nasa_get_asteroid_fullname(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "close_approach_date") == 0){

        const char *valor1 = item_nasa_get_close_approach_date(item1);
        const char *valor2 = item_nasa_get_close_approach_date(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "year") == 0){

        int valor1 = item_nasa_get_year(item1);
        int valor2 = item_nasa_get_year(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "month") == 0){

        int valor1 = item_nasa_get_month(item1);
        int valor2 = item_nasa_get_month(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "distance_au") == 0){

        double valor1 = item_nasa_get_distance_au(item1);
        double valor2 = item_nasa_get_distance_au(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "velocity_km_s") == 0){

        double valor1 = item_nasa_get_velocity_km_s(item1);
        double valor2 = item_nasa_get_velocity_km_s(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "absolute_magnitude") == 0){

        double valor1 = item_nasa_get_absolute_magnitude(item1);
        double valor2 = item_nasa_get_absolute_magnitude(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "days_until_approach") == 0){

        int valor1 = item_nasa_get_days_until_approach(item1);
        int valor2 = item_nasa_get_days_until_approach(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "is_past_event") == 0){

        bool valor1 = item_nasa_get_is_past_event(item1);
        bool valor2 = item_nasa_get_is_past_event(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "is_future_event") == 0){

        bool valor1 = item_nasa_get_is_future_event(item1);
        bool valor2 = item_nasa_get_is_future_event(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "risk_score") == 0){

        double valor1 = item_nasa_get_risk_score(item1);
        double valor2 = item_nasa_get_risk_score(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "panic_level") == 0){

        int valor1 = item_nasa_get_panic_level(item1);
        int valor2 = item_nasa_get_panic_level(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "threat_category") == 0){

        const char *valor1 = item_nasa_get_threat_category(item1);
        const char *valor2 = item_nasa_get_threat_category(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int ordem1 = ordem_threat_category(valor1);
        int ordem2 = ordem_threat_category(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "panic_verdict") == 0){

        const char *valor1 = item_nasa_get_panic_verdict(item1);
        const char *valor2 = item_nasa_get_panic_verdict(item2);

        if (valor1 == NULL || valor2 == NULL)
            return COMPARACAO_FALHOU;

        int ordem1 = ordem_panic_verdict(valor1);
        int ordem2 = ordem_panic_verdict(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "on_sentry_list") == 0){

        bool valor1 = item_nasa_get_on_sentry_list(item1);
        bool valor2 = item_nasa_get_on_sentry_list(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_impact_prob") == 0){

        if (!item_nasa_get_on_sentry_list(item1) || !item_nasa_get_on_sentry_list(item2))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_impact_prob(item1);
        double valor2 = item_nasa_get_sentry_impact_prob(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_torino_scale") == 0){

        if (!item_nasa_get_on_sentry_list(item1) || !item_nasa_get_on_sentry_list(item2))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_torino_scale(item1);
        double valor2 = item_nasa_get_sentry_torino_scale(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_palermo_scale") == 0){

        if (!item_nasa_get_on_sentry_list(item1) || !item_nasa_get_on_sentry_list(item2))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_palermo_scale(item1);
        double valor2 = item_nasa_get_sentry_palermo_scale(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_diameter_km") == 0){

        if (!item_nasa_get_on_sentry_list(item1) || !item_nasa_get_on_sentry_list(item2))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_diameter_km(item1);
        double valor2 = item_nasa_get_sentry_diameter_km(item2);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    return COMPARACAO_FALHOU;
}

/*
Objetivo: Comparar o campo de um registro NASA com um valor recebido como
texto, como ocorre em consultas e filtros. Converte o texto conforme o tipo
da coluna e respeita as ordens das categorias e as regras dos campos Sentry.

Parametros:
- item: Registro NASA que contem o campo consultado.
- campo: Nome da coluna do item que sera comparada.
- valor: Texto do valor de referencia, convertido quando necessario.

Retorno: -1 se o campo do item e menor que o valor, 0 se igual, 1 se maior
ou COMPARACAO_FALHOU (-2) se nao for possivel comparar.
Observacao: atoi e atof nao distinguem texto numerico invalido de zero.
*/
int comparar_item_nasa_valor(ITEM *item, const char *campo, const char *valor){

    if (item == NULL || campo == NULL || valor == NULL ||
        item_get_tipo(item) != TIPO_NASA)
        return COMPARACAO_FALHOU;

    if (strcmp(campo, "asteroid_designation") == 0){

        const char *valor1 = item_nasa_get_asteroid_designation(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "asteroid_fullname") == 0){

        const char *valor1 = item_nasa_get_asteroid_fullname(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "close_approach_date") == 0){

        const char *valor1 = item_nasa_get_close_approach_date(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;

        const char *valor2 = valor;

        int comparacao = strcmp(valor1, valor2);
        if (comparacao > 0) return 1;
        if (comparacao == 0) return 0;

        return -1;
    }

    if (strcmp(campo, "year") == 0){

        int valor1 = item_nasa_get_year(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "month") == 0){

        int valor1 = item_nasa_get_month(item);
        int valor2 = atoi(valor);
    
        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "distance_au") == 0){

        double valor1 = item_nasa_get_distance_au(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "velocity_km_s") == 0){

        double valor1 = item_nasa_get_velocity_km_s(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "absolute_magnitude") == 0){

        double valor1 = item_nasa_get_absolute_magnitude(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "days_until_approach") == 0){

        int valor1 = item_nasa_get_days_until_approach(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "is_past_event") == 0){

        bool valor2;
        if (!converter_bool_nasa(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_nasa_get_is_past_event(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "is_future_event") == 0){

        bool valor2;
        if (!converter_bool_nasa(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_nasa_get_is_future_event(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "risk_score") == 0){

        double valor1 = item_nasa_get_risk_score(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "panic_level") == 0){

        int valor1 = item_nasa_get_panic_level(item);
        int valor2 = atoi(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "threat_category") == 0){

        const char *valor1 = item_nasa_get_threat_category(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;
        
        const char *valor2 = valor;

        int ordem1 = ordem_threat_category(valor1);
        int ordem2 = ordem_threat_category(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "panic_verdict") == 0){

        const char *valor1 = item_nasa_get_panic_verdict(item);
        if (valor1 == NULL)
            return COMPARACAO_FALHOU;
        const char *valor2 = valor;

        int ordem1 = ordem_panic_verdict(valor1);
        int ordem2 = ordem_panic_verdict(valor2);

        if (ordem1 == COMPARACAO_FALHOU || ordem2 == COMPARACAO_FALHOU)
            return COMPARACAO_FALHOU;

        if (ordem1 > ordem2) return 1;
        if (ordem1 == ordem2) return 0;

        return -1;
    }

    if (strcmp(campo, "on_sentry_list") == 0){
        bool valor2;
        if (!converter_bool_nasa(valor, &valor2))
            return COMPARACAO_FALHOU;
        bool valor1 = item_nasa_get_on_sentry_list(item);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_impact_prob") == 0){

        if (!item_nasa_get_on_sentry_list(item))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_impact_prob(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_torino_scale") == 0){

        if (!item_nasa_get_on_sentry_list(item))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_torino_scale(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_palermo_scale") == 0){

        if (!item_nasa_get_on_sentry_list(item))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_palermo_scale(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    if (strcmp(campo, "sentry_diameter_km") == 0){

        if (!item_nasa_get_on_sentry_list(item))
            return COMPARACAO_FALHOU;

        double valor1 = item_nasa_get_sentry_diameter_km(item);
        double valor2 = atof(valor);

        if (valor1 > valor2) return 1;
        if (valor1 == valor2) return 0;

        return -1;
    }

    return COMPARACAO_FALHOU;
}