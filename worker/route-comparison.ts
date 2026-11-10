import polygons from './municipal-polygons.json';
import network from './road-network.json';
import { buildComparison, routeLengthByMunicipality, type Rings } from '../lib/route-comparison';
import { routeLength, type Coordinate } from '../lib/journey';

export function prepareComparison(coordinates: Coordinate[], city: number) {
    const lengths = routeLengthByMunicipality(coordinates, polygons as Record<string, Rings>);
    const selected = Object.entries(lengths.metres).filter(([code, metres]) => metres >= 1 && (!city || Number(code) === city)).sort((a, b) => b[1] - a[1]);
    return { lengths, selected: selected.slice(0, 40), counts: {} as Record<string, number>, route_m: routeLength(coordinates), outsideFilter: !!city && !selected.length };
}
export type ComparisonContext = ReturnType<typeof prepareComparison>;
export function finishComparison(context: ComparisonContext, observed: number, corridor_m: number, cities: { code: number; name: string }[]) {
    return buildComparison(context, observed, corridor_m, cities, network.municipalities);
}