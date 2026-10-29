import test from 'node:test';
import assert from 'node:assert/strict';
import { routeBounds, routeLength, validateRoute, validCoordinate } from '../lib/journey.ts';
test('route validation rejects unsupported, nonfinite and excessive geometry', () => {
    for (const point of [[NaN, -22], ['-47.9', -22], [0, 0], [-47.9, -22, 0], null]) assert.equal(validCoordinate(point), false);
    for (const route of [[], [[-47.9, -22]], [[-47.9, -22], [-43, -19]]]) assert.throws(() => validateRoute(route));
    assert.ok(validateRoute([[-47.9, -22], [-47.89, -22]]));
});
test('corridor bounds expand in metres and support horizontal and vertical paths', () => {
    for (const route of [[[-47.9, -22], [-47.89, -22]], [[-47.9, -22], [-47.9, -21.99]]] as [number, number][][]) {
        const b = routeBounds(route, 50);
        assert.ok(b[0] < Math.min(...route.map(p => p[0])) && b[2] > Math.max(...route.map(p => p[0])));
        assert.ok(b[1] < Math.min(...route.map(p => p[1])) && b[3] > Math.max(...route.map(p => p[1])));
        assert.ok(routeLength(route) > 1000 && routeLength(route) < 1200);
    }
});
