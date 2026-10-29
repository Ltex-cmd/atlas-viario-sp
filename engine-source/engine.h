#ifndef ATLAS_ENGINE_H
#define ATLAS_ENGINE_H
#include <stdint.h>
#include <stddef.h>
/* Binary v1: little-endian, IEEE754 double; originals kept in CSV/detail shards. */
typedef struct { double lat, lon; uint32_t id, ibge, date; uint16_t fatal, grave, leve, ileso, unknown; uint8_t type, road, registration, transport, geo, reserved; uint16_t city; uint16_t pad; } Record;
_Static_assert(sizeof(Record)==48,"Record ABI");
typedef struct RouteCorridor RouteCorridor;
typedef struct { uint32_t start, end, city; int type, road, transport, registration, spatial, method, zoom; double west,south,east,north; int grid_size,grid_cell; const RouteCorridor *corridor; } Filter;
typedef struct { uint32_t total, located, fatal_events, fatal, grave, leve, ileso, unknown, examined, nodes; uint32_t months[12], types[8], cities[1024]; uint64_t fingerprint; } Stats;
typedef struct { uint32_t count, fatal, first_id; double lat,lon; } Cluster;
typedef struct Engine Engine;
typedef struct { double allocation_ms,date_ms,city_ms,id_ms,hash_ms,grid_ms,quad_ms,total_ms; size_t records_bytes,date_bytes,city_bytes,id_bytes,hash_bytes,grid_bytes,quad_bytes; } EngineProfile;
Engine *engine_create(Record *records,uint32_t n);
const EngineProfile *engine_profile(Engine *e);
void engine_destroy(Engine *e);
void engine_query(Engine *e,const Filter *f,Stats *out,Cluster *clusters,int cells);
uint32_t engine_select(Engine *e,const Filter *f,uint32_t offset,uint32_t limit,Record *out,uint32_t *total);
void *query_items(void);
uint32_t query_items_total(void);
uint32_t run_select(Engine *e,uint32_t offset,uint32_t limit);
size_t engine_bytes(Engine *e);
uint32_t engine_count(Engine *e);
Record *engine_find(Engine *e,uint32_t id,int hash);
/* ABI exported to the hosted C/Wasm adapter. */
void *allocate(size_t n);
void release(void *p);
void *query_result(void);
void *query_clusters(void);
void *query_filter(void);
void run_query(Engine *e,int cells);
/* Coordinates are longitude/latitude pairs. A record is counted once even
   when several segments pass nearby. Zero vertices clears the route filter. */
int query_set_route(const double *coordinates,uint32_t vertices,double radius_m);
#endif
