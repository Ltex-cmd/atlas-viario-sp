import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { municipalityBounds, HOME_BOUNDS, STATE_BOUNDS } from '../components/map-view.ts';

test('every municipality in the source has a finite IBGE frame, independent of accident availability', () => {
    const metadata = JSON.parse(readFileSync(new URL('../public/data/metadata.json', import.meta.url), 'utf8'));
    assert.equal(metadata.cities.length, 645);
    for (const city of metadata.cities) {
        const box = municipalityBounds(String(city.code));
        assert.ok(box, city.name);
        const [[west, south], [east, north]] = box;
        assert.ok([west, south, east, north].every(Number.isFinite));
        assert.ok(west < east && south < north && west > -54 && east < -43 && south > -26 && north < -19);
    }
    assert.deepEqual(HOME_BOUNDS, municipalityBounds('3548906'));
    assert.notDeepEqual(HOME_BOUNDS, STATE_BOUNDS);
    assert.equal(municipalityBounds('invalid'), undefined);
});
