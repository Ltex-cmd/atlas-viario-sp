import GeoJSONReader from 'jsts/org/locationtech/jts/io/GeoJSONReader.js';
import GeoJSONWriter from 'jsts/org/locationtech/jts/io/GeoJSONWriter.js';
import GeometryFactory from 'jsts/org/locationtech/jts/geom/GeometryFactory.js';
import BufferOp from 'jsts/org/locationtech/jts/operation/buffer/BufferOp.js';
import type { Feature, Polygon, MultiPolygon, Position } from 'geojson';
import { METRES_PER_DEGREE, routeLatitude, validateRoute, type Coordinate } from './journey.ts';

/** A single dissolved surface, using the same local metric projection as C/Wasm.
 * Round joins/end caps approximate the distance-to-segment predicate. With 32
 * segments per quadrant the maximum arc error is below 3.1 cm at a 100 m radius.
 * The route stays intact: this polygon is only for display, never for counting.
 */
export function journeyCorridor(coordinates: Coordinate[], radius: number): Feature<Polygon | MultiPolygon> {
    validateRoute(coordinates);
    if (!Number.isFinite(radius) || radius <= 0 || radius > 100) throw Error('Faixa de proximidade inválida.');
    const scaleY = METRES_PER_DEGREE, scaleX = scaleY * Math.cos(routeLatitude(coordinates) * Math.PI / 180);
    const [originX, originY] = coordinates[0];
    const projected = coordinates.map(([lon, lat]) => [(lon - originX) * scaleX, (lat - originY) * scaleY]);
    const line = new GeoJSONReader(new GeometryFactory()).read({ type: 'LineString', coordinates: projected });
    const geometry = new GeoJSONWriter().write(BufferOp.bufferOp(line, radius, 32)) as Polygon | MultiPolygon;
    const unproject = ([x, y]: Position): Position => [originX + x / scaleX, originY + y / scaleY];
    if (geometry.type === 'Polygon') geometry.coordinates = geometry.coordinates.map(ring => ring.map(unproject));
    else if (geometry.type === 'MultiPolygon') geometry.coordinates = geometry.coordinates.map(polygon => polygon.map(ring => ring.map(unproject)));
    else throw Error('Não foi possível desenhar a faixa de proximidade.');
    return { type: 'Feature', properties: {}, geometry };
}
