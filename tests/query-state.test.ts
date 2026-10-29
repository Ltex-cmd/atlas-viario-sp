import {test} from 'node:test';
import assert from 'node:assert/strict';
import {findCity,filterKey,makeExport,validateDates,statisticsAreCurrent} from '../components/query-state.ts';
const filters={ano:'2026',municipio:'',tipo:'',transporte:'',via:'',registro:'',inicio:'',fim:''};
test('statistics distinguish a real zero from absent, pending or previous-filter data',()=>{
  const zero={total:0,months:Array(12).fill(0)},key=filterKey(filters);
  assert.equal(statisticsAreCurrent(zero,key,filters,false),true);
  assert.equal(statisticsAreCurrent(null,key,filters,false),false);
  assert.equal(statisticsAreCurrent(zero,key,filters,true),false);
  assert.equal(statisticsAreCurrent(zero,key,{...filters,ano:'2025'},false),false);
});
test('municipality lookup accepts accents, case and whitespace without partial matches',()=>{
  const cities=[{name:'SAO PAULO',code:3550308},{name:'SAO PEDRO',code:3550407}];
  for(const name of ['São Paulo','são paulo','  SAO   PAULO  '])assert.equal(findCity(cities,name)?.code,3550308);
  assert.equal(findCity(cities,'São'),undefined);
  assert.equal(findCity([...cities,{name:'São Paulo',code:123}], 'São Paulo'),undefined);
});
test('dates respect actual coverage, calendar validity and inclusive single days',()=>{
  const check=(inicio:string,fim:string)=>validateDates({...filters,inicio,fim},'2015-01-01','2026-09-30');
  assert.equal(check('2026-09-30','2026-09-30'),'');assert.equal(check('2024-02-29',''),'');
  for(const pair of [['2026-02-29',''],['2026-10-01',''],['2014-12-31',''],['2026-09-30','2026-01-01']])assert.ok(check(...pair as [string,string]));
});
test('exports reject stale or pending responses and retain the exact displayed region',()=>{
  const input={meta:{version:'sample',source_url:'source'},filters,statistics:{total:3},statisticsKey:filterKey(filters),map:{count:2,features:[]},mapKey:filterKey(filters),viewport:{bbox:'-47,-24,-46,-23',zoom:8,resolution:4},busy:false};
  const output=makeExport(input);assert.equal(output.scope.bbox,input.viewport.bbox);assert.equal(output.statistics.total,3);assert.equal(output.map.count,2);
  for(const patch of [{busy:true},{statisticsKey:'ano=2025'},{mapKey:'ano=2025'},{map:null},{statistics:null}])assert.throws(()=>makeExport({...input,...patch}));
});
