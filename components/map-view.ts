import type { Map } from 'maplibre-gl';
import bounds from './municipal-bounds.json' with { type: 'json' };

export type MapBounds = [[number, number], [number, number]];
export const MIN_MAP_ZOOM = 3.5;
export const STATE_BOUNDS: MapBounds = [[-53.2, -25.4], [-44.1, -19.7]];
export const HOME_MUNICIPALITY = '3548906';
export function municipalityBounds(code: string): MapBounds | undefined {
    const box = (bounds as Record<string, number[]>)[code];
    return box ? [[box[0], box[1]], [box[2], box[3]]] : undefined;
}
export const HOME_BOUNDS = municipalityBounds(HOME_MUNICIPALITY)!;
export const frameOptions = () => ({ padding: { top: 24, right: 24, bottom: 40, left: 24 }, maxZoom: 12 });
export function fitFrame(map: Map, box: MapBounds, duration = 0) {
    const container = map.getContainer();
    if (container.clientWidth && container.clientHeight) map.fitBounds(box, { ...frameOptions(), duration });
}
