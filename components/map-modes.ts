import type { Map } from 'maplibre-gl';
export type MapMode='groups'|'density'|'roads'|'points';
export function applyMapMode(map:Map,mode:MapMode,showOriginal:boolean,individual:boolean,journey=false){
  const showEvents=!journey&&(mode==='groups'||individual&&(mode==='points'||mode==='roads'&&showOriginal));
  for(const id of ['events','counts','event-hit'])map.setLayoutProperty(id,'visibility',showEvents?'visible':'none');
  map.setLayoutProperty('events-glow','visibility',!journey&&mode==='groups'?'visible':'none');
  map.setLayoutProperty('density','visibility',!journey&&mode==='density'?'visible':'none');
  map.setPaintProperty('events','circle-radius',['case',['==',['get','count'],1],mode==='roads'?3.2:3.8,['get','radius']]);
  map.setPaintProperty('events','circle-color',['case',['==',['get','count'],1],mode==='roads'?'#145d70':'#ad4b27',['step',['get','count'],'#dc9768',30,'#ca7745',150,'#b85b32',500,'#a64b29']]);
}
