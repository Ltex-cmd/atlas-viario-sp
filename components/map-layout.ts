type Coordinate = [number, number];
type Pixel = {x:number;y:number};

/** Size the server's aggregation grid for the actual map, including narrow phones. */
export function gridResolution(width:number,height:number){
  return Math.max(2,Math.min(64,Math.floor(Math.min(width,height)/88)));
}

/** Only the free heatmap needs a fine grid; drawable groups need readable cells. */
export function displayResolution(mode:string,width:number,height:number){
  return mode==='density'?Math.max(16,Math.min(64,Math.floor(Math.min(width,height)/14))):gridResolution(width,height);
}

export function groupBounds(properties:any):[number,number,number,number]|null{
  const b=String(properties.bbox).split(',').map(Number);
  const grid=Number(properties.grid),cell=Number(properties.cell);
  if(b.length!==4||b.some(n=>!Number.isFinite(n))||!Number.isInteger(grid)||grid<1||
    !Number.isInteger(cell)||cell<0||cell>=grid*grid)return null;
  const x=cell%grid,y=Math.floor(cell/grid),dx=(b[2]-b[0])/grid,dy=(b[3]-b[1])/grid;
  return [b[0]+x*dx,b[1]+y*dy,b[0]+(x+1)*dx,b[1]+(y+1)*dy];
}

const compact=new Intl.NumberFormat('pt-BR',{notation:'compact',maximumFractionDigits:1});
const clamp=(value:number,lo:number,hi:number)=>lo>hi?(lo+hi)/2:Math.max(lo,Math.min(hi,value));

/** Keep aggregate symbols inside their cell; individual official coordinates are untouched. */
export function layoutGroups(collection:any,project:(p:Coordinate)=>Pixel,unproject:(p:Pixel)=>{lng:number;lat:number}){
  return {...collection,features:collection.features.map((feature:any)=>{
    const p=feature.properties,count=Number(p.count),bounds=count>1?groupBounds(p):null;
    let radius=4.5,coordinates=feature.geometry.coordinates;
    if(bounds){
      const a=project([bounds[0],bounds[1]]),b=project([bounds[2],bounds[3]]);
      const left=Math.min(a.x,b.x),right=Math.max(a.x,b.x),top=Math.min(a.y,b.y),bottom=Math.max(a.y,b.y);
      radius=Math.max(3,Math.min(20,Math.max(12,10+Math.log10(count)*2.5),(right-left)/2-7,(bottom-top)/2-7));
      const original=project(coordinates),inset=radius+7;
      // A cell narrower than the symbol cannot accommodate an inset. Clamping
      // that inverted interval would snap every centroid onto the grid center.
      if(right-left>=2*inset&&bottom-top>=2*inset){
        const center=unproject({x:clamp(original.x,left+inset,right-inset),y:clamp(original.y,top+inset,bottom-inset)});
        coordinates=[center.lng,center.lat];
      }
    }
    return {...feature,geometry:{...feature.geometry,coordinates},properties:{...p,radius,hit_radius:Math.max(11,radius+2),label:count>1&&radius>=12?compact.format(count):''}};
  })};
}
