import {test} from 'node:test';
import assert from 'node:assert/strict';
import {buildRoadDensity,isRoad} from '../components/road-density.ts';
const coord=(x:number,y:number):[number,number]=>[x/111320,y/111320];
const road=(xy:number[][])=>({properties:{kind:'highway'},geometry:{type:'LineString',coordinates:xy.map(([x,y])=>coord(x,y))}});
const point=(x:number,y:number,id=1)=>({properties:{id,count:1},geometry:{coordinates:coord(x,y)}});
const bounds=[-0.01,-0.01,0.01,0.01];
test('diffusion follows the curve and stays on the road geometry',()=>{
  const roads=[road([[0,0],[100,0],[100,200]])],points=[point(90,2)],original=structuredClone(points);
  const result=buildRoadDensity(roads,points,bounds,300);assert.equal(result.matched,1);assert.ok(result.collection.features.length>0);
  for(const f of result.collection.features)for(const [x,y] of f.geometry.coordinates)assert.ok(Math.abs(y)<1e-8||Math.abs(x-100/111320)<1e-8);
  assert.deepEqual(points,original);
});
test('nearby disconnected roads are not colored by two-dimensional blur',()=>{
  const result=buildRoadDensity([road([[-300,0],[300,0]]),road([[-300,40],[300,40]])],[point(0,0)],bounds,300);
  assert.equal(result.matched,1);assert.ok(result.collection.features.length>0);
  for(const f of result.collection.features)assert.equal(f.geometry.coordinates[0][1],0);
});
test('points outside the matching tolerance remain accounted for without invented matches',()=>{
  const result=buildRoadDensity([road([[-300,0],[300,0]])],[point(0,0),point(0,200,2)],bounds);
  assert.equal(result.total,2);assert.equal(result.matched,1);assert.equal(result.unmatched,1);
});
test('diffusion crosses connected segments and handles duplicate map tiles',()=>{
  const roads=[road([[-200,0],[0,0]]),road([[0,0],[200,0]])];
  const a=buildRoadDensity(roads,[point(-20,0)],bounds),b=buildRoadDensity([...roads,...roads],[point(-20,0)],bounds);
  assert.equal(a.collection.features.length,b.collection.features.length);assert.ok(a.collection.features.some(f=>f.geometry.coordinates[0][0]>0));
});
test('aggregate centroids are rejected as a source for road precision',()=>{
  const p=point(0,0);p.properties.count=30;
  const result=buildRoadDensity([road([[-300,0],[300,0]])],[p],bounds);
  assert.equal(result.reason,'aggregates');assert.equal(result.collection.features.length,0);
});
test('urban avenues do not receive road diffusion unless explicitly classified or named as highways',()=>{
  assert.equal(isRoad({kind:'major_road',kind_detail:'primary',name:'Avenida Ipiranga'}),false);
  assert.equal(isRoad({kind:'major_road',kind_detail:'secondary',name:'Rua da Consolação'}),false);
  assert.equal(isRoad({kind:'major_road',kind_detail:'primary',ref:'SP-332'}),true);
  assert.equal(isRoad({kind:'major_road',kind_detail:'primary',name:'Rodovia Professor Zeferino Vaz'}),true);
  const avenue=road([[0,0],[0,300]]);avenue.properties={kind:'major_road'};
  const result=buildRoadDensity([road([[-300,0],[300,0]]),avenue],[point(0,0)],bounds,600);
  assert.ok(result.collection.features.length>0);for(const f of result.collection.features)for(const c of f.geometry.coordinates)assert.equal(c[1],0);
});
