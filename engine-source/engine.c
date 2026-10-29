#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef __wasm__
#include <time.h>
#endif
typedef struct { uint32_t key,index; } Entry;
typedef struct { double w,s,e,n; uint32_t start,count,children[4]; } Node;
struct Engine { Record *r; uint32_t n; Entry *date,*city,*id; uint32_t *hash,*spatial,*grid,*grid_offsets; size_t hcap; Node *nodes; uint32_t nnodes,capnodes; EngineProfile profile; };
typedef struct { double x,y,dx,dy,length2,w,s,e,n; } RouteSegment;
struct RouteCorridor { RouteSegment *segments; uint32_t count; double scale_x,scale_y,radius2; };
static int in_corridor(const Record *record,const RouteCorridor *route){
 if(!record->geo)return 0;
 double x=record->lon*route->scale_x,y=record->lat*route->scale_y;
 for(uint32_t i=0;i<route->count;i++){
  const RouteSegment *s=&route->segments[i];
  if(x<s->w||x>s->e||y<s->s||y>s->n)continue;
  double t=s->length2?((x-s->x)*s->dx+(y-s->y)*s->dy)/s->length2:0;
  if(t<0)t=0;else if(t>1)t=1;
  double dx=x-s->x-t*s->dx,dy=y-s->y-t*s->dy;
  if(dx*dx+dy*dy<=route->radius2+1e-7)return 1;
 }
 return 0;
}
static double profile_now(void){
#ifdef __wasm__
 return 0; /* Worker CPU clocks are not a valid benchmark. Native measurements only. */
#else
 struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec*1000.0+t.tv_nsec/1e6;
#endif
}
#define GRID 128
static int cmp_entry(const void *a,const void *b){const Entry *x=a,*y=b;return x->key<y->key?-1:x->key>y->key?1:x->index<y->index?-1:x->index>y->index;}
static uint32_t lower(Entry *a,uint32_t n,uint32_t k){uint32_t l=0;while(l<n){uint32_t m=l+(n-l)/2;if(a[m].key<k)l=m+1;else n=m;}return l;}
static int cell(double x,double lo,double hi){int k=(int)((x-lo)/(hi-lo)*GRID);return k<0?0:k>=GRID?GRID-1:k;}
static uint32_t quad(Engine *e,uint32_t start,uint32_t count,double w,double s,double east,double north,int depth){
 uint32_t ni=e->nnodes++;Node node={w,s,east,north,start,count,{0,0,0,0}};
 if(count>128&&depth<14){double mx=(w+east)/2,my=(s+north)/2;uint32_t counts[4]={0},offsets[4],next[4];
 for(uint32_t i=start;i<start+count;i++){Record *r=&e->r[e->spatial[i]];counts[(r->lon>=mx)+2*(r->lat>=my)]++;}
 offsets[0]=start;for(int c=1;c<4;c++)offsets[c]=offsets[c-1]+counts[c-1];memcpy(next,offsets,sizeof next);
 /* In-place American-flag partition, no record copying. */
 for(int c=0;c<4;c++)while(next[c]<offsets[c]+counts[c]){uint32_t pos=next[c];Record *r=&e->r[e->spatial[pos]];int t=(r->lon>=mx)+2*(r->lat>=my);if(t==c){next[c]++;continue;}uint32_t other=next[t]++,v=e->spatial[pos];e->spatial[pos]=e->spatial[other];e->spatial[other]=v;}
 for(int c=0;c<4;c++)if(counts[c])node.children[c]=quad(e,offsets[c],counts[c],c&1?mx:w,c&2?my:s,c&1?east:mx,c&2?north:my,depth+1);
 }
 if(ni>=e->capnodes){uint32_t capacity=e->capnodes?e->capnodes*2:1024;while(capacity<=ni)capacity*=2;Node *p=realloc(e->nodes,(size_t)capacity*sizeof(Node));if(!p)abort();e->nodes=p;e->capnodes=capacity;}e->nodes[ni]=node;return ni;
}
Engine *engine_create(Record *records,uint32_t n){double start=profile_now(),t=start;Engine *e=calloc(1,sizeof *e);if(!e)return NULL;e->r=records;e->n=n;e->hcap=1;while(e->hcap<(size_t)n*2)e->hcap*=2;
 e->date=malloc((size_t)(n+1)*sizeof(Entry));e->city=malloc((size_t)(n+1)*sizeof(Entry));e->id=malloc((size_t)(n+1)*sizeof(Entry));e->spatial=malloc((size_t)(n+1)*4);e->grid=malloc((size_t)(n+1)*4);e->grid_offsets=calloc(GRID*GRID+1,4);e->hash=calloc(e->hcap,4);
 if(!e->date||!e->city||!e->id||!e->spatial||!e->grid||!e->grid_offsets||!e->hash){engine_destroy(e);return NULL;}
 e->profile.allocation_ms=profile_now()-t;t=profile_now();
 for(uint32_t i=0;i<n;i++)e->date[i]=(Entry){records[i].date,i};qsort(e->date,n,sizeof(Entry),cmp_entry);e->profile.date_ms=profile_now()-t;t=profile_now();
 for(uint32_t i=0;i<n;i++)e->city[i]=(Entry){records[i].ibge,i};qsort(e->city,n,sizeof(Entry),cmp_entry);e->profile.city_ms=profile_now()-t;t=profile_now();
 for(uint32_t i=0;i<n;i++)e->id[i]=(Entry){records[i].id,i};qsort(e->id,n,sizeof(Entry),cmp_entry);e->profile.id_ms=profile_now()-t;t=profile_now();
 for(uint32_t i=0;i<n;i++){size_t k=(records[i].id*2654435761u)&(e->hcap-1);while(e->hash[k])k=(k+1)&(e->hcap-1);e->hash[k]=i+1;}e->profile.hash_ms=profile_now()-t;t=profile_now();
 uint32_t valid=0;for(uint32_t i=0;i<n;i++){Record *r=&records[i];if(r->geo){e->spatial[valid++]=i;e->grid_offsets[cell(r->lat,-26,-19)*GRID+cell(r->lon,-54,-43)+1]++;}}
 for(int i=1;i<=GRID*GRID;i++)e->grid_offsets[i]+=e->grid_offsets[i-1];uint32_t *next=malloc(GRID*GRID*4);if(!next){engine_destroy(e);return NULL;}memcpy(next,e->grid_offsets,GRID*GRID*4);for(uint32_t i=0;i<n;i++)if(records[i].geo){int g=cell(records[i].lat,-26,-19)*GRID+cell(records[i].lon,-54,-43);e->grid[next[g]++]=i;}free(next);
 e->profile.grid_ms=profile_now()-t;t=profile_now();quad(e,0,valid,-54,-26,-43,-19,0);e->profile.quad_ms=profile_now()-t;e->profile.total_ms=profile_now()-start;
 e->profile.records_bytes=(size_t)n*sizeof(Record);e->profile.date_bytes=e->profile.city_bytes=e->profile.id_bytes=(size_t)(n+1)*sizeof(Entry);e->profile.hash_bytes=e->hcap*4;e->profile.grid_bytes=(size_t)(n+1)*4+(GRID*GRID+1)*4;e->profile.quad_bytes=(size_t)(n+1)*4+(size_t)e->capnodes*sizeof(Node);return e;
}
void engine_destroy(Engine *e){if(!e)return;free(e->date);free(e->city);free(e->id);free(e->spatial);free(e->grid);free(e->grid_offsets);free(e->hash);free(e->nodes);free(e);}
size_t engine_bytes(Engine *e){return sizeof *e+e->profile.records_bytes+e->profile.date_bytes+e->profile.city_bytes+e->profile.id_bytes+e->profile.hash_bytes+e->profile.grid_bytes+e->profile.quad_bytes;}
const EngineProfile *engine_profile(Engine *e){return &e->profile;}
uint32_t engine_count(Engine *e){return e->n;}
Record *engine_find(Engine *e,uint32_t id,int hash){if(hash){size_t k=(id*2654435761u)&(e->hcap-1);while(e->hash[k]){Record *r=&e->r[e->hash[k]-1];if(r->id==id)return r;k=(k+1)&(e->hcap-1);}return NULL;}uint32_t k=lower(e->id,e->n,id);return k<e->n&&e->id[k].key==id?&e->r[e->id[k].index]:NULL;}
static int matches(const Record *r,const Filter *f){
 if((f->start&&r->date<f->start)||(f->end&&r->date>f->end)||(f->city&&r->ibge!=f->city)||(f->type>=0&&r->type!=f->type)||(f->road>=0&&r->road!=f->road)||(f->registration>=0&&r->registration!=f->registration)||(f->transport>=0&&!(r->transport&(1u<<f->transport))))return 0;
 if(f->spatial&&(!r->geo||r->lon<f->west||r->lon>f->east||r->lat<f->south||r->lat>f->north))return 0;
 if(f->grid_size>0){if(!r->geo||r->lon<f->west||r->lon>f->east||r->lat<f->south||r->lat>f->north)return 0;int x=(int)((r->lon-f->west)/(f->east-f->west)*f->grid_size),y=(int)((r->lat-f->south)/(f->north-f->south)*f->grid_size);if(x>=f->grid_size)x=f->grid_size-1;if(y>=f->grid_size)y=f->grid_size-1;if(y*f->grid_size+x!=f->grid_cell)return 0;}
 return !f->corridor||in_corridor(r,f->corridor);
}
static void visit(Engine *e,uint32_t i,const Filter *f,Stats *s,Cluster *c,int cells){s->examined++;Record *r=&e->r[i];if(!matches(r,f))return;s->total++;s->fingerprint+=(uint64_t)r->id*11400714819323198485ull+(uint64_t)r->date;s->located+=r->geo;s->fatal_events+=r->fatal>0;s->fatal+=r->fatal;s->grave+=r->grave;s->leve+=r->leve;s->ileso+=r->ileso;s->unknown+=r->unknown;int m=(r->date/100)%100;if(m>=1&&m<=12)s->months[m-1]++;if(r->type<8)s->types[r->type]++;if(r->city<1024)s->cities[r->city]++;
 if(c&&r->geo&&r->lon>=f->west&&r->lon<=f->east&&r->lat>=f->south&&r->lat<=f->north){int x=(int)((r->lon-f->west)/(f->east-f->west)*cells),y=(int)((r->lat-f->south)/(f->north-f->south)*cells);if(x>=cells)x=cells-1;if(y>=cells)y=cells-1;if(x<0||y<0)return;Cluster *p=&c[y*cells+x];p->count++;p->fatal+=r->fatal;p->lat+=r->lat;p->lon+=r->lon;p->first_id=r->id;}}
