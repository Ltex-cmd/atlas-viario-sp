import {buildRoadDensity} from './road-density';
self.onmessage=(event:MessageEvent)=>{
  const {id,roads,points,bounds,reach}=event.data;
  try{self.postMessage({id,...buildRoadDensity(roads,points,bounds,reach)});}catch{self.postMessage({id,reason:'error',collection:{type:'FeatureCollection',features:[]},total:points.length,matched:0,unmatched:points.length});}
};
