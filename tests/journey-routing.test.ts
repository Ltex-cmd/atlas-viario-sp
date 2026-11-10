import test from 'node:test';
import assert from 'node:assert/strict';
import { journeyNeedsConfirmation, parseCoordinateInput, type Coordinate } from '../lib/journey.ts';
import { JourneyError, readProviderResponse } from '../worker/journey-response.ts';
import { requestConnectedJourney } from '../worker/journey-routing.ts';

const from: Coordinate = [-48.09051, -21.71571], to: Coordinate = [-47.90249, -21.79387];
const geometry: Coordinate[] = [[-48.093102, -21.71415], [-47.906484, -21.785111]];
const response = (offset = 100) => ({ code: 'Ok', routes: [{ distance: 38281.7, duration: 32662.6, geometry: { coordinates: geometry } }], waypoints: [
    { location: geometry[0], distance: 318.98 }, { location: geometry[1], distance: offset },
] });

test('normal routes use one bounded request and need no extra confirmation', async () => {
    let calls = 0;
    const route = await requestConnectedJourney(from, to, 'foot', async (url, mode, radius) => {
        calls++;
        assert.equal(url.searchParams.get('radiuses'), '500;500');
        assert.equal(mode, 'foot'); assert.equal(radius, 500);
        return response();
    });
    assert.equal(calls, 1);
    assert.deepEqual(route.snapping, { search_radius_m: 500, expanded_search: false, requires_confirmation: false });
});

for (const code of ['NoRoute', 'NoSegment']) {
    test(`${code} widens once, preserving points, mode and provider; long access stays outside the geometry`, async () => {
        const urls: URL[] = [];
        const route = await requestConnectedJourney(from, to, 'foot', async (url, mode, radius) => {
            urls.push(url);
            return readProviderResponse(Response.json(urls.length === 1 ? { code } : response(1054.16), { status: urls.length === 1 ? 400 : 200 }), mode, radius);
        });
        assert.deepEqual(urls.map(url => url.searchParams.get('radiuses')), ['500;500', '2000;2000']);
        for (const url of urls) {
            assert.equal(url.origin, 'https://routing.openstreetmap.de');
            assert.equal(url.pathname, '/routed-foot/route/v1/driving/-48.09051,-21.71571;-47.90249,-21.79387');
        }
        assert.deepEqual(route.coordinates, geometry);
        assert.equal(route.distance_m, 38281.7);
        assert.equal(route.mode, 'foot');
        assert.deepEqual(route.snapping, { search_radius_m: 2000, expanded_search: true, requires_confirmation: true });
        assert.equal(journeyNeedsConfirmation(route), true);
    });
}

test('service errors and rate limits never trigger extra requests', async () => {
    for (const status of [429, 503]) {
        let calls = 0;
        await assert.rejects(requestConnectedJourney(from, to, 'bike', async () => { calls++; throw new JourneyError('Service failure', status); }),
            error => error instanceof JourneyError && error.status === status);
        assert.equal(calls, 1);
    }
});

test('a second route failure stops at the wider bound', async () => {
    let calls = 0;
    await assert.rejects(requestConnectedJourney(from, to, 'car', async (_url, mode, radius) => {
        calls++;
        return readProviderResponse(Response.json({ code: 'NoSegment' }, { status: 400 }), mode, radius);
    }), error => error instanceof JourneyError && error.code === 'NoSegment' && error.message.includes('2.000 m'));
    assert.equal(calls, 2);
});

test('expanded results reject out-of-bounds, negative and missing waypoint offsets', async () => {
    for (const offset of [2000.1, -1, NaN]) {
        let calls = 0;
        await assert.rejects(requestConnectedJourney(from, to, 'foot', async () => {
            if (++calls === 1) throw new JourneyError('No route', 422, 'NoRoute');
            return response(offset);
        }), error => error instanceof JourneyError && error.status === 422);
        assert.equal(calls, 2);
    }
});

test('invalid successful responses are not retried; confirmation depends on actual offset', async () => {
    let calls = 0;
    await assert.rejects(requestConnectedJourney(from, to, 'foot', async () => { calls++; return { code: 'Ok' }; }),
        error => error instanceof JourneyError && error.status === 503);
    assert.equal(calls, 1);
    const route = await requestConnectedJourney(from, to, 'foot', async () => response(500));
    assert.equal(journeyNeedsConfirmation(route), false);
    assert.equal(journeyNeedsConfirmation({ ...route, waypoints: route.waypoints.map(w => ({ ...w, distance_m: 500.1 })) }), true);
});

test('coordinate input keeps the selected points exact and enforces latitude-longitude order', () => {
    assert.deepEqual(parseCoordinateInput(' -21.71571, -48.09051 '), from);
    assert.deepEqual(parseCoordinateInput('-21.79387, -47.90249'), to);
    for (const text of ['-48.09051,-21.71571', 'Rua São Carlos', 'NaN, -48', '-10, -40']) assert.equal(parseCoordinateInput(text), null);
});
