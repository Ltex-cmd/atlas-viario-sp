import { validCoordinate, validateRoute, JOURNEY_MAX_METRES, type Coordinate, type TravelMode, type Journey, type Place } from '../lib/journey';

import { JourneyError, readProviderResponse } from './journey-response';
export { JourneyError } from './journey-response';
const profile: Record<TravelMode, string> = { foot: 'routed-foot', bike: 'routed-bike', car: 'routed-car' };
const identity = 'AtlasViarioAcademic/1.0 (+https://ltex-cmd.github.io/atlas-viario-sp/)';
const cache = new Map<string, { expires: number; data: unknown }>();
let nextRequest = 0;
let pending = 0;
// The private demonstration sends requests only after explicit user actions.
// Serialise all provider requests in this isolate, including address searches.
async function upstream(url: URL, mode?: TravelMode) {
    const cached = cache.get(url.href);
    if (cached && cached.expires > Date.now()) return cached.data as any;
    if (pending >= 6) throw new JourneyError('Há consultas em andamento. Aguarde alguns segundos e tente novamente.', 429);
    const scheduled = Math.max(Date.now(), nextRequest);
    nextRequest = scheduled + 1100;
    pending++;
    try {
        const delay = scheduled - Date.now();
        if (delay > 0) await new Promise(resolve => setTimeout(resolve, delay));
        let response: Response;
        try { response = await fetch(url, { headers: { 'User-Agent': identity, 'Accept': 'application/json', 'Referer': 'https://ltex-cmd.github.io/atlas-viario-sp/' }, signal: AbortSignal.timeout(20_000) }); }
        catch { throw new JourneyError(mode ? 'O serviço de rotas não respondeu. Tente traçar o percurso novamente.' : 'O serviço de busca não respondeu. Tente novamente ou escolha os pontos no mapa.', 503); }
        const data = await readProviderResponse(response, mode);
        if (cache.size >= 64) cache.delete(cache.keys().next().value!);
        cache.set(url.href, { data, expires: Date.now() + 15 * 60_000 });
        return data;
    } finally { pending--; }
}
export async function searchPlaces(params: URLSearchParams): Promise<{ places: Place[] }> {
    const q = params.get('q')?.trim() || '';
    if (q.length < 3 || q.length > 160) throw new JourneyError('Escreva uma rua ou um lugar com 3 a 160 caracteres.');
    const lon = Number(params.get('lon') || '-47.89'), lat = Number(params.get('lat') || '-22.01');
    if (!validCoordinate([lon, lat])) throw new JourneyError('Local de busca inválido.');
    const url = new URL('https://photon.komoot.io/api/');
    url.search = new URLSearchParams({ q, limit: '6', lon: String(lon), lat: String(lat), bbox: '-54,-26,-43,-19' }).toString();
    const data = await upstream(url);
    if (!Array.isArray(data.features)) throw new JourneyError('O serviço de busca retornou uma resposta incompleta.', 503);
    const places: Place[] = [];
    for (const f of data.features) {
        if (!validCoordinate(f.geometry?.coordinates)) continue;
        const p = f.properties || {};
        const street = [p.street, p.housenumber].filter(Boolean).join(', ');
        const label = [...new Set([p.name, street, p.city || p.district, p.state].filter(Boolean))].join(' · ');
        if (label) places.push({ label, coordinates: f.geometry.coordinates });
    }
    return { places };
}
export async function requestJourney(params: URLSearchParams): Promise<Journey> {
    const point = (name: string): Coordinate => {
        const raw = params.get(name) || '';
        const values = raw.split(',').map(Number);
        if (!/^[-\d.,]+$/.test(raw) || !validCoordinate(values)) throw new JourneyError('Escolha uma origem e um destino válidos no mapa ou na busca.');
        return values;
    };
    const from = point('from'), to = point('to');
    const mode = params.get('mode') || 'foot';
    if (!Object.hasOwn(profile, mode)) throw new JourneyError('Modo de transporte inválido.');
    const url = new URL(`https://routing.openstreetmap.de/${profile[mode as TravelMode]}/route/v1/driving/${from.join(',')};${to.join(',')}`);
    url.search = new URLSearchParams({ geometries: 'geojson', overview: 'full', steps: 'false', alternatives: 'false', radiuses: '500;500' }).toString();
    const data = await upstream(url, mode as TravelMode);
    if (data.code !== 'Ok' || !data.routes?.length) throw new JourneyError('Não foi encontrado um caminho entre esses pontos. Escolha pontos mais próximos das ruas.', 422);
    const r = data.routes[0];
    if (!Number.isFinite(r.distance) || !Number.isFinite(r.duration) || r.distance < 0 || r.duration < 0) throw new JourneyError('O serviço de rotas retornou uma resposta incompleta.', 503);
    if (r.distance > JOURNEY_MAX_METRES) throw new JourneyError('Este teste aceita percursos de até 100 km. Escolha pontos mais próximos.');
    const coordinates = validateRoute(r.geometry?.coordinates);
    const waypoints = (data.waypoints || []).map((w: any) => ({ coordinates: w.location, distance_m: w.distance, name: String(w.name || '') }));
    if (waypoints.length !== 2 || waypoints.some((w: any) => !validCoordinate(w.coordinates) || !Number.isFinite(w.distance_m) || w.distance_m > 500)) throw new JourneyError('Não foi possível associar os pontos às ruas. Escolha novamente no mapa.', 422);
    return { coordinates, distance_m: r.distance, duration_s: r.duration, mode: mode as TravelMode, waypoints };
}
export async function readJourneyBody(request: Request): Promise<any> {
    if (!request.headers.get('Content-Type')?.includes('application/json')) throw new JourneyError('Envie o percurso em JSON.', 415);
    if (!request.body) throw new JourneyError('Percurso ausente.');
    const reader = request.body.getReader(), parts: Uint8Array[] = [];
    let size = 0;
    while (true) {
        const { done, value } = await reader.read();
        if (done) break;
        size += value.byteLength;
        if (size > 512_000) { await reader.cancel(); throw new JourneyError('Percurso muito extenso. Escolha pontos mais próximos.', 413); }
        parts.push(value);
    }
    const bytes = new Uint8Array(size); let position = 0;
    for (const part of parts) { bytes.set(part, position); position += part.length; }
    try { return JSON.parse(new TextDecoder().decode(bytes)); } catch { throw new JourneyError('Percurso inválido.'); }
}
