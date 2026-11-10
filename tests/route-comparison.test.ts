import test from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { pointInRings, routeLengthByMunicipality, poissonUpperTail, classifyRoute, type Rings } from '../lib/route-comparison.ts';
import { routeLength, type Coordinate } from '../lib/journey.ts';
const square = (w: number, s: number, e: number, n: number): Rings => [[[w, s], [e, s], [e, n], [w, n], [w, s]]];

test('Poisson: cauda exata, extremos e monotonicidade', () => {
    assert.equal(poissonUpperTail(0, 5), 1);
    assert.ok(Math.abs(poissonUpperTail(1, 0.2) - 0.181269) < 1e-6);
    assert.ok(Math.abs(poissonUpperTail(3, 2.1) - 0.350369) < 1e-6);
    assert.ok(poissonUpperTail(10, 4.31) > 0.01 && poissonUpperTail(10, 4.31) < 0.02);
    assert.ok(Number.isFinite(poissonUpperTail(60000, 50000)) && poissonUpperTail(60000, 50000) < 1e-100);
    assert.ok(poissonUpperTail(40000, 50000) > 0.999999);
    assert.equal(poissonUpperTail(1, 0), 0);
    for (const mean of [0.001, 0.2, 4.31, 50, 50000]) {
        let previous = 1;
        const start = mean > 100 ? 49990 : 0;
        for (let n = start; n < start + 30; n++) {
            const p = poissonUpperTail(n, mean);
            assert.ok(p >= 0 && p <= 1 && p <= previous + 1e-10);
            previous = p;
        }
    }
});
test('Classificação: ordem das regras e contagens críticas DNIT', () => {
    assert.equal(classifyRoute(2, 4)?.level, 'below');
    assert.equal(classifyRoute(1, 0.01)?.level, 'near');
    assert.equal(classifyRoute(3, 2.1)?.level, 'near');
    assert.equal(classifyRoute(12, 4.31)?.level, 'critical_995');
    const eight = classifyRoute(8, 4.31)!;
    assert.ok(eight.p_value > 0.05 && eight.p_value <= 0.1);
    assert.equal(eight.level, 'critical_90');
    assert.equal(classifyRoute(5, 0), null);
    assert.equal(classifyRoute(5, Infinity), null);
    assert.equal(classifyRoute(5, NaN), null);
    assert.deepEqual(classifyRoute(5, 4)?.critical_counts, { c90: 4 + 1.282 * 2 - 0.5, c95: 4 + 1.645 * 2 - 0.5, c995: 4 + 2.576 * 2 - 0.5 });
});
test('Polígono com furo', () => {
    const rings = [...square(0, 0, 10, 10), ...square(4, 4, 6, 6)];
    assert.equal(pointInRings(2, 2, rings), true);
    assert.equal(pointInRings(5, 5, rings), false);
    assert.equal(pointInRings(11, 2, rings), false);
});
test('Extensão por município: divisa, trecho externo e conservação', () => {
    const polygons = { a: square(0, 0, 0.01, 0.01), b: square(0.01, 0, 0.02, 0.01) };
    const route: Coordinate[] = [[-0.005, 0.005], [0.025, 0.005]];
    const result = routeLengthByMunicipality(route, polygons), length = routeLength(route);
    assert.ok(Math.abs(result.metres.a - length / 3) < 50);
    assert.ok(Math.abs(result.metres.b - length / 3) < 50);
    assert.ok(Math.abs(result.outside_m - length / 3) < 50);
    assert.ok(Math.abs(result.metres.a + result.metres.b + result.outside_m - length) < 1e-8);
    assert.deepEqual(routeLengthByMunicipality([route[0], route[0]], polygons), { metres: {}, outside_m: 0 });
});
test('Integridade da referência OSM', () => {
    const roads = JSON.parse(readFileSync(new URL('../worker/road-network.json', import.meta.url), 'utf8'));
    for (const code of ['3548906', '3550308']) {
        assert.ok(roads.municipalities[code]?.main_km > 0, `Malha principal ausente: ${code}`);
        assert.ok(roads.municipalities[code]?.local_km > 0, `Malha local ausente: ${code}`);
    }
});
test('Integridade dos 645 polígonos IBGE e ponto de São Carlos', () => {
    const polygons = JSON.parse(readFileSync(new URL('../worker/municipal-polygons.json', import.meta.url), 'utf8'));
    assert.equal(Object.keys(polygons).length, 645);
    assert.equal(pointInRings(-47.8908, -22.0175, polygons['3548906']), true);
});

test('Duas partes disjuntas: par-ímpar e caixa envolvendo ambas', () => {
    const rings = [...square(0, 0, 0.01, 0.01), ...square(0.02, 0, 0.03, 0.01)];
    assert.equal(pointInRings(0.005, 0.005, rings), true);
    assert.equal(pointInRings(0.025, 0.005, rings), true);
    assert.equal(pointInRings(0.015, 0.005, rings), false);
    // A rota está inteiramente na segunda parte, fora da caixa do primeiro anel.
    const route: Coordinate[] = [[0.021, 0.005], [0.029, 0.005]];
    const result = routeLengthByMunicipality(route, { a: rings });
    assert.ok(Math.abs(result.metres.a - routeLength(route)) < 1e-8);
    assert.equal(result.outside_m, 0);
});

test('Taboão da Serra: divisa detalhada e referência urbana', () => {
    const polygons = JSON.parse(readFileSync(new URL('../worker/municipal-polygons.json', import.meta.url), 'utf8'));
    const roads = JSON.parse(readFileSync(new URL('../worker/road-network.json', import.meta.url), 'utf8'));
    assert.ok(polygons['3552809'].reduce((total: number, ring: number[][]) => total + ring.length, 0) >= 20);
    const taboao = roads.municipalities['3552809'];
    assert.ok(taboao.main_km + taboao.local_km > 100);
});
