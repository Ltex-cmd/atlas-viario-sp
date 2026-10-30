const quantities = new Set([
    'qtd_pedestre', 'qtd_bicicleta', 'qtd_motocicleta', 'qtd_automovel',
    'qtd_onibus', 'qtd_caminhao', 'qtd_veic_outros', 'qtd_veic_nao_disponivel',
    'qtd_gravidade_fatal', 'qtd_gravidade_grave', 'qtd_gravidade_leve',
    'qtd_gravidade_ileso', 'qtd_gravidade_nao_disponivel',
]);
const typeIndicators = new Set([
    'tp_sinistro_atrop_pedestre', 'tp_sinistro_atrop_vitima_fora_veic',
    'tp_sinistro_colisao_frontal', 'tp_sinistro_colisao_traseira',
    'tp_sinistro_colisao_lateral', 'tp_sinistro_colisao_transversal',
    'tp_sinistro_colisao_outros', 'tp_sinistro_choque', 'tp_sinistro_atrop_animal',
    'tp_sinistro_capotamento', 'tp_sinistro_engavetamento', 'tp_sinistro_tombamento',
    'tp_sinistro_outros', 'tp_sinistro_nao_disponivel',
]);

/** Display the same meanings as reader_detran.c without changing the CSV values. */
export function displayField(name: string, value: unknown): string {
    const raw = value == null ? '' : String(value);
    if (typeIndicators.has(name)) return raw === 'S' ? 'Sim' : 'Não';
    if (!raw.trim()) return quantities.has(name) ? '0' : 'Não informado';
    return raw;
}
