export type Coordinate = [number, number];
export type TravelMode = 'foot' | 'bike' | 'car';
export type Place = { label: string; coordinates: Coordinate };
export type Journey = {
    coordinates: Coordinate[];
    distance_m: number;
    duration_s: number;
    mode: TravelMode;
    waypoints: { coordinates: Coordinate; distance_m: number; name: string }[];
};
export const METRES_PER_DEGREE = Math.PI * 6371008.8 / 180;
export const JOURNEY_MAX_METRES = 100_000;
export const JOURNEY_MAX_VERTICES = 10_000;
export function validCoordinate(value: unknown): value is Coordinate {
    return Array.isArray(value) && value.length === 2 && value.every(v => typeof v === 'number' && Number.isFinite(v)) && value[0] >= -54 && value[0] <= -43 && value[1] >= -26 && value[1] <= -19;
}
export function routeLength(coordinates: Coordinate[]) {
    let metres = 0;
    for (let i = 1; i < coordinates.length; i++) {
        const a = coordinates[i - 1], b = coordinates[i];
        metres += Math.hypot((b[0] - a[0]) * Math.cos((a[1] + b[1]) * Math.PI / 360), b[1] - a[1]) * METRES_PER_DEGREE;
    }
    return metres;
}
export function validateRoute(value: unknown): Coordinate[] {
    if (!Array.isArray(value) || value.length < 2 || value.length > JOURNEY_MAX_VERTICES || !value.every(validCoordinate))
        throw Error('Percurso inválido. Escolha dois pontos na região de São Paulo.');
    if (routeLength(value) > JOURNEY_MAX_METRES)
        throw Error('Este teste aceita percursos de até 100 km. Escolha pontos mais próximos.');
    return value;
}
export function routeBounds(coordinates: Coordinate[], corridor = 0): [number, number, number, number] {
    let w = Infinity, s = Infinity, e = -Infinity, n = -Infinity;
    for (const [lon, lat] of coordinates) { w = Math.min(w, lon); e = Math.max(e, lon); s = Math.min(s, lat); n = Math.max(n, lat); }
    // At least a metre avoids a zero-width bounding box for horizontal/vertical paths.
    const dy = Math.max(1, corridor) / METRES_PER_DEGREE;
    const dx = dy / Math.cos((s + n) * Math.PI / 360);
    return [w - dx, s - dy, e + dx, n + dy];
}
export function routeLatitude(coordinates: Coordinate[]) {
    const b = routeBounds(coordinates);
    return (b[1] + b[3]) / 2;
}
export function distanceLabel(metres: number) {
    return metres < 1000 ? `${Math.round(metres).toLocaleString('pt-BR')} m` : `${(metres / 1000).toLocaleString('pt-BR', { maximumFractionDigits: 1 })} km`;
}
export function durationLabel(seconds: number) {
    const minutes = Math.max(1, Math.round(seconds / 60));
    return minutes < 60 ? `${minutes} min` : `${Math.floor(minutes / 60)} h${minutes % 60 ? ` ${minutes % 60} min` : ''}`;
}
