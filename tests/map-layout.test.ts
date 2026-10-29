import {test} from 'node:test';
import assert from 'node:assert/strict';
import {gridResolution,groupBounds,layoutGroups,displayResolution} from '../components/map-layout.ts';

test('narrow and wide maps keep enough space for readable aggregate symbols',()=>{
  for(const [width,height] of [[356,460],[768,420],[1083,420],[1440,480]]){
    const grid=gridResolution(width,height);
    assert.ok(width/grid>=88&&height/grid>=88);
  }
});
test('precise-point requests retain readable group cells if the API exceeds its point limit',()=>{
  for(const [width,height] of [[356,512],[1440,530],[2826,1140]]){
    for(const mode of ['groups','points','roads']){
      const grid=displayResolution(mode,width,height);
      assert.ok(width/grid>=88&&height/grid>=88);
    }
    assert.ok(displayResolution('density',width,height)>displayResolution('groups',width,height));
  }
});
test('dense aggregate cells do not snap their centroids into artificial rows or columns',()=>{
  const original={features:[{geometry:{type:'Point',coordinates:[2.1,7.8]},properties:{count:80,grid:40,cell:0,bbox:'0,0,400,400'}}]};
  const result=layoutGroups(original,([x,y])=>({x,y}),({x,y})=>({lng:x,lat:y}));
  assert.deepEqual(result.features[0].geometry.coordinates,[2.1,7.8]);
  assert.equal(result.features[0].properties.count,80);
});
test('adjacent groups with almost identical centroids remain separately clickable without losing counts',()=>{
  const features=[];
  for(let cell=0;cell<16;cell++)features.push({type:'Feature',geometry:{type:'Point',coordinates:[(cell%4+1)*100-.01,(Math.floor(cell/4)+1)*100-.01]},properties:{count:1500+cell,grid:4,cell,bbox:'0,0,400,400'}});
  const original={type:'FeatureCollection',features};
  const laid=layoutGroups(original,([x,y])=>({x,y}),({x,y})=>({lng:x,lat:y}));
  assert.equal(laid.features.length,original.features.length);
  assert.equal(laid.features.reduce((s:number,f:any)=>s+f.properties.count,0),features.reduce((s,f)=>s+f.properties.count,0));
  for(let i=0;i<laid.features.length;i++)for(let j=i+1;j<laid.features.length;j++){
    const a=laid.features[i],b=laid.features[j];
    assert.ok(Math.hypot(a.geometry.coordinates[0]-b.geometry.coordinates[0],a.geometry.coordinates[1]-b.geometry.coordinates[1])>a.properties.hit_radius+b.properties.hit_radius);
  }
  assert.deepEqual(features[0].geometry.coordinates,[99.99,99.99]);
});
test('individual coordinates and coincident events are preserved exactly',()=>{
  const features=[1,2].map(id=>({type:'Feature',geometry:{type:'Point',coordinates:[-47.2724912,-23.1807199]},properties:{id,count:1}}));
  const result=layoutGroups({features},([x,y])=>({x,y}),({x,y})=>({lng:x,lat:y}));
  assert.equal(result.features.length,2);
  result.features.forEach((f:any)=>assert.deepEqual(f.geometry.coordinates,features[0].geometry.coordinates));
});
test('last cell bounds retain the exact upper boundary',()=>{
  assert.deepEqual(groupBounds({grid:4,cell:15,bbox:'0,0,400,400'}),[300,300,400,400]);
});
