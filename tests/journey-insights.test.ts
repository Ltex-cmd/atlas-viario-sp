import test from 'node:test';
import assert from 'node:assert/strict';
import { guessFeedback, transportBreakdown, duelHeadline, type DuelEntry } from '../lib/journey-insights.ts';
import { buildComparison } from '../lib/route-comparison.ts';

test('Chute: acerto, proximidade, abaixo, acima e zero', () => {
    assert.equal(guessFeedback(10, 10), 'Na mosca: 10.');
    assert.equal(guessFeedback(8, 10), 'Chegou perto: você disse 8, foram 10.');
    assert.equal(guessFeedback(3, 4), 'Chegou perto: você disse 3, foram 4.');
    assert.equal(guessFeedback(1, 10), 'Foram mais: você disse 1, foram 10.');
    assert.equal(guessFeedback(15, 10), 'Foram menos: você disse 15, foram 10.');
    assert.equal(guessFeedback(0, 0), 'Na mosca: 0.');
    assert.equal(guessFeedback(1, 0), 'Chegou perto: você disse 1, foram 0.');
    assert.equal(guessFeedback(2, 0), 'Foram menos: você disse 2, foram 0.');
});
test('Transportes: máscaras sobrepostas, vazio e indisponível', () => {
    assert.deepEqual(transportBreakdown([]), [0, 0, 0, 0, 0, 0, 0, 0]);
    assert.deepEqual(transportBreakdown([{ transport: 3 }, { transport: 9 }, { transport: 128 }, { transport: 255 }]), [3, 2, 1, 2, 1, 1, 1, 2]);
});
const entry = (mode: DuelEntry['mode'], total: number): DuelEntry => ({ id: mode, mode, total, from: { label: 'A', coordinates: [0, 0] }, to: { label: 'B', coordinates: [1, 1] } });
test('Duelo: razão, zero, parecidos, repetidos e incompletos', () => {
    assert.equal(duelHeadline([entry('foot', 10)]), null);
    assert.equal(duelHeadline([entry('foot', 10), { ...entry('car', 1), total: undefined }]), null);
    assert.equal(duelHeadline([entry('foot', 10), entry('car', 24)]), 'De carro tem 2,4× mais registros que a pé.');
    assert.equal(duelHeadline([entry('foot', 0), entry('bike', 5)]), 'De bicicleta tem 5 registros; a pé, nenhum.');
    assert.equal(duelHeadline([entry('foot', 10), entry('car', 11)]), 'Os caminhos têm números parecidos.');
    assert.equal(duelHeadline([entry('foot', 0), entry('car', 0)]), 'Os caminhos têm números parecidos.');
    assert.equal(duelHeadline([entry('car', 5), entry('car', 10)]), 'O caminho 2 tem 2,0× mais registros que o caminho 1.');
});
test('Montagem compartilhada: valores fixos e referências ausentes', () => {
    const context = { lengths: { metres: { '1': 2000 }, outside_m: 100 }, selected: [['1', 2000]] as [string, number][], counts: { '1': 100 }, route_m: 2100, outsideFilter: false };
    const result = buildComparison(context, 8, 50, [{ code: 1, name: 'Município' }], { '1': { main_km: 20, local_km: 80 } });
    assert.equal(result.status, 'ok');
    assert.equal(result.route_km, 2.1);
    assert.equal(result.outside_km, 0.1);
    assert.equal(result.expected, 2);
    assert.equal(result.ratio, 4);
    assert.deepEqual(result.municipalities, [{ code: 1, name: 'Município', route_km: 2, network_km: 100, reference_events: 100, density_per_km: 1, expected: 2 }]);
    assert.equal(buildComparison(context, 8, 50, [], {}).reason, 'no_reference');
    assert.equal(buildComparison({ ...context, outsideFilter: true }, 8, 50, [], {}).reason, 'outside_filter');
    assert.equal(buildComparison({ ...context, counts: {} }, 8, 50, [], { '1': { main_km: 1, local_km: 1 } }).reason, 'no_events');
});
