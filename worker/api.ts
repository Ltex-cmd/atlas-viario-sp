// The Worker handles transport and JSON. Filtering, indexes and aggregation execute in C.
import { prepareComparison, finishComparison, type ComparisonContext } from './route-comparison';
import { routeLength } from '../lib/journey';
import wasmModule from './engine.wasm?module';
import { createPartitionReader } from './partition-cache';
import { JourneyError, searchPlaces, requestJourney, readJourneyBody } from './journey-provider';
import { validateRoute, routeBounds } from '../lib/journey';
type Env={ASSETS:{fetch:(request:Request)=>Promise<Response>}};
let metadata:any=null;
const allowed=new Set(['ano','municipio','tipo','via','transporte','registro','metodo','zoom','inicio','fim','bbox','celula','grade','offset','limit','resolucao','pontos']);
export function parseFilters(p:URLSearchParams,spatial:boolean){
 const seen=new Set();for(const [k] of p){if(!allowed.has(k)||seen.has(k))throw Error('Filtro desconhecido ou repetido.');seen.add(k);}
 const num=(k:string,min:number,max:number,def:number)=>{const s=p.get(k);if(s===null)return def;if(!/^\d+$/.test(s))throw Error(`Valor inválido: ${k}`);const n=Number(s);if(n<min||n>max)throw Error(`Valor fora do intervalo: ${k}`);return n;};
 const y=num('ano',2015,2100,0);let start=y?y*10000+101:0,end=y?y*10000+1231:0;
 const date=(s:string)=>{if(!/^\d{4}-\d{2}-\d{2}$/.test(s))throw Error('Data inválida.');const d=new Date(s+'T12:00:00Z');if(!Number.isFinite(+d)||d.toISOString().slice(0,10)!==s||+s.slice(0,4)<2015||+s.slice(0,4)>2100)throw Error('Data inválida.');return Number(s.replaceAll('-',''));};
 if(p.has('inicio'))start=date(p.get('inicio')!);if(p.has('fim'))end=date(p.get('fim')!);if(start&&end&&start>end)throw Error('O início deve preceder o fim.');
 if(p.has('bbox')&&p.get('bbox')!.split(',').some(v=>!/^[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?$/.test(v)))throw Error('Região geográfica inválida.');const bbox=p.has('bbox')?p.get('bbox')!.split(',').map(Number):[-54,-26,-43,-19];if(bbox.length!==4||bbox.some(x=>!Number.isFinite(x))||bbox[0]<-180||bbox[2]>180||bbox[1]<-90||bbox[3]>90||bbox[0]>=bbox[2]||bbox[1]>=bbox[3])throw Error('Região geográfica inválida.');
 const grid_size=num('grade',1,64,0),grid_cell=num('celula',0,4095,0);if(p.has('grade')!==p.has('celula')||(grid_size&&grid_cell>=grid_size*grid_size))throw Error('Agrupamento inválido.');
 return {precise:num('pontos',0,1,0),resolution:num('resolucao',2,64,0),grid_size,grid_cell,offset:num('offset',0,100000000,0),limit:num('limit',1,200,30),start,end,city:num('municipio',1,9999999,0),type:num('tipo',0,4,-1),road:num('via',0,2,-1),transport:num('transporte',0,7,-1),registration:num('registro',0,2,-1),spatial:+spatial,method:num('metodo',0,4,4),zoom:num('zoom',0,22,7),bbox};
}
async function asset(env:Env,name:string){const r=await env.ASSETS.fetch(new Request('http://localhost/data/'+name));if(!r.ok)throw Error('Dados não preparados: '+name+' ('+r.status+')');return r;}
async function meta(env:Env){return metadata??=(await (await asset(env,'metadata.json')).json());}
type EngineState={e:any;ptr:number;db:number};
const readPartition=createPartitionReader<EngineState>((bytes,previous)=>{
 let e:any=previous?.e;
 if(previous){e.engine_destroy(previous.db);e.release(previous.ptr);}
 const imports={wasi_snapshot_preview1:{proc_exit:()=>{throw Error('Falha de memória no motor C');}}};
 if(!e){const instance=new WebAssembly.Instance(wasmModule,imports);e=instance.exports;e._initialize?.();}
 const ptr=e.allocate(bytes.byteLength);if(!ptr)throw Error('Memória insuficiente.');
 new Uint8Array(e.memory.buffer,ptr,bytes.byteLength).set(new Uint8Array(bytes));
 const db=e.engine_create(ptr,bytes.byteLength/48);if(!db){e.release(ptr);throw Error('Falha ao construir índices.');}
 return {e,ptr,db};
});
async function partitionBytes(env:Env,year:number){
 const bytes=await (await asset(env,`year-${year}.bin`)).arrayBuffer();
 if(bytes.byteLength%48)throw Error('Partição incompatível.');
 return bytes;
}
function setEngineFilter(state:EngineState,f:any){
 const {e}=state;
 e.query_set_route(0,0,0);
 let v=new DataView(e.memory.buffer),p=e.query_filter();
 [f.start,f.end,f.city,f.type,f.road,f.transport,f.registration,f.spatial,f.method,f.zoom].forEach((n,i)=>v.setInt32(p+i*4,n,true));
 f.bbox.forEach((n:number,i:number)=>v.setFloat64(p+40+i*8,n,true));
 v.setInt32(p+72,f.grid_size||0,true);v.setInt32(p+76,f.grid_cell||0,true);
 if(f.route){
  const ptr=e.allocate(f.route.length*16);if(!ptr)throw Error('Memória insuficiente.');
  try{v=new DataView(e.memory.buffer);for(let i=0;i<f.route.length;i++){v.setFloat64(ptr+i*16,f.route[i][0],true);v.setFloat64(ptr+i*16+8,f.route[i][1],true);}
   if(!e.query_set_route(ptr,f.route.length,f.corridor_m))throw Error('Percurso inválido.');
  }finally{e.release(ptr);}
 }
}
function execute(state:EngineState,f:any,cells:number){
 const {e,db}=state;setEngineFilter(state,f);
 try{e.run_query(db,cells);}finally{e.query_set_route(0,0,0);}
 const view=new DataView(e.memory.buffer),ptr=e.query_result();
 const a=(off:number,n:number)=>Array.from({length:n},(_,i)=>view.getUint32(ptr+off+i*4,true));const n=a(0,10);
 const s:any={total:n[0],located:n[1],fatal_events:n[2],fatal:n[3],grave:n[4],leve:n[5],ileso:n[6],unknown:n[7],examined:n[8],nodes:n[9],months:a(40,12),types:a(88,8),cities:a(120,1024),checksum:view.getBigUint64(ptr+4216,true),memory_bytes:e.engine_bytes(db),clusters:[]};
 const cp=e.query_clusters();for(let k=0;k<cells*cells;k++){const b=cp+k*32,count=view.getUint32(b,true);s.clusters.push({count,fatal:view.getUint32(b+4,true),id:view.getUint32(b+8,true),lat:view.getFloat64(b+16,true),lon:view.getFloat64(b+24,true)});}return s;
}
function select(state:EngineState,f:any,offset:number,limit:number){
 const {e,db}=state;setEngineFilter(state,f);
 let n:number;try{n=e.run_select(db,offset,limit);}finally{e.query_set_route(0,0,0);}
 const total=e.query_items_total(),v=new DataView(e.memory.buffer),p=e.query_items(),items=[];
 for(let i=0;i<n;i++){const b=p+i*48;items.push({id:v.getUint32(b+16,true),date:v.getUint32(b+24,true),municipio:v.getUint32(b+20,true),latitude:v.getFloat64(b,true),longitude:v.getFloat64(b+8,true),fatal:v.getUint16(b+28,true),grave:v.getUint16(b+30,true),leve:v.getUint16(b+32,true),type:v.getUint8(b+38),road:v.getUint8(b+39),registration:v.getUint8(b+40),transport:v.getUint8(b+41),coordinates_valid:!!v.getUint8(b+42)});}return {items,total};
}
function merge(a:any,b:any){if(!a)return b;for(const k of ['total','located','fatal_events','fatal','grave','leve','ileso','unknown','examined','nodes'])a[k]+=b[k];a.checksum=BigInt.asUintN(64,a.checksum+b.checksum);for(const k of ['months','types','cities'])a[k]=a[k].map((v:number,i:number)=>v+b[k][i]);a.memory_bytes=Math.max(a.memory_bytes,b.memory_bytes);a.clusters.forEach((p:any,i:number)=>{const q=b.clusters[i];if(q.count){p.count+=q.count;p.fatal+=q.fatal;p.lat+=q.lat;p.lon+=q.lon;p.id=q.id;}});return a;}
function empty(cells:number){return {total:0,located:0,fatal_events:0,fatal:0,grave:0,leve:0,ileso:0,unknown:0,examined:0,nodes:0,months:Array(12).fill(0),types:Array(8).fill(0),cities:Array(1024).fill(0),checksum:0n,memory_bytes:0,clusters:Array.from({length:cells*cells},()=>({count:0,fatal:0,id:0,lat:0,lon:0}))};}
function parseCSV(line:string){const fields:string[]=[];let v='',quote=false;for(let i=0;i<line.length;i++){const c=line[i];if(c==='"'){if(quote&&line[i+1]==='"'){v+='"';i++;}else quote=!quote;}else if(c===';'&&!quote){fields.push(v);v='';}else v+=c;}fields.push(v);return fields;}
const replies=new Map<string,{body:string,etag:string}>();
export async function handleAPI(request:Request,env:Env):Promise<Response>{
 const url=new URL(request.url);const journeyEvents=url.pathname==='/api/v1/route-events';if(request.method!==(journeyEvents?'POST':'GET'))return Response.json({error:journeyEvents?'Use POST.':'Use GET.'},{status:405,headers:{Allow:journeyEvents?'POST':'GET'}});
 try{
 if(url.pathname==='/api/v1/places'||url.pathname==='/api/v1/route'){const data=url.pathname.endsWith('/places')?await searchPlaces(url.searchParams):await requestJourney(url.searchParams);return Response.json(data,{headers:{'Cache-Control':'no-store','X-Content-Type-Options':'nosniff'}});}
 const m=await meta(env);const etag=`"${m.version}-${m.presentation_revision||0}-${url.pathname}-${url.search}"`;const cacheable=request.method==='GET'&&!url.pathname.endsWith('/benchmarks');const key=url.pathname+url.search;
 if(cacheable&&request.headers.get('If-None-Match')===etag)return new Response(null,{status:304,headers:{ETag:etag}});
 const old=replies.get(key);if(old&&cacheable)return new Response(old.body,{headers:{'Content-Type':'application/json; charset=utf-8','Cache-Control':'private, max-age=60',ETag:old.etag,'X-Atlas-Cache':'hit'}});
 let output:any;
 if(journeyEvents){
 const body=await readJourneyBody(request);
 const coordinates=validateRoute(body?.coordinates),radius=body.corridor_m;
 if(![25,50,100].includes(radius))throw new JourneyError('Escolha uma proximidade de 25, 50 ou 100 metros.');
 if(typeof body.filters!=='string'||body.filters.length>2048)throw new JourneyError('Filtros do percurso inválidos.');
 const params=new URLSearchParams(body.filters);
 if(params.has('bbox')||params.has('grade')||params.has('celula'))throw new JourneyError('Filtros do percurso inválidos.');
 params.set('bbox',routeBounds(coordinates,radius).join(','));
 const f={...parseFilters(params,true),route:coordinates,corridor_m:radius};
 const offset=body.offset??0,limit=body.limit??30;
 if(!Number.isInteger(offset)||offset<0||offset>100000000||!Number.isInteger(limit)||limit<1||limit>200)throw new JourneyError('Página inválida.');
 const includeMap=body.include_map===true,points:any[]=[],items:any[]=[];
 let total=0,remainingOffset=offset;
 let comparisonContext:ComparisonContext|undefined,comparisonFailed=false;
 if(includeMap)try{comparisonContext=prepareComparison(coordinates,f.city);}catch(error){comparisonFailed=true;console.error('Comparação do percurso',error);}
 const years=m.years.filter((y:any)=>(!f.start||y.year>=Math.floor(f.start/10000))&&(!f.end||y.year<=Math.floor(f.end/10000)));
 for(const y of years)await readPartition(y.year,()=>partitionBytes(env,y.year),(state)=>{
  const page=select(state,f,Math.max(0,remainingOffset),Math.max(0,limit-items.length));
  items.push(...page.items);total+=page.total;remainingOffset-=page.total;
  if(includeMap&&points.length<2000)points.push(...select(state,f,0,2000-points.length).items);
  if(comparisonContext&&!comparisonFailed)try{for(const [code] of comparisonContext.selected)comparisonContext.counts[code]=(comparisonContext.counts[code]||0)+execute(state,{...f,city:Number(code),spatial:0,bbox:[-54,-26,-43,-19],grid_size:0,grid_cell:0,route:undefined},1).located;}catch(error){comparisonFailed=true;console.error('Comparação do percurso',error);}
 });
 output={total,offset,limit,items,has_more:offset+items.length<total,corridor_m:radius,bbox:f.bbox,filters:body.filters,points:includeMap?points:undefined,map_limit:2000,map_shown:includeMap?points.length:undefined,order:'data crescente; ordem da fonte em empates',distance_model:'distância aproximada à linha do percurso; projeção local com latitude de referência'};
 if(includeMap){try{if(comparisonFailed||!comparisonContext)throw Error('Referência indisponível');output.comparison=finishComparison(comparisonContext,total,radius,m.cities);}catch(error){console.error('Comparação do percurso',error);output.comparison={status:'unavailable',reason:'no_reference',observed:total,route_km:Math.round(routeLength(coordinates)/10)/100,municipalities:[],level:null};}}
 }
 else if(url.pathname==='/api/v1/metadata')output=m;
 else if(url.pathname.startsWith('/api/v1/sinistros/')){
 const id=url.pathname.split('/').pop()!;if(!/^\d+$/.test(id)||Number(id)>4294967295)throw Error('Identificador inválido.');let raw:Response;try{raw=await asset(env,`details-${Math.floor(Number(id)/5000)}.csvz`);}catch{return Response.json({error:'Sinistro não encontrado.'},{status:404});}
 const data=new Uint8Array(await new Response(raw.body!.pipeThrough(new DecompressionStream('gzip'))).arrayBuffer());let text='';for(let offset=0;offset<data.length;offset+=8192)text+=String.fromCharCode(...data.subarray(offset,offset+8192));const line=text.split(/\r?\n/).find(row=>row.startsWith(id+';'));if(!line)return Response.json({error:'Sinistro não encontrado.'},{status:404});const values=parseCSV(line);output={id:Number(id),attributes:Object.fromEntries(m.columns.map((name:string,i:number)=>[name,values[i]])),source_sha256:m.source_sha256,encoding:m.encoding};
 }else if(url.pathname==='/api/v1/native-benchmarks'){output=await (await asset(env,'native-benchmarks.json')).json();}
 else{
 const kind=url.pathname.split('/').pop();if(!['map','statistics','benchmarks','events'].includes(kind!))return Response.json({error:'Endpoint inexistente.'},{status:404});const f=parseFilters(url.searchParams,kind!=='statistics');const years=m.years.filter((y:any)=>(!f.start||y.year>=Math.floor(f.start/10000))&&(!f.end||y.year<=Math.floor(f.end/10000)));const cells=f.resolution||(f.zoom<8?16:f.zoom<10?24:f.zoom<12?40:64);
 const pointLimit=f.precise?12000:2000;const job=async()=>{let result=empty(kind==='map'?cells:1);let methods:any[]=[];let cold=0;let items:any[]=[];let pageTotal=0;let remainingOffset=f.offset;for(const y of years){await readPartition(y.year,()=>partitionBytes(env,y.year),(state,warm)=>{cold+=!warm?1:0;if(kind==='benchmarks'){for(let method=0;method<5;method++)methods[method]=merge(methods[method],execute(state,{...f,method},1));}else if(kind==='events'){const page=select(state,f,Math.max(0,remainingOffset),Math.max(0,f.limit-items.length));items.push(...page.items);pageTotal+=page.total;remainingOffset-=page.total;}else{result=merge(result,execute(state,f,kind==='map'?cells:1));if(kind==='map'&&(f.precise||f.zoom>=12)&&items.length<pointLimit)items.push(...select(state,f,0,pointLimit-items.length).items);}});}
 if(kind==='events')return {total:pageTotal,offset:f.offset,limit:f.limit,items,has_more:f.offset+items.length<pageTotal,order:'data crescente; ordem da fonte em empates'};
 if(kind==='benchmarks'){if(!years.length)methods=Array.from({length:5},()=>empty(1));return {environment:'C / WebAssembly no servidor',timing_note:'O servidor não fornece uma medição isolada confiável de CPU para esta consulta. As medições do computador de teste estão nos cenários fixos.',methods:methods.map((r,i)=>({method:i,count:r.total,examined:r.examined,nodes:r.nodes,checksum:r.checksum.toString(),equivalent:r.total===methods[0].total&&r.checksum===methods[0].checksum,median_ms:null,memory_bytes:r.memory_bytes})),partitions:years.length,cold_partitions:cold};}
 if(kind==='map'){const individual=(f.precise||f.zoom>=12)&&result.total<=pointLimit;return {type:'FeatureCollection',mode:individual?'points':'clusters',count:result.total,examined:result.examined,query_ms:null,features:individual?items.map(p=>({type:'Feature',geometry:{type:'Point',coordinates:[p.longitude,p.latitude]},properties:{count:1,fatal:p.fatal,id:p.id,date:p.date,type:p.type,registration:p.registration}})):result.clusters.flatMap((p:any,k:number)=>p.count?[{type:'Feature',geometry:{type:'Point',coordinates:[p.lon/p.count,p.lat/p.count]},properties:{count:p.count,fatal:p.fatal,id:p.count===1?p.id:0,cell:k,grid:cells,bbox:f.bbox.join(',')}}]:[]),partitions:years.length,cold_partitions:cold};}

 const {clusters,checksum,...stats}=result;return {...stats,unlocated:stats.total-stats.located,query_ms:null,partitions:years.length,cold_partitions:cold};};
 output=await job();
 }
 const body=JSON.stringify(output);if(cacheable&&body.length<700000){if(replies.size>=48)replies.delete(replies.keys().next().value!);replies.set(key,{body,etag});}
 return new Response(body,{headers:{'Content-Type':'application/json; charset=utf-8','Cache-Control':cacheable?'private, max-age=60':'no-store',ETag:etag,'X-Content-Type-Options':'nosniff'}});
 }catch(e:any){console.error('Atlas API',url.pathname,e.message);return Response.json({error:e.message||'Não foi possível consultar a base.'},{status:e instanceof JourneyError?e.status:/inválid|Filtro|intervalo|preceder|Região|Este teste aceita/.test(e.message)?400:503});}
}
