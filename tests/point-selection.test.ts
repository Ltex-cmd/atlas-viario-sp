import { test } from 'node:test';
import assert from 'node:assert/strict';
import { resolvePointSelection, loadPointSelection } from '../components/point-selection.ts';
const feature = (id: number, coords: number[]) => ({ type: 'Feature', properties: { id, count: 1 }, geometry: { type: 'Point', coordinates: coords } });

test('overlapping hits use official coordinates despite vector-tile rounding', () => {
    const original = [-47.9294280806, -21.977170512];
    const selected = resolvePointSelection([feature(2753363, [-47.92944, -21.97718]), feature(2751226, [-47.92944, -21.97718])], { features: [feature(2753363, original), feature(2751226, original)] })!;
    const [west, south, east, north] = selected.bbox.split(',').map(Number);
    assert.ok(west < original[0] && east > original[0] && south < original[1] && north > original[1]);
    assert.deepEqual(selected.ids, [2753363, 2751226]);
});
test('duplicate tile hits select one real ID and stale hits do not invent coordinates', () => {
    const point = feature(9, [-47.9, -22]);
    assert.equal(resolvePointSelection([point, point], { features: [point] })?.count, 1);
    assert.equal(resolvePointSelection([point], { features: [] }), null);
});
test('selected IDs exclude nearby unclicked records and survive multiple result pages', async () => {
    const calls: number[] = [];
    const items = await loadPointSelection([1, 3], async offset => {
        calls.push(offset);
        return offset === 0 ? { items: [{ id: 2 }, { id: 1 }], has_more: true } : { items: [{ id: 3 }, { id: 4 }], has_more: false };
    });
    assert.deepEqual(items.map(item => item.id), [1, 3]);
    assert.deepEqual(calls, [0, 2]);
    await assert.rejects(loadPointSelection([99], async () => ({ items: [], has_more: false })), /Não foi possível/);
});
