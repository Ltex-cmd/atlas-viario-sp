import { test } from 'node:test';
import assert from 'node:assert/strict';
import { applyMapMode } from '../components/map-modes.ts';

const mapStub = () => {
    const visible = new Map<string, unknown>();
    return { visible, map: {
        setLayoutProperty: (layer: string, property: string, value: unknown) => { if (property === 'visibility') visible.set(layer, value); },
        setPaintProperty() {}
    } as unknown as Parameters<typeof applyMapMode>[0] };
};

test('a response above the point limit cannot masquerade as individual events', () => {
    const response = { count: 40070, mode: 'clusters', features: [{ properties: { count: 146 } }, { properties: { count: 1 } }] };
    const before = structuredClone(response), { map, visible } = mapStub();
    applyMapMode(map, 'points', false, response.mode === 'points');
    for (const layer of ['events', 'counts', 'event-hit']) assert.equal(visible.get(layer), 'none');
    assert.deepEqual(response, before);
    applyMapMode(map, 'groups', false, false);
    for (const layer of ['events', 'counts', 'event-hit']) assert.equal(visible.get(layer), 'visible');
});
test('exact points are drawn in Points and when original road points are enabled', () => {
    const { map, visible } = mapStub();
    applyMapMode(map, 'points', false, true);
    assert.equal(visible.get('events'), 'visible');
    applyMapMode(map, 'roads', true, false);
    assert.equal(visible.get('events'), 'none');
    applyMapMode(map, 'roads', true, true);
    assert.equal(visible.get('events'), 'visible');
    applyMapMode(map, 'roads', false, true);
    assert.equal(visible.get('events'), 'none');
});
test('journey exploration hides the general accident layers and returning restores the chosen mode', () => {
    const { map, visible } = mapStub();
    applyMapMode(map, 'groups', true, true, true);
    for (const layer of ['events', 'counts', 'event-hit', 'events-glow', 'density']) assert.equal(visible.get(layer), 'none');
    applyMapMode(map, 'density', false, true, false);
    assert.equal(visible.get('density'), 'visible');
    assert.equal(visible.get('events'), 'none');
});
