import { test } from 'node:test';
import assert from 'node:assert/strict';
import { createPartitionReader } from '../worker/partition-cache.ts';

const bytes = (value: number) => new Uint32Array([value]).buffer;
const makeReader = () => createPartitionReader((data: ArrayBuffer, previous: { value: number; destroyed: boolean } | null) => {
    if (previous) previous.destroyed = true;
    return { value: new Uint32Array(data)[0], destroyed: false };
});

test('an abandoned pending request cannot block an independent query', async () => {
    const read = makeReader();
    void read(2025, () => new Promise<ArrayBuffer>(() => {}), state => state.value);
    let timer: ReturnType<typeof setTimeout>;
    const timeout = new Promise<never>((_, reject) => { timer = setTimeout(() => reject(new Error('query blocked by another request')), 500); });
    try {
        const value = await Promise.race([read(2026, async () => bytes(126272), state => state.value), timeout]);
        assert.equal(value, 126272);
    } finally { clearTimeout(timer!); }
});

test('out-of-order loads each consume their own complete partition before replacement', async () => {
    const read = makeReader();
    let finish!: (value: ArrayBuffer) => void;
    const older = read(2025, () => new Promise(resolve => { finish = resolve; }), state => {
        assert.equal(state.destroyed, false);
        return state.value;
    });
    const newer = await read(2026, async () => bytes(126272), state => {
        assert.equal(state.destroyed, false);
        return state.value;
    });
    finish(bytes(123));
    assert.equal(await older, 123);
    assert.equal(newer, 126272);
    assert.equal(await read(2026, async () => bytes(126272), state => state.value), 126272);
});

test('failed I/O leaves a completed cached partition available', async () => {
    const read = makeReader();
    await read(2026, async () => bytes(126272), state => state.value);
    await assert.rejects(read(2025, async () => { throw new Error('disconnected'); }, state => state.value));
    const value = await read(2026, async () => { throw new Error('warm cache should not fetch'); }, (state, warm) => {
        assert.equal(warm, true);
        assert.equal(state.destroyed, false);
        return state.value;
    });
    assert.equal(value, 126272);
});
