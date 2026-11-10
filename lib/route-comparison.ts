import { METRES_PER_DEGREE } from './journey.ts';

export type ComparisonLevel = 'below' | 'near' | 'critical_90' | 'critical_95' | 'critical_995';
export type Rings = number[][][];

export function pointInRings(lon: number, lat: number, rings: Rings): boolean {
    const inside = (ring: number[][]) => {
        let hit = false;
        for (let i = 0, j = ring.length - 1; i < ring.length; j = i++) {
            const a = ring[i], b = ring[j];
            if ((a[1] > lat) !== (b[1] > lat) && lon < (b[0] - a[0]) * (lat - a[1]) / (b[1] - a[1]) + a[0]) hit = !hit;
        }
        return hit;
    };
    return rings.reduce((hit, ring) => hit !== inside(ring), false);
}

export function routeLengthByMunicipality(coordinates: [number, number][], polygons: Record<string, Rings>): { metres: Record<string, number>; outside_m: number } {
    const entries = Object.entries(polygons).map(([code, rings]) => {
        let west = Infinity, east = -Infinity, south = Infinity, north = -Infinity;
        for (const [lon, lat] of rings.flat()) {
            west = Math.min(west, lon); east = Math.max(east, lon);
            south = Math.min(south, lat); north = Math.max(north, lat);
        }
        return { code, rings, west, east, south, north };
    });
    const metres: Record<string, number> = {};
    let outside_m = 0, previous: typeof entries[number] | undefined;
    for (let i = 1; i < coordinates.length; i++) {
        const a = coordinates[i - 1], b = coordinates[i];
        const length = Math.hypot((b[0] - a[0]) * Math.cos((a[1] + b[1]) * Math.PI / 360), b[1] - a[1]) * METRES_PER_DEGREE;
        const pieces = Math.ceil(length / 50);
        for (let j = 0; j < pieces; j++) {
            const t = (j + 0.5) / pieces, lon = a[0] + (b[0] - a[0]) * t, lat = a[1] + (b[1] - a[1]) * t;
            const contains = (p: typeof entries[number]) => lon >= p.west && lon <= p.east && lat >= p.south && lat <= p.north && pointInRings(lon, lat, p.rings);
            const municipality = previous && contains(previous) ? previous : entries.find(contains);
            if (municipality) metres[municipality.code] = (metres[municipality.code] || 0) + length / pieces;
            else outside_m += length / pieces;
            previous = municipality;
        }
    }
    return { metres, outside_m };
}

function logFactorial(n: number): number {
    if (n < 256) {
        let result = 0;
        for (let i = 2; i <= n; i++) result += Math.log(i);
        return result;
    }
    // Stirling com correções até n^-7; erro abaixo da precisão de double neste intervalo.
    return (n + 0.5) * Math.log(n) - n + 0.5 * Math.log(2 * Math.PI) + 1 / (12 * n) - 1 / (360 * n ** 3) + 1 / (1260 * n ** 5) - 1 / (1680 * n ** 7);
}

export function poissonUpperTail(n: number, mean: number): number {
    if (n <= 0) return 1;
    if (mean === 0) return 0;
    if (!Number.isFinite(mean) || mean < 0 || !Number.isFinite(n)) return 0;
    n = Math.ceil(n);
    let logTerm = -mean + n * Math.log(mean) - logFactorial(n), logSum = logTerm;
    // Log-sum-exp evita perder a cauda quando o primeiro termo subflui antes da moda.
    for (let i = n; ; i++) {
        logTerm += Math.log(mean / (i + 1));
        const high = Math.max(logSum, logTerm);
        logSum = high + Math.log1p(Math.exp(Math.min(logSum, logTerm) - high));
        if (i + 1 > mean && logTerm - logSum < Math.log(1e-17)) break;
    }
    return Math.max(0, Math.min(1, Math.exp(logSum)));
}

export function classifyRoute(observed: number, expected: number): {
    level: ComparisonLevel; ratio: number; p_value: number;
    critical_counts: { c90: number; c95: number; c995: number };
} | null {
    if (!Number.isFinite(expected) || expected <= 0) return null;
    const p_value = poissonUpperTail(observed, expected);
    const level: ComparisonLevel = observed < expected ? 'below' : observed < 3 ? 'near' : p_value <= 0.005 ? 'critical_995' : p_value <= 0.05 ? 'critical_95' : p_value <= 0.10 ? 'critical_90' : 'near';
    const critical = (k: number) => expected + k * Math.sqrt(expected) - 0.5;
    return { level, ratio: observed / expected, p_value, critical_counts: { c90: critical(1.282), c95: critical(1.645), c995: critical(2.576) } };
}

export type ComparisonContext = {
    lengths: { metres: Record<string, number>; outside_m: number };
    selected: [string, number][];
    counts: Record<string, number>;
    route_m: number;
    outsideFilter: boolean;
};
const round = (n: number) => Math.round(n * 100) / 100;
export function buildComparison(context: ComparisonContext, observed: number, corridor_m: number, cities: { code: number; name: string }[], network: Record<string, { main_km: number; local_km: number }>) {
    const roads = network;
    const missing_reference: number[] = [];
    let expected = 0;
    const municipalities = context.selected.flatMap(([code, metres]) => {
        const road = roads[code], network_km = road ? road.main_km + road.local_km : 0;
        if (!(network_km > 0)) { missing_reference.push(Number(code)); return []; }
        const reference_events = context.counts[code] || 0, density = reference_events / network_km, contribution = metres / 1000 * density;
        expected += contribution;
        return [{ code: Number(code), name: cities.find(c => String(c.code) === code)?.name || code, route_km: round(metres / 1000), network_km: round(network_km), reference_events, density_per_km: round(density), expected: round(contribution) }];
    });
    const common = { observed, route_km: round(context.route_m / 1000), outside_km: round(context.lengths.outside_m / 1000), corridor_m, municipalities, missing_reference };
    const classification = classifyRoute(observed, expected);
    // Uma referência parcial não sustenta um veredito para toda a contagem observada.
    const reason = context.outsideFilter ? 'outside_filter' : missing_reference.length || !municipalities.length ? 'no_reference' : !classification ? 'no_events' : null;
    if (reason || !classification) return { ...common, status: 'unavailable' as const, reason, level: null };
    return { ...common, status: 'ok' as const, reason: null, expected: round(expected), level: classification.level, ratio: round(classification.ratio), p_value: Number(classification.p_value.toPrecision(4)), critical_counts: { c90: round(classification.critical_counts.c90), c95: round(classification.critical_counts.c95), c995: round(classification.critical_counts.c995) } };
}
