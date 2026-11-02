#define _POSIX_C_SOURCE 200809L
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif
#include "engine.h"
#include "estrutura.h"
#include "reader_detran.h"
#include "item_detran.h"
#include "processamento_query.h"
#include "alloc.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define N 1000
#define SCENARIOS 4
#define METHODS 13
#define RUNS 21
static ITEM *items[N];
static Record *records;
static uint32_t expected_ids[SCENARIOS][N], expected_n[SCENARIOS];
static Stats expected[SCENARIOS];
static const char *kinds[] = {"pilha", "fila", "lista_seq", "lista_enc", "lista_cabeca", "lista_ord", "lista_gen", "lista_cruz"};
static const char *type_names[] = {"COLISAO", "ATROPELAMENTO", "CHOQUE", "OUTROS", "NAO DISPONIVEL"};
static const char *road_names[] = {"VIAS URBANAS", "ESTRADAS E RODOVIAS", "NAO DISPONIVEL"};
static const char *registration_names[] = {"SINISTRO NAO FATAL", "SINISTRO FATAL", "NOTIFICACAO"};
typedef struct { uint32_t code; uint16_t ordinal; } City;
static City cities[N];
static int ncities;
static QUERY *queries[SCENARIOS];
static Filter filters[SCENARIOS];
static double build_times[METHODS][SCENARIOS][RUNS];
static double query_times[METHODS][SCENARIOS][RUNS];
static double destroy_times[METHODS][SCENARIOS][RUNS];
static uint32_t examined[METHODS][SCENARIOS];
static size_t retained[METHODS], peak[METHODS];
static volatile uint64_t sink;
static uint32_t observed_ids[N], observed_n;

static void require(int condition, const char *message) {
    if (!condition) { fprintf(stderr, "Benchmark: %s\n", message); exit(2); }
}
void bench_visit_id(uint32_t id) {
    require(observed_n < N, "duplicate or endless site traversal");
    observed_ids[observed_n++] = id;
}
static uint64_t now_ns(void) {
    struct timespec value;
    require(clock_gettime(CLOCK_MONOTONIC_RAW, &value) == 0, "clock failure");
    return (uint64_t)value.tv_sec * UINT64_C(1000000000) + value.tv_nsec;
}
static int cmp_u32(const void *a, const void *b) {
    uint32_t x = *(const uint32_t *)a, y = *(const uint32_t *)b;
    return (x > y) - (x < y);
}
static int cmp_city(const void *a, const void *b) {
    const City *x = a, *y = b;
    return (x->code > y->code) - (x->code < y->code);
}
static double median(const double *input, int n) {
    double sorted[RUNS];
    memcpy(sorted, input, n * sizeof(double));
    for (int i = 1; i < n; i++) {
        double value = sorted[i]; int j = i;
        while (j > 0 && sorted[j - 1] > value) { sorted[j] = sorted[j - 1]; j--; }
        sorted[j] = value;
    }
    return sorted[n / 2];
}
static int category(const char *name, const char *const *values, int count) {
    for (int i = 0; i < count; i++) if (strcmp(name, values[i]) == 0) return i;
    require(0, "unrecognized category"); return -1;
}
static uint16_t city_ordinal(uint32_t code) {
    int lo = 0, hi = ncities;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (cities[mid].code < code) lo = mid + 1; else hi = mid;
    }
    if (lo < ncities && cities[lo].code == code) return cities[lo].ordinal;
    require(0, "unknown city"); return 0;
}
/* This adapter reads the original ITEM through its public getters. Its work is
   included in original query times. It does not replace the original FILTER. */
static void aggregate_original(Stats *stats, ITEM *item) {
    uint32_t id = item_detran_get_id_sinistro(item);
    uint32_t month = item_detran_get_mes_sinistro(item);
    uint32_t date = item_detran_get_ano_sinistro(item) * 10000 + month * 100 + item_detran_get_dia_sinistro(item);
    uint32_t fatal = item_detran_get_qtd_gravidade_fatal(item);
    double lat = item_detran_get_latitude(item), lon = item_detran_get_longitude(item);
    stats->total++;
    stats->located += isfinite(lat) && isfinite(lon) && lat >= -26 && lat <= -19 && lon >= -54 && lon <= -43;
    stats->fingerprint += (uint64_t)id * UINT64_C(11400714819323198485) + date;
    stats->fatal_events += fatal > 0; stats->fatal += fatal;
    stats->grave += item_detran_get_qtd_gravidade_grave(item);
    stats->leve += item_detran_get_qtd_gravidade_leve(item);
    stats->ileso += item_detran_get_qtd_gravidade_ileso(item);
    stats->unknown += item_detran_get_qtd_gravidade_nao_disponivel(item);
    if (month >= 1 && month <= 12) stats->months[month - 1]++;
    stats->types[category(item_detran_get_tp_sinistro_primario(item), type_names, 5)]++;
    stats->cities[city_ordinal(item_detran_get_cod_ibge(item))]++;
}
static void same_stats(const Stats *a, const Stats *b) {
    require(a->total == b->total && a->located == b->located && a->fingerprint == b->fingerprint &&
        a->fatal_events == b->fatal_events && a->fatal == b->fatal && a->grave == b->grave &&
        a->leve == b->leve && a->ileso == b->ileso && a->unknown == b->unknown &&
        memcmp(a->months, b->months, sizeof a->months) == 0 &&
        memcmp(a->types, b->types, sizeof a->types) == 0 &&
        memcmp(a->cities, b->cities, sizeof a->cities) == 0, "different aggregates");
}
static void verify_ids(uint32_t *ids, uint32_t count, int scenario) {
    require(count == expected_n[scenario], "different result count");
    qsort(ids, count, sizeof(uint32_t), cmp_u32);
    require(memcmp(ids, expected_ids[scenario], count * sizeof(uint32_t)) == 0, "different exact ID set");
}
static QUERY *parse_query(const char *text) {
    FILE *file = tmpfile(); require(file != NULL, "query temporary file");
    require(fputs(text, file) >= 0, "write query"); rewind(file);
    QUERY *query = query_criar(); require(query != NULL, "allocate query");
    require(ler_query(file, &query), "parse original query"); fclose(file); return query;
}
static void prepare_queries(void) {
    for (int s = 0; s < SCENARIOS; s++) filters[s] = (Filter){.type=-1, .road=-1, .transport=-1, .registration=-1, .west=-54, .south=-26, .east=-43, .north=-19};
    queries[0] = parse_query("");
    queries[1] = parse_query("FILTER|mes_sinistro|>=|4\nFILTER|mes_sinistro|<=|6\n");
    filters[1].start = 20260401; filters[1].end = 20260630;
    queries[2] = parse_query("FILTER|cod_ibge|=|3548906\n");
    filters[2].city = 3548906;
    queries[3] = parse_query("FILTER|longitude|>=|-48.15\nFILTER|longitude|<=|-47.65\nFILTER|latitude|>=|-22.25\nFILTER|latitude|<=|-21.75\n");
    filters[3].spatial = 1; filters[3].west = -48.15; filters[3].east = -47.65;
    filters[3].south = -22.25; filters[3].north = -21.75;
}
static ESTRUTURA *build_original(int method) {
    const char *ordered[] = {"id_sinistro"};
    const char *grouped[] = {"ano_sinistro", "mes_sinistro", "cod_ibge"};
    const char *crossed[] = {"cod_ibge", "id_sinistro"};
    const char *const *fields = method == 5 ? ordered : method == 6 ? grouped : method == 7 ? crossed : NULL;
    int dimensions = method == 5 ? 1 : method == 6 ? 3 : method == 7 ? 2 : 0;
    ESTRUTURA *structure = estrutura_criar_base(kinds[method], fields, dimensions, N);
    require(structure != NULL, "create original structure");
    for (int i = 0; i < N; i++) require(estrutura_inserir(structure, items[i]), "insert original item");
    return structure;
}
static void run_original(int method, int scenario, int repetition) {
    size_t before = bench_live_bytes(); bench_reset_peak();
    uint64_t start = now_ns();
    ESTRUTURA *structure = build_original(method);
    uint64_t built = now_ns();
    size_t structure_bytes = bench_live_bytes() - before;
    size_t structure_peak = bench_peak_bytes() - before;
    Stats stats;
#ifdef ATLAS_MEMORY_RUN
    uint32_t ids[N], nids = 0;
#endif
    ITEM *item;
    uint64_t query_start = now_ns();
    memset(&stats, 0, sizeof stats);
    while ((item = estrutura_percorrer(structure, "cod_ibge")) != NULL) {
        stats.examined++;
        if (!item_satisfaz_filter(item, queries[scenario])) continue;
#ifdef ATLAS_MEMORY_RUN
        require(nids < N, "duplicate or endless traversal");
        ids[nids++] = item_detran_get_id_sinistro(item);
#endif
        aggregate_original(&stats, item);
    }
    sink = stats.fingerprint;
    uint64_t queried = now_ns();
    require(stats.examined == N, "incomplete original traversal");
    same_stats(&stats, &expected[scenario]);
#ifdef ATLAS_MEMORY_RUN
    verify_ids(ids, nids, scenario);
#endif
    uint64_t destroy_start = now_ns(); estrutura_apagar(&structure, false); uint64_t destroyed = now_ns();
    require(structure == NULL && bench_live_bytes() == before, "original structure leaked memory");
    if (repetition >= 0) {
        build_times[method][scenario][repetition] = (built - start) / 1e6;
        query_times[method][scenario][repetition] = (queried - query_start) / 1e6;
        destroy_times[method][scenario][repetition] = (destroyed - destroy_start) / 1e6;
        examined[method][scenario] = stats.examined;
        retained[method] = structure_bytes; peak[method] = structure_peak;
    }
}
static void run_site(int repetition) {
    size_t before = bench_live_bytes(); bench_reset_peak();
    uint64_t start = now_ns(); Engine *engine = engine_create(records, N); uint64_t built = now_ns();
    require(engine != NULL, "create compact engine");
    size_t bytes = bench_live_bytes() - before, high = bench_peak_bytes() - before;
    for (int s = 0; s < SCENARIOS; s++) for (int step = 0; step < 5; step++) {
        int method = (step + (repetition < 0 ? 0 : repetition)) % 5;
        Filter filter = filters[s]; filter.method = method;
        observed_n = 0;
        Stats stats; uint64_t query_start = now_ns();
        engine_query(engine, &filter, &stats, NULL, 1); sink = stats.fingerprint;
        uint64_t queried = now_ns();
        same_stats(&stats, &expected[s]);
#ifdef ATLAS_MEMORY_RUN
        verify_ids(observed_ids, observed_n, s);
#endif
        Record selected[N]; uint32_t total = 0, ids[N];
        uint32_t count = engine_select(engine, &filter, 0, N, selected, &total);
        require(total == count, "incomplete site selection");
        for (uint32_t k = 0; k < count; k++) ids[k] = selected[k].id;
        verify_ids(ids, count, s);
        if (repetition >= 0) {
            int row = method + 8;
            build_times[row][s][repetition] = (built - start) / 1e6;
            query_times[row][s][repetition] = (queried - query_start) / 1e6;
            examined[row][s] = stats.examined;
            retained[row] = bytes; peak[row] = high;
        }
    }
    uint64_t destroy_start = now_ns(); engine_destroy(engine); uint64_t destroyed = now_ns();
    require(bench_live_bytes() == before, "compact engine leaked memory");
    if (repetition >= 0) for (int row = 8; row < METHODS; row++) for (int s = 0; s < SCENARIOS; s++)
        destroy_times[row][s][repetition] = (destroyed - destroy_start) / 1e6;
}
int main(int argc, char **argv) {
    require(argc == 3, "usage: bench_{timing,memory} sample.csv sample.bin");
    records = malloc(N * sizeof(Record)); require(records != NULL, "allocate compact records");
    FILE *binary = fopen(argv[2], "rb"); require(binary != NULL, "open compact sample");
    require(fread(records, sizeof(Record), N, binary) == N && fgetc(binary) == EOF, "exactly 1000 compact records required"); fclose(binary);
    for (int i = 0; i < N; i++) {
        require(records[i].date / 10000 == 2026, "sample year");
        int city = 0; while (city < ncities && cities[city].code != records[i].ibge) city++;
        if (city == ncities) { cities[city].code = records[i].ibge; cities[city].ordinal = records[i].city; ncities++; }
        require(cities[city].ordinal == records[i].city, "city mapping");
    }
    qsort(cities, ncities, sizeof(City), cmp_city);
    size_t before = bench_live_bytes();
    FILE *csv = fopen(argv[1], "rb"); require(csv != NULL, "open original sample");
    char header[8192]; require(fgets(header, sizeof header, csv) != NULL, "CSV header");
    for (int i = 0; i < N; i++) {
        items[i] = reader_ler_item_detran(csv); require(items[i] != NULL, "read original sample");
        Record *r = &records[i]; ITEM *item = items[i];
        require((uint32_t)item_detran_get_id_sinistro(item) == r->id &&
            (uint32_t)item_detran_get_cod_ibge(item) == r->ibge &&
            item_detran_get_ano_sinistro(item) * 10000u + item_detran_get_mes_sinistro(item) * 100u + item_detran_get_dia_sinistro(item) == r->date &&
            item_detran_get_latitude(item) == r->lat && item_detran_get_longitude(item) == r->lon &&
            item_detran_get_qtd_gravidade_fatal(item) == r->fatal && item_detran_get_qtd_gravidade_grave(item) == r->grave &&
            item_detran_get_qtd_gravidade_leve(item) == r->leve && item_detran_get_qtd_gravidade_ileso(item) == r->ileso &&
            item_detran_get_qtd_gravidade_nao_disponivel(item) == r->unknown &&
            category(item_detran_get_tp_sinistro_primario(item), type_names, 5) == r->type &&
            category(item_detran_get_tipo_via(item), road_names, 3) == r->road &&
            category(item_detran_get_tipo_registro(item), registration_names, 3) == r->registration, "CSV and compact record differ");
    }
    require(reader_ler_item_detran(csv) == NULL && feof(csv), "exactly 1000 CSV records required"); fclose(csv);
    size_t record_bytes = bench_live_bytes() - before;
    prepare_queries();
    Engine *reference = engine_create(records, N); require(reference != NULL, "reference engine");
    for (int s = 0; s < SCENARIOS; s++) {
        observed_n = 0;
        engine_query(reference, &filters[s], &expected[s], NULL, 1);
        Record selected[N]; uint32_t total;
        expected_n[s] = engine_select(reference, &filters[s], 0, N, selected, &total);
        require(total == expected_n[s], "reference selection");
        for (uint32_t k = 0; k < total; k++) expected_ids[s][k] = selected[k].id;
        qsort(expected_ids[s], total, sizeof(uint32_t), cmp_u32);
    }
    engine_destroy(reference);
    int runs = RUNS;
    const char *exact = "null";
#ifdef ATLAS_MEMORY_RUN
    runs = 1;
    exact = "true";
#endif
    /* Fresh instances avoid relying on a reset operation absent from some TADs.
       One warmup is excluded. The first method rotates between repetitions. */
    for (int repetition = -1; repetition < runs; repetition++) {
        if (repetition % 2) run_site(repetition);
        for (int step = 0; step < 8; step++) {
            int method = (step + (repetition < 0 ? 0 : repetition)) % 8;
            for (int s = 0; s < SCENARIOS; s++) run_original(method, s, repetition);
        }
        if (!(repetition % 2)) run_site(repetition);
    }
    printf("{\"records\":%d,\"repetitions\":%d,\"original_record_heap_bytes\":%zu,\"scenarios\":[", N, runs, record_bytes);
    for (int s = 0; s < SCENARIOS; s++) {
        printf("%s{\"count\":%u,\"checksum\":\"%" PRIu64 "\",\"sorted_ids\":[", s ? "," : "", expected_n[s], expected[s].fingerprint);
        for (uint32_t k = 0; k < expected_n[s]; k++) printf("%s%u", k ? "," : "", expected_ids[s][k]);
        printf("],\"methods\":[");
        for (int row = 0; row < METHODS; row++) {
            printf("%s{\"index\":%d,\"build_ms\":%.9f,\"query_ms\":%.9f,\"destroy_ms\":%.9f,\"examined\":%u,\"retained_structure_bytes\":%zu,\"peak_structure_bytes\":%zu,\"exact_ids_match\":%s,\"aggregates_match\":true,\"query_samples_ms\":[",
                row ? "," : "", row, median(build_times[row][s], runs), median(query_times[row][s], runs), median(destroy_times[row][s], runs), examined[row][s], retained[row], peak[row], exact);
            for (int run = 0; run < runs; run++) printf("%s%.9f", run ? "," : "", query_times[row][s][run]);
            printf("]}");
        }
        printf("]}");
    }
    printf("]}\n");
    for (int s = 0; s < SCENARIOS; s++) query_apagar(&queries[s]);
    for (int i = 0; i < N; i++) item_apagar(&items[i]);
    free(records);
    require(bench_live_bytes() == 0, "live allocations after cleanup");
    return 0;
}
