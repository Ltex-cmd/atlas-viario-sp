import type {Map,GeoJSONSource} from 'maplibre-gl';
import RoadDensityWorker from './road-density.worker?worker&inline';
export { applyMapMode, type MapMode } from './map-modes';
export const BASEMAP_STYLE='https://basemaps.cartocdn.com/gl/voyager-gl-style/style.json';
export function improveMapContext(map:Map){
  const layers=map.getStyle().layers||[];
  for(const layer of layers){
    if(layer.type==='symbol'&&layer.id.startsWith('place_')){
      map.setPaintProperty(layer.id,'text-color','#334e59');map.setPaintProperty(layer.id,'text-halo-color','#fff');map.setPaintProperty(layer.id,'text-halo-width',1.8);
    }
    if(layer.type==='symbol'&&layer.id.startsWith('roadname_')){
      map.setPaintProperty(layer.id,'text-color','#46515b');map.setPaintProperty(layer.id,'text-halo-color','#fff');map.setPaintProperty(layer.id,'text-halo-width',1.8);
      if(layer.id==='roadname_major'||layer.id==='roadname_pri'){map.setLayerZoomRange(layer.id,10,24);map.setLayoutProperty(layer.id,'text-size',['interpolate',['linear'],['zoom'],10,10,15,12]);}
    }
  }
  // Place and road names are always drawn above occurrence circles and heat layers.
  for(const layer of layers)if(layer.type==='symbol'&&layer.id!=='counts')map.moveLayer(layer.id);

}
export class RoadDensityController {
  private worker:Worker|null=null;private timer:ReturnType<typeof setTimeout>|undefined;private id=0;private enabled=false;private data:any=null;private bbox:number[]=[];private reach=300;
  private onSource=(event:any)=>{if(event.sourceId==='protomaps'&&event.isSourceLoaded)this.schedule();};
  constructor(private map:Map,private report:(state:any)=>void){
    map.addSource('road-density',{type:'geojson',data:{type:'FeatureCollection',features:[]}});
    map.addLayer({id:'road-density',type:'line',source:'road-density',layout:{visibility:'none','line-cap':'round','line-join':'round'},paint:{'line-color':['interpolate',['linear'],['get','intensity'],0,'#f3b73b',.15,'#f59b26',.4,'#ec6823',.7,'#d33d20',1,'#9c201e'],'line-width':['interpolate',['linear'],['zoom'],10,4,13,6,16,9,18,12],'line-blur':.7,'line-opacity':['interpolate',['linear'],['get','intensity'],0,.22,.08,.7,.4,.92]}});
    map.on('sourcedata',this.onSource);
  }
  setMode(enabled:boolean){this.enabled=enabled;this.map.setLayoutProperty('road-density','visibility',enabled?'visible':'none');if(enabled)this.schedule();else this.cancel();}
  setReach(reach:number){this.reach=reach;this.schedule();}
  setData(data:any,bbox:string){this.data=data;this.bbox=bbox.split(',').map(Number);this.schedule();}
  invalidate(){this.cancel();this.data=null;this.clear();if(this.enabled)this.report({reason:'loading'});}
  private clear(){(this.map.getSource('road-density') as GeoJSONSource)?.setData({type:'FeatureCollection',features:[]});}
  private cancel(){clearTimeout(this.timer);this.id++;this.worker?.terminate();this.worker=null;}
  private schedule(){if(!this.enabled)return;clearTimeout(this.timer);this.timer=setTimeout(()=>this.compute(),100);}
  private compute(){
    if(!this.enabled||!this.data||this.map.isMoving())return;
    this.cancel();this.clear();
    if(this.map.getZoom()<10){this.report({reason:'zoom_level',total:this.data.count});return;}
    if(this.data.mode!=='points'){this.report({reason:'point_limit',total:this.data.count});return;}
    const roads=this.map.querySourceFeatures('protomaps',{sourceLayer:'roads',filter:['in',['get','kind'],['literal',['highway','major_road']]]}).map(f=>({id:f.id,properties:f.properties,geometry:f.geometry}));
    if(!roads.length&&!this.map.isSourceLoaded('protomaps')){this.report({reason:'loading'});return;}
    const id=++this.id;this.report({reason:'working'});
    try{this.worker=new RoadDensityWorker();}catch{this.report({reason:'error'});return;}
    this.worker.onmessage=event=>{if(event.data.id!==this.id||!this.enabled)return;(this.map.getSource('road-density') as GeoJSONSource).setData(event.data.collection);this.report(event.data);this.worker?.terminate();this.worker=null;};
    this.worker.onerror=()=>{if(id===this.id){this.report({reason:'error'});this.worker?.terminate();this.worker=null;}};
    this.worker.postMessage({id,roads,points:this.data.features,bounds:this.bbox,reach:this.reach});
  }
  destroy(){this.cancel();this.map.off('sourcedata',this.onSource);}
}
