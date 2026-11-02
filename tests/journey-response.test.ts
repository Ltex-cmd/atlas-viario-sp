import test from 'node:test';
import assert from 'node:assert/strict';
import { JourneyError, readProviderResponse } from '../worker/journey-response.ts';

for (const status of [400, 200]) {
    test(`OSRM NoRoute with HTTP ${status} is an unroutable selection, not downtime`, async () => {
        await assert.rejects(readProviderResponse(Response.json({ code: 'NoRoute', message: 'Impossible route between points' }, { status }), 'foot'),
            error => error instanceof JourneyError && error.status === 422 && error.message.includes('caminho a pé') && !error.message.includes('indisponível'));
    });
}
test('NoSegment identifies a point outside the supported snapping radius', async () => {
    await assert.rejects(readProviderResponse(Response.json({ code: 'NoSegment' }, { status: 400 }), 'car'),
        error => error instanceof JourneyError && error.status === 422 && error.message.includes('500 m'));
});
test('rate limits and actual outages retain distinct statuses and service names', async () => {
    await assert.rejects(readProviderResponse(new Response('Too many requests', { status: 429 }), 'bike'),
        error => error instanceof JourneyError && error.status === 429 && error.message.includes('rotas'));
    await assert.rejects(readProviderResponse(new Response('<html>Bad gateway</html>', { status: 502 })),
        error => error instanceof JourneyError && error.status === 503 && error.message.includes('busca'));
});
test('malformed success responses are rejected; valid routes and search results remain intact', async () => {
    await assert.rejects(readProviderResponse(new Response('<html>Not JSON</html>'), 'foot'),
        error => error instanceof JourneyError && error.status === 503 && error.message.includes('incompleta'));
    const route = { code: 'Ok', routes: [{ distance: 100 }] }, places = { features: [] };
    assert.deepEqual(await readProviderResponse(Response.json(route), 'car'), route);
    assert.deepEqual(await readProviderResponse(Response.json(places)), places);
});
