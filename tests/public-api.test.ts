import { test } from 'node:test';
import assert from 'node:assert/strict';
import { corsHeaders } from '../worker/public-api.ts';
test('Pages can call the public API without credentials and receive JSON preflight headers', () => {
    const h = corsHeaders('https://ltex-cmd.github.io');
    assert.equal(h?.get('Access-Control-Allow-Origin'), 'https://ltex-cmd.github.io');
    assert.ok(h?.get('Access-Control-Allow-Methods')?.includes('POST'));
    assert.ok(h?.get('Access-Control-Allow-Headers')?.includes('Content-Type'));
    assert.equal(h?.get('Access-Control-Allow-Credentials'), null);
    assert.equal(h?.get('Vary'), 'Origin');
});
test('CORS rejects unrelated origins and permits loopback only in development', () => {
    assert.equal(corsHeaders('https://example.com'), null);
    assert.equal(corsHeaders('https://ltex-cmd.github.io.example.com'), null);
    assert.equal(corsHeaders('http://127.0.0.1:5174'), null);
    assert.ok(corsHeaders('http://127.0.0.1:5174', true));
    assert.ok(corsHeaders(null));
});
