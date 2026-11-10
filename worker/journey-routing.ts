import {
    validCoordinate, validateRoute, JOURNEY_MAX_METRES, JOURNEY_SNAP_RADIUS_M,
    JOURNEY_EXPANDED_SNAP_RADIUS_M, journeyNeedsConfirmation,
    type Coordinate, type TravelMode, type Journey,
} from '../lib/journey.ts';
import { JourneyError } from './journey-response.ts';

export const routingProfile: Record<TravelMode, string> = { foot: 'routed-foot', bike: 'routed-bike', car: 'routed-car' };
type RequestRoute = (url: URL, mode: TravelMode, radius: number) => Promise<any>;

/** Retry a disconnected nearest street against the wider connected network.
 * Only one wider request, same provider/mode/coordinates, still distance-bounded.
 * Large adjustments are returned for review, never added to the route geometry.
 */
export async function requestConnectedJourney(from: Coordinate, to: Coordinate, mode: TravelMode, request: RequestRoute): Promise<Journey> {
    const get = (radius: number) => {
        const url = new URL(`https://routing.openstreetmap.de/${routingProfile[mode]}/route/v1/driving/${from.join(',')};${to.join(',')}`);
        url.search = new URLSearchParams({ geometries: 'geojson', overview: 'full', steps: 'false', alternatives: 'false', radiuses: `${radius};${radius}` }).toString();
        return request(url, mode, radius);
    };
    let radius = JOURNEY_SNAP_RADIUS_M, data: any;
    try { data = await get(radius); }
    catch (error) {
        if (!(error instanceof JourneyError) || !['NoRoute', 'NoSegment'].includes(error.code ?? '')) throw error;
        radius = JOURNEY_EXPANDED_SNAP_RADIUS_M;
        data = await get(radius);
    }
    if (data?.code !== 'Ok' || !data.routes?.length) throw new JourneyError('O serviço de rotas retornou uma resposta incompleta.', 503);
    const r = data.routes[0];
    if (!Number.isFinite(r.distance) || !Number.isFinite(r.duration) || r.distance < 0 || r.duration < 0) throw new JourneyError('O serviço de rotas retornou uma resposta incompleta.', 503);
    if (r.distance > JOURNEY_MAX_METRES) throw new JourneyError('Este teste aceita percursos de até 100 km. Escolha pontos mais próximos.');
    const coordinates = validateRoute(r.geometry?.coordinates);
    const waypoints = (data.waypoints || []).map((w: any) => ({ coordinates: w.location, distance_m: w.distance, name: String(w.name || '') }));
    if (waypoints.length !== 2 || waypoints.some((w: any) => !validCoordinate(w.coordinates) || !Number.isFinite(w.distance_m) || w.distance_m < 0 || w.distance_m > radius))
        throw new JourneyError('Não foi possível associar os pontos às vias dentro da distância permitida. Escolha novamente no mapa.', 422);
    const journey: Journey = { coordinates, distance_m: r.distance, duration_s: r.duration, mode, waypoints };
    journey.snapping = { search_radius_m: radius, expanded_search: radius > JOURNEY_SNAP_RADIUS_M, requires_confirmation: journeyNeedsConfirmation(journey) };
    return journey;
}
