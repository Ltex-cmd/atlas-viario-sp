// Check the rendered state, including the difference between missing data and zero.
import { build } from 'esbuild';
import { mkdirSync } from 'node:fs';
import { createRequire } from 'node:module';
import assert from 'node:assert/strict';
import { renderToStaticMarkup } from 'react-dom/server';
import { createElement } from 'react';

const require = createRequire(import.meta.url);
const output = new URL('../.sites-runtime/qa/StatisticsPanel.cjs', import.meta.url);
mkdirSync(new URL('.', output), { recursive: true });
await build({ entryPoints: [new URL('../components/StatisticsPanel.tsx', import.meta.url).pathname], outfile: output.pathname, bundle: true, platform: 'node', format: 'cjs', packages: 'external', jsx: 'automatic', logLevel: 'silent' });
const Panel = require(output.pathname).default;
const zero = { total: 0, fatal_events: 0, fatal: 0, located: 0, months: Array(12).fill(0), types: [0, 0] };
const props = { error: '', onRetry() {}, meta: { types: ['Colisão', 'Choque'] }, year: '2026', dateEnd: '2026-09-30', partial: true };
const html = (extra) => renderToStaticMarkup(createElement(Panel, { ...props, ...extra }));
for (const stats of [null, zero, { ...zero, total: 100 }]) {
    const pending = html({ stats, ready: false });
    assert.match(pending, /Carregando estatísticas/);
    assert.doesNotMatch(pending, /0%|Jan|Tipos de sinistro|Nenhum sinistro/);
}
const empty = html({ stats: zero, ready: true });
assert.match(empty, /Nenhum sinistro corresponde/);
assert.match(empty, /0%/);
assert.doesNotMatch(empty, /Carregando estatísticas/);
const failed = html({ stats: null, ready: false, error: 'offline' });
assert.match(failed, /Tentar novamente/);
assert.doesNotMatch(failed, /Carregando estatísticas|0%|Jan/);
console.log('Statistics rendering passed: absent, stale and pending data show no zero results; real zero and failed requests have distinct states.');
