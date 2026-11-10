import test from 'node:test';
import assert from 'node:assert/strict';
import { comparisonPeriod, monthlyPoints, selectedPeriod } from '../components/monthly-comparison.ts';
const filters = { ano: '2026', municipio: '3548906', tipo: '0', transporte: '', via: '', registro: '', inicio: '', fim: '' };

test('partial year compares only covered calendar interval and retains category/municipality filters', () => {
    const result = comparisonPeriod(filters, '2015-01-01', '2026-09-30')!;
    assert.equal(result.previousStart, '2025-01-01');
    assert.equal(result.previousEnd, '2025-09-30');
    assert.deepEqual(result.filters, { ...filters, ano: '', inicio: '2025-01-01', fim: '2025-09-30' });
    const points = monthlyPoints([0, 2, 0, 3, 4, 0, 0, 0, 7, 0, 0, 0], result, Array(12).fill(1));
    assert.equal(points[0].current, 0);
    assert.equal(points[8].current, 7);
    assert.equal(points[9].current, null);
    assert.equal(points[9].previous, null);
});

test('annual comparison excludes mixed coverage, missing preceding years and multi-year selections', () => {
    assert.equal(comparisonPeriod({ ...filters, ano: '2019' }, '2015-01-01', '2026-09-30'), null);
    assert.equal(comparisonPeriod({ ...filters, ano: '2015' }, '2015-01-01', '2026-09-30'), null);
    assert.equal(comparisonPeriod({ ...filters, ano: '' }, '2015-01-01', '2026-09-30'), null);
    assert.ok(comparisonPeriod({ ...filters, ano: '2018' }, '2015-01-01', '2026-09-30'));
});

test('custom date ranges preserve coverage boundaries and clamp leap days in preceding year', () => {
    const custom = { ...filters, ano: '', inicio: '2024-02-29', fim: '2024-03-05' };
    const result = comparisonPeriod(custom, '2015-01-01', '2026-09-30')!;
    assert.equal(result.previousStart, '2023-02-28');
    assert.equal(result.previousEnd, '2023-03-05');
    const points = monthlyPoints(Array(12).fill(0), selectedPeriod(custom, '2015-01-01', '2026-09-30'));
    assert.equal(points[0].current, null);
    assert.equal(points[1].current, 0);
    assert.equal(points[2].current, 0);
    assert.equal(points[3].current, null);
});

test('ranges spanning a year boundary only expose included month buckets', () => {
    const points = monthlyPoints(Array(12).fill(5), { start: '2024-11-20', end: '2025-02-10' });
    assert.deepEqual(points.map(p => p.current), [5, 5, null, null, null, null, null, null, null, null, 5, 5]);
});
