import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { createHash } from 'node:crypto';

const root = new URL('../', import.meta.url);
const read = (path: string) => readFileSync(new URL(path, root));
const hash = (path: string) => createHash('sha256').update(read(path)).digest('hex');
const report = JSON.parse(read('components/sample-benchmark.json').toString());

test('published sample and sources match the measured native comparison', () => {
    assert.equal(report.sample.size, 1000);
    assert.equal(report.sample.selected_ids.length, 1000);
    assert.equal(new Set(report.sample.selected_ids).size, 1000);
    assert.equal(hash('benchmarks/sample/sinistros-1000.csv'), report.sample.csv_sha256);
    assert.equal(hash('benchmarks/sample/records-1000.bin'), report.sample.binary_sha256);
    const records = read('benchmarks/sample/records-1000.bin');
    assert.equal(records.length, 1000 * 48);
    for (let index = 0; index < 1000; index++) assert.equal(records.readUInt32LE(index * 48 + 16), report.sample.selected_ids[index]);
    assert.equal(hash('engine-source/engine.c'), report.provenance.site_engine_sha256);
    assert.equal(hash('engine-source/engine.h'), report.provenance.site_header_sha256);
    assert.equal(hash('benchmarks/src/compare.c'), report.provenance.harness_sha256);
    const original = JSON.parse(read('benchmarks/original-sha256.json').toString());
    assert.deepEqual(original, report.provenance.original_sha256);
    for (const [path, digest] of Object.entries(original)) assert.equal(hash('benchmarks/original/' + path), digest);
});

test('all published comparisons have 13 verified methods and measured medians', () => {
    assert.equal(report.scenarios.length, 4);
    for (const scenario of report.scenarios) {
        assert.equal(scenario.methods.length, 13);
        assert.equal(scenario.methods.filter((method: any) => method.family === 'original').length, 8);
        assert.equal(scenario.methods.filter((method: any) => method.family === 'site').length, 5);
        assert.equal(scenario.sorted_ids.length, scenario.count);
        assert.ok(scenario.sorted_ids.every((id: number) => report.sample.selected_ids.includes(id)));
        for (const method of scenario.methods) {
            assert.equal(method.exact_ids_match, true);
            assert.equal(method.aggregates_match, true);
            assert.ok(Number.isFinite(method.build_ms) && method.build_ms > 0);
            assert.ok(Number.isFinite(method.query_ms) && method.query_ms > 0);
            assert.ok(method.examined >= scenario.count && method.examined <= 1000);
            assert.ok(method.heap_retained_bytes >= method.record_bytes);
            assert.ok(method.heap_peak_bytes >= method.heap_retained_bytes);
            assert.equal(method.query_samples_ms.length, 21);
            const median = method.query_samples_ms.slice().sort((a: number, b: number) => a - b)[10];
            assert.equal(method.query_ms, median);
        }
    }
    assert.equal(report.scenarios[0].count, 1000);
});
