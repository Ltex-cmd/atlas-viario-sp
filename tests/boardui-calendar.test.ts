import test from 'node:test';
import assert from 'node:assert/strict';
import { parseChipDate } from '../components/boardui/base/date-picker/date-value.ts';

test('typed calendar dates preserve the exact day, month and year', () => {
    assert.equal(parseChipDate(' 1/1/2015 ')?.toString(), '2015-01-01');
    assert.equal(parseChipDate('29/02/2024')?.toString(), '2024-02-29');
    assert.equal(parseChipDate('30/09/2026')?.toString(), '2026-09-30');
});

test('impossible calendar dates are rejected instead of changed to another day', () => {
    for (const text of ['31/02/2026', '29/02/2025', '31/04/2026', '00/09/2026', '01/13/2026', '01/01/0000', '2026-09-30', '']) {
        assert.equal(parseChipDate(text), null, text);
    }
});
