// Visual density on the road network. Official event coordinates are never modified.
export type Coordinate=[number,number];
type Road={id?:string|number;properties?:Record<string,any>;geometry:{type:string;coordinates:any}};
type EventPoint={geometry:{coordinates:Coordinate};properties:{id?:number;count?:number}};
type Edge={a:number;b:number;length:number;coordinates:[Coordinate,Coordinate];name:string};
type Node={x:number;y:number;edges:number[]};
const empty=()=>({type:'FeatureCollection' as const,features:[] as any[]});
export const ROAD_SNAP_METERS=80;
export function isRoad(properties:Record<string,any>={}) {
  const kind=String(properties.kind_detail||properties.class||''),label=String(properties.ref||'')+' '+String(properties.name||'');
  // Urban primary/secondary/tertiary roads are not automatically highways.
  // Retain explicit highway classes and named/numbered state/federal roads.
  return properties.kind==='highway'||/^(motorway|trunk)(?:_link)?$/.test(kind)||/\b(?:BR|SP)[ -]?\d{2,3}\b/i.test(label)||/\b(?:rodovia|estrada|rodoanel)\b/i.test(label);
}
// A binary heap keeps bounded graph diffusion off the map's animation thread.
class Heap {
  values:[number,number][]=[];
  push(value:[number,number]) {let i=this.values.length;this.values.push(value);while(i){const p=(i-1)>>1;if(this.values[p][0]<=value[0])break;this.values[i]=this.values[p];i=p;}this.values[i]=value;}
  pop(){const first=this.values[0],last=this.values.pop()!;if(this.values.length){let i=0;while(i*2+1<this.values.length){let child=i*2+1;if(child+1<this.values.length&&this.values[child+1][0]<this.values[child][0])child++;if(this.values[child][0]>=last[0])break;this.values[i]=this.values[child];i=child;}this.values[i]=last;}return first;}
}
export function buildRoadDensity(roads:Road[],points:EventPoint[],bounds:number[],reach=300) {
  const output=empty(),n=points.length;
  if(points.some(p=>Number(p.properties.count??1)!==1))return {collection:output,total:n,matched:0,unmatched:n,reason:'aggregates'};
  if(!roads.length||!points.length)return {collection:output,total:n,matched:0,unmatched:n,reason:points.length?'roads_unavailable':'empty'};
  reach=Math.max(100,Math.min(600,reach));
  const latitude=(bounds[1]+bounds[3])/2,lonScale=111320*Math.cos(latitude*Math.PI/180),latScale=111320;
  const project=([lon,lat]:Coordinate)=>({x:(lon-bounds[0])*lonScale,y:(lat-bounds[1])*latScale});
  const right=(bounds[2]-bounds[0])*lonScale,top=(bounds[3]-bounds[1])*latScale;
  const nodes:Node[]=[],edges:Edge[]=[],nodeIDs=new Map<string,number>(),edgeKeys=new Set<string>(),spatial=new Map<string,number[]>();
  const node=(coordinate:Coordinate,level:string)=>{const key=level+':'+coordinate.map(v=>v.toFixed(6)).join(',');let id=nodeIDs.get(key);if(id===undefined){id=nodes.length;nodes.push({...project(coordinate),edges:[]});nodeIDs.set(key,id);}return id;};
  const cell=ROAD_SNAP_METERS;
  for(const road of roads){
    if(!isRoad(road.properties))continue;
    const properties=road.properties||{},level=String(properties.layer??(properties.is_bridge?1:properties.is_tunnel?-1:0));
    const lines=road.geometry.type==='LineString'?[road.geometry.coordinates]:road.geometry.type==='MultiLineString'?road.geometry.coordinates:[];
    for(const line of lines)for(let i=1;i<line.length;i++){
      const start=line[i-1] as Coordinate,end=line[i] as Coordinate,a=project(start),b=project(end);
      if(Math.max(a.x,b.x)<-reach||Math.min(a.x,b.x)>right+reach||Math.max(a.y,b.y)<-reach||Math.min(a.y,b.y)>top+reach)continue;
      const length=Math.hypot(b.x-a.x,b.y-a.y),parts=Math.max(1,Math.ceil(length/40));
      for(let part=0;part<parts;part++){
        const at=(t:number):Coordinate=>[start[0]+(end[0]-start[0])*t,start[1]+(end[1]-start[1])*t];
        const ca=at(part/parts),cb=at((part+1)/parts),ia=node(ca,level),ib=node(cb,level);if(ia===ib)continue;
        const key=ia<ib?`${ia}:${ib}`:`${ib}:${ia}`;if(edgeKeys.has(key))continue;edgeKeys.add(key);
        const id=edges.length,na=nodes[ia],nb=nodes[ib];edges.push({a:ia,b:ib,length:Math.hypot(nb.x-na.x,nb.y-na.y),coordinates:[ca,cb],name:String(properties.name||properties.ref||'')});nodes[ia].edges.push(id);nodes[ib].edges.push(id);
        for(let x=Math.floor((Math.min(na.x,nb.x)-cell)/cell);x<=Math.floor((Math.max(na.x,nb.x)+cell)/cell);x++)for(let y=Math.floor((Math.min(na.y,nb.y)-cell)/cell);y<=Math.floor((Math.max(na.y,nb.y)+cell)/cell);y++){const key=`${x}:${y}`,list=spatial.get(key)||[];list.push(id);spatial.set(key,list);}
        // Do not silently truncate an oversized network or substitute aggregate centroids.
        if(edges.length>60000)return {collection:empty(),total:n,matched:0,unmatched:n,reason:'network_too_large'};
      }
    }
  }
  const weights=new Float64Array(edges.length),seeds=new Map<string,{edge:number;offset:number;count:number}>();let matched=0;
  for(const point of points){
    const p=project(point.geometry.coordinates),candidates=spatial.get(`${Math.floor(p.x/cell)}:${Math.floor(p.y/cell)}`)||[];let best=-1,distance=cell,offset=0;
    for(const id of candidates){const edge=edges[id],a=nodes[edge.a],b=nodes[edge.b],t=Math.max(0,Math.min(1,((p.x-a.x)*(b.x-a.x)+(p.y-a.y)*(b.y-a.y))/(edge.length*edge.length))),d=Math.hypot(p.x-a.x-(b.x-a.x)*t,p.y-a.y-(b.y-a.y)*t);if(d<distance){best=id;distance=d;offset=t*edge.length;}}
    if(best<0)continue;matched++;
    const key=`${best}:${Math.round(offset)}`,seed=seeds.get(key);if(seed)seed.count++;else seeds.set(key,{edge:best,offset,count:1});
  }
  for(const seed of seeds.values()){
    const distances=new Map<number,number>(),heap=new Heap(),visited=new Set<number>(),first=edges[seed.edge];
    const add=(id:number,d:number)=>{if(d<=reach&&d<(distances.get(id)??Infinity)){distances.set(id,d);heap.push([d,id]);}};
    const weight=(id:number,d:number)=>{if(d<=reach)weights[id]+=seed.count*Math.exp(-4.5*(d/reach)**2);};
    weight(seed.edge,Math.abs(first.length/2-seed.offset));visited.add(seed.edge);add(first.a,seed.offset);add(first.b,first.length-seed.offset);
    while(heap.values.length){const [distance,id]=heap.pop();if(distance!==distances.get(id))continue;for(const edgeID of nodes[id].edges){const edge=edges[edgeID];if(!visited.has(edgeID)){weight(edgeID,distance+edge.length/2);visited.add(edgeID);}add(edge.a===id?edge.b:edge.a,distance+edge.length);}}
  }
  let max=0;for(const weight of weights)max=Math.max(max,weight);
  edges.forEach((edge,i)=>{if(weights[i]>0.01)output.features.push({type:'Feature',geometry:{type:'LineString',coordinates:edge.coordinates},properties:{weight:weights[i],intensity:weights[i]/(max||1),name:edge.name}});});
  return {collection:output,total:n,matched,unmatched:n-matched,reason:matched?'ready':'no_matches',segments:edges.length};
}
