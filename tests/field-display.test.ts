import test from 'node:test';
import assert from 'node:assert/strict';
import { displayField } from '../components/field-display.ts';

test('empty Infosiga counts mean zero and empty type indicators mean no', () => {
    assert.equal(displayField('qtd_motocicleta', ''), '0');
    assert.equal(displayField('qtd_gravidade_fatal', null), '0');
    assert.equal(displayField('qtd_gravidade_leve', '2'), '2');
    assert.equal(displayField('tp_sinistro_colisao_frontal', ''), 'Não');
    assert.equal(displayField('tp_sinistro_colisao_frontal', 'N'), 'Não');
    assert.equal(displayField('tp_sinistro_colisao_frontal', 'S'), 'Sim');
});

test('unknown categories and missing locations stay unknown', () => {
    assert.equal(displayField('tp_sinistro_primario', 'NAO DISPONIVEL'), 'NAO DISPONIVEL');
    for (const name of ['tp_sinistro_primario', 'tipo_via', 'latitude', 'logradouro']) {
        assert.equal(displayField(name, ''), 'Não informado');
    }
    assert.equal(displayField('numero_logradouro', 0), '0');
});
