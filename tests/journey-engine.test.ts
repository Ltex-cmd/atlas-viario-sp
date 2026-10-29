import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
const binary = readFileSync(new URL('../worker/engine.wasm', import.meta.url));
const degree = Math.PI * 6371008.8 / 180;
test('C/Wasm counts a route corridor, including coincident events and endpoint caps, without counting the bounding rectangle', async () => {
    const module = await WebAssembly.compile(new Uint8Array(binary).buffer);
    const instance = await WebAssembly.instantiate(module, { wasi_snapshot_preview1: { proc_exit() { throw Error('C failed'); } } });
    const e: any = instance.exports; e._initialize?.();
    const x = degree * Math.cos(-22 * Math.PI / 180), y = degree;
    const points = [
        [-47.89, -22], [-47.89, -22 + 49.9 / y], [-47.89, -22 + 50.1 / y],
        [-47.89, -22], [-47.88 + 49.9 / x, -22], [-47.88 + 50.1 / x, -22],
        [-47.89, -22], [-47.89, -22], [-47.89, -22 + 400 / y],
    ];
    const records = e.allocate(points.length * 48); let view = new DataView(e.memory.buffer);
    new Uint8Array(e.memory.buffer, records, points.length * 48).fill(0);
    points.forEach(([lon, lat], i) => {
        const p = records + i * 48;
        view.setFloat64(p, lat, true); view.setFloat64(p + 8, lon, true);
        view.setUint32(p + 16, i + 1, true); view.setUint32(p + 20, 3548906, true);
        view.setUint32(p + 24, i === 7 ? 20250101 : 20260101 + i, true);
        view.setUint8(p + 42, i === 6 ? 0 : 1);
    });
    const db = e.engine_create(records, points.length); assert.ok(db);
    const path = [[-47.9, -22], [-47.88, -22], [-47.88, -22], [-47.9, -22]];
    const route = e.allocate(path.length * 16);
    view = new DataView(e.memory.buffer);
    path.forEach(([lon, lat], i) => { view.setFloat64(route + i * 16, lon, true); view.setFloat64(route + i * 16 + 8, lat, true); });
    const setFilter = (method: number) => {
        const p = e.query_filter(); view = new DataView(e.memory.buffer);
        [20260101, 20261231, 0, -1, -1, -1, -1, 1, method, 16].forEach((n, i) => view.setInt32(p + i * 4, n, true));
        [-48, -22.1, -47.8, -21.9].forEach((n, i) => view.setFloat64(p + 40 + i * 8, n, true));
        view.setInt32(p + 72, 0, true); view.setInt32(p + 76, 0, true);
    };
    try {
        for (let method = 0; method < 5; method++) {
            setFilter(method); assert.equal(e.query_set_route(route, path.length, 50), 1);
            e.run_query(db, 1); view = new DataView(e.memory.buffer);
            assert.equal(view.getUint32(e.query_result(), true), 4, `method ${method}`);
            assert.equal(e.run_select(db, 0, 30), 4);
            assert.equal(e.query_items_total(), 4);
            view = new DataView(e.memory.buffer);
            assert.deepEqual(Array.from({ length: 4 }, (_, i) => view.getUint32(e.query_items() + i * 48 + 16, true)), [1, 2, 4, 5]);
            assert.equal(e.run_select(db, 2, 1), 1); view = new DataView(e.memory.buffer);
            assert.equal(view.getUint32(e.query_items() + 16, true), 4);
        }
        assert.equal(e.query_set_route(route, path.length, 100), 1);
        e.run_query(db, 1); view = new DataView(e.memory.buffer);
        assert.equal(view.getUint32(e.query_result(), true), 6);
        assert.equal(e.query_set_route(0, 0, 0), 1);
        e.run_query(db, 1); view = new DataView(e.memory.buffer);
        assert.equal(view.getUint32(e.query_result(), true), 7, 'ordinary queries do not retain the previous corridor');
        assert.equal(e.query_set_route(route, 1, 50), 0);
    } finally { e.query_set_route(0, 0, 0); e.engine_destroy(db); e.release(records); e.release(route); }
});
