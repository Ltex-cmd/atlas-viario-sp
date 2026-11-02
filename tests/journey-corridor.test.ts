import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import type { Polygon, MultiPolygon, Position } from 'geojson';
import { journeyCorridor } from '../lib/journey-corridor.ts';
import { METRES_PER_DEGREE, routeLatitude, type Coordinate } from '../lib/journey.ts';

const scaleY = METRES_PER_DEGREE, scaleX = scaleY * Math.cos(-22 * Math.PI / 180);
const at = ([x, y]: number[]): Coordinate => [-47.89 + x / scaleX, -22 + y / scaleY];
function insideRing([x, y]: Coordinate, ring: Position[]) {
    let inside = false;
    for (let i = 0, j = ring.length - 1; i < ring.length; j = i++) {
        const [xi, yi] = ring[i], [xj, yj] = ring[j];
        if ((yi > y) !== (yj > y) && x < (xj - xi) * (y - yi) / (yj - yi) + xi) inside = !inside;
    }
    return inside;
}
function inside(point: Coordinate, geometry: Polygon | MultiPolygon) {
    const polygons = geometry.type === 'Polygon' ? [geometry.coordinates] : geometry.coordinates;
    return polygons.some(([shell, ...holes]) => insideRing(point, shell) && !holes.some(hole => insideRing(point, hole)));
}
function area(geometry: Polygon | MultiPolygon) {
    const polygons = geometry.type === 'Polygon' ? [geometry.coordinates] : geometry.coordinates;
    const ringArea = (ring: Position[]) => Math.abs(ring.reduce((sum, p, i) => {
        const q = ring[(i + 1) % ring.length];
        return sum + (p[0] + 47.89) * (q[1] + 22) - (q[0] + 47.89) * (p[1] + 22);
    }, 0)) * scaleX * scaleY / 2;
    return polygons.reduce((total, [shell, ...holes]) => total + ringArea(shell) - holes.reduce((a, h) => a + ringArea(h), 0), 0);
}

test('retraced segments and repeated vertices yield one surface, without multiplying its area', () => {
    const direct = [at([-500, 0]), at([500, 0])];
    const retraced = [...direct, direct[1], direct[0], direct[1], direct[0]];
    const original = structuredClone(retraced);
    for (const radius of [25, 50, 100]) {
        const straight = journeyCorridor(direct, radius).geometry, repeated = journeyCorridor(retraced, radius).geometry;
        assert.equal(repeated.type, 'Polygon');
        assert.ok(Math.abs(area(straight) - area(repeated)) < 0.001);
        assert.ok(Math.abs(area(repeated) - (2000 * radius + Math.PI * radius ** 2)) / area(repeated) < 0.0001);
        assert.equal(inside(at([500 + radius - 0.1, 0]), repeated), true, 'round cap includes the endpoint distance');
        assert.equal(inside(at([500 + radius + 0.1, 0]), repeated), false);
    }
    assert.deepEqual(retraced, original, 'display geometry must not modify the counted route');
});

test('closed loops retain their unselected interior; crossing arms form one filled surface', () => {
    const loop = [[-300, -300], [300, -300], [300, 300], [-300, 300], [-300, -300]].map(at);
    const geometry = journeyCorridor(loop, 100).geometry;
    assert.equal(geometry.type, 'Polygon');
    assert.equal(geometry.coordinates.length, 2, 'one shell and one hole');
    assert.equal(inside(at([0, 0]), geometry), false);
    assert.equal(inside(at([290, 0]), geometry), true);
    const cross = [[-300, -300], [300, 300], [-300, 300], [300, -300]].map(at);
    assert.equal(inside(at([0, 0]), journeyCorridor(cross, 50).geometry), true);
});

test('visual coverage agrees with the real C/Wasm corridor at 25, 50 and 100 m, including bends and overlaps', async () => {
    const binary = readFileSync(new URL('../worker/engine.wasm', import.meta.url));
    const instance = await WebAssembly.instantiate(await WebAssembly.compile(new Uint8Array(binary).buffer), { wasi_snapshot_preview1: { proc_exit() { throw Error('C failed'); } } });
    const e: any = instance.exports; e._initialize?.();
    const points: Coordinate[] = [];
    for (let x = -400; x <= 400; x += 40) for (let y = -400; y <= 400; y += 40) points.push(at([x + 7.3, y + 3.7]));
    const records = e.allocate(points.length * 48); let view = new DataView(e.memory.buffer);
    new Uint8Array(e.memory.buffer, records, points.length * 48).fill(0);
    points.forEach(([lon, lat], i) => {
        const p = records + i * 48;
        view.setFloat64(p, lat, true); view.setFloat64(p + 8, lon, true);
        view.setUint32(p + 16, i + 1, true); view.setUint32(p + 24, 20260101, true); view.setUint8(p + 42, 1);
    });
    const db = e.engine_create(records, points.length); assert.ok(db);
    const paths = [
        [[-300, -300], [300, 300], [-300, 300], [300, -300]],
        [[-300, -300], [300, -300], [300, 300], [-300, 300], [-300, -300]],
        [[-300, -40], [300, -40], [300, 40], [-300, 40], [-300, 40], [300, 40]],
    ].map(path => path.map(at));
    try {
        for (const path of paths) {
            const ptr = e.allocate(path.length * 16); view = new DataView(e.memory.buffer);
            path.forEach(([lon, lat], i) => { view.setFloat64(ptr + i * 16, lon, true); view.setFloat64(ptr + i * 16 + 8, lat, true); });
            try {
                for (const radius of [25, 50, 100]) {
                    const f = e.query_filter(); view = new DataView(e.memory.buffer);
                    [20260101, 20261231, 0, -1, -1, -1, -1, 1, 0, 16].forEach((n, i) => view.setInt32(f + i * 4, n, true));
                    [-48, -22.1, -47.8, -21.9].forEach((n, i) => view.setFloat64(f + 40 + i * 8, n, true));
                    view.setInt32(f + 72, 0, true); view.setInt32(f + 76, 0, true);
                    assert.equal(e.query_set_route(ptr, path.length, radius), 1);
                    const count = e.run_select(db, 0, points.length);
                    view = new DataView(e.memory.buffer);
                    const selected = new Set(Array.from({ length: count }, (_, i) => view.getUint32(e.query_items() + i * 48 + 16, true)));
                    const geometry = journeyCorridor(path, radius).geometry;
                    points.forEach((point, i) => assert.equal(inside(point, geometry), selected.has(i + 1), `radius ${radius}, point ${i}, latitude ${routeLatitude(path)}`));
                }
            } finally { e.query_set_route(0, 0, 0); e.release(ptr); }
        }
    } finally { e.engine_destroy(db); e.release(records); }
});

test('a stationary route still has a round corridor; invalid input is rejected', () => {
    const point = at([0, 0]);
    const geometry = journeyCorridor([point, point], 25).geometry;
    assert.equal(inside(at([24, 0]), geometry), true);
    assert.equal(inside(at([26, 0]), geometry), false);
    for (const radius of [0, -25, NaN, Infinity, 101]) assert.throws(() => journeyCorridor([point, point], radius));
});