static void query_quad(Engine *e,uint32_t ni,const Filter *f,Stats *s,Cluster *c,int cells){Node *n=&e->nodes[ni];s->nodes++;if(n->e<f->west||n->w>f->east||n->n<f->south||n->s>f->north)return;if(!n->children[0]&&!n->children[1]&&!n->children[2]&&!n->children[3]){for(uint32_t k=n->start;k<n->start+n->count;k++)visit(e,e->spatial[k],f,s,c,cells);}else for(int j=0;j<4;j++)if(n->children[j])query_quad(e,n->children[j],f,s,c,cells);}
static uint32_t estimate_quad(Engine *e,uint32_t ni,const Filter *f,uint32_t cap){Node *n=&e->nodes[ni];if(n->e<f->west||n->w>f->east||n->n<f->south||n->s>f->north)return 0;if((f->west<=n->w&&f->east>=n->e&&f->south<=n->s&&f->north>=n->n)||(!n->children[0]&&!n->children[1]&&!n->children[2]&&!n->children[3]))return n->count;uint32_t count=0;for(int j=0;j<4;j++)if(n->children[j]){count+=estimate_quad(e,n->children[j],f,cap>count?cap-count:0);if(count>=cap)break;}return count;}
void engine_query(Engine *e,const Filter *f,Stats *s,Cluster *c,int cells){memset(s,0,sizeof *s);if(c)memset(c,0,(size_t)cells*cells*sizeof *c);Entry *index=e->date;uint32_t lo=lower(e->date,e->n,f->start),hi=f->end?lower(e->date,e->n,f->end+1):e->n;if(f->city){uint32_t a=lower(e->city,e->n,f->city),b=lower(e->city,e->n,f->city+1);if(b-a<hi-lo){index=e->city;lo=a;hi=b;}}
 int method=f->method;if(method==4)method=f->spatial&&estimate_quad(e,0,f,hi-lo)<hi-lo?3:1;
 if(method==2&&f->spatial){int x0=cell(f->west,-54,-43),x1=cell(f->east,-54,-43),y0=cell(f->south,-26,-19),y1=cell(f->north,-26,-19);for(int y=y0;y<=y1;y++)for(int x=x0;x<=x1;x++){s->nodes++;int g=y*GRID+x;for(uint32_t k=e->grid_offsets[g];k<e->grid_offsets[g+1];k++)visit(e,e->grid[k],f,s,c,cells);}}
 else if(method==3&&f->spatial)query_quad(e,0,f,s,c,cells);
 else if(method==0){for(uint32_t i=0;i<e->n;i++)visit(e,i,f,s,c,cells);}
 else for(uint32_t k=lo;k<hi;k++)visit(e,index[k].index,f,s,c,cells);
}
static Stats result;static Cluster clusters[4096];static Filter filter;
static RouteCorridor query_corridor;
int query_set_route(const double *coordinates,uint32_t vertices,double radius_m){
 free(query_corridor.segments);memset(&query_corridor,0,sizeof query_corridor);filter.corridor=NULL;
 if(!vertices)return 1;
 if(!coordinates||vertices<2||vertices>10000||!isfinite(radius_m)||radius_m<1||radius_m>250)return 0;
 double south=90,north=-90;
 for(uint32_t i=0;i<vertices;i++){
  double lon=coordinates[i*2],lat=coordinates[i*2+1];
  if(!isfinite(lon)||!isfinite(lat)||lon<-54||lon>-43||lat<-26||lat>-19)return 0;
  if(lat<south)south=lat;if(lat>north)north=lat;
 }
 query_corridor.scale_y=3.14159265358979323846*6371008.8/180;
 query_corridor.scale_x=query_corridor.scale_y*cos((south+north)*3.14159265358979323846/360);
 query_corridor.radius2=radius_m*radius_m;
 query_corridor.count=vertices-1;
 query_corridor.segments=malloc((size_t)(vertices-1)*sizeof(RouteSegment));
 if(!query_corridor.segments){memset(&query_corridor,0,sizeof query_corridor);return 0;}
 for(uint32_t i=0;i<vertices-1;i++){
  double x=coordinates[i*2]*query_corridor.scale_x,y=coordinates[i*2+1]*query_corridor.scale_y;
  double next_x=coordinates[(i+1)*2]*query_corridor.scale_x,next_y=coordinates[(i+1)*2+1]*query_corridor.scale_y;
  double dx=next_x-x,dy=next_y-y;
  query_corridor.segments[i]=(RouteSegment){x,y,dx,dy,dx*dx+dy*dy,fmin(x,next_x)-radius_m,fmin(y,next_y)-radius_m,fmax(x,next_x)+radius_m,fmax(y,next_y)+radius_m};
 }
 filter.corridor=&query_corridor;return 1;
}
void *allocate(size_t n){return malloc(n);}void release(void *p){free(p);}void *query_result(void){return &result;}void *query_clusters(void){return clusters;}void *query_filter(void){return &filter;}void run_query(Engine *e,int cells){if(cells<1||cells>64)return;engine_query(e,&filter,&result,clusters,cells);}

uint32_t engine_select(Engine *e,const Filter *f,uint32_t offset,uint32_t limit,Record *out,uint32_t *total){uint32_t matched=0,written=0;uint32_t lo=lower(e->date,e->n,f->start),hi=f->end?lower(e->date,e->n,f->end+1):e->n;for(uint32_t k=lo;k<hi;k++){Record *r=&e->r[e->date[k].index];if(!matches(r,f))continue;if(matched>=offset&&written<limit)out[written++]=*r;matched++;}*total=matched;return written;}
static Record selected[12000];static uint32_t selected_total;
void *query_items(void){return selected;}uint32_t query_items_total(void){return selected_total;}
uint32_t run_select(Engine *e,uint32_t offset,uint32_t limit){if(limit>12000)limit=12000;return engine_select(e,&filter,offset,limit,selected,&selected_total);}
