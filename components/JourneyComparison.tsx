import type { ComparisonLevel } from '../lib/route-comparison';
export type RouteComparison = {
    status: 'ok' | 'unavailable'; reason: string | null; observed: number; route_km: number;
    expected?: number; ratio?: number; p_value?: number; level: ComparisonLevel | null;
    critical_counts?: { c90: number; c95: number; c995: number };
    municipalities: { code: number; name: string; route_km: number; network_km: number; reference_events: number; density_per_km: number; expected: number }[];
};
const titles: Record<ComparisonLevel, string> = {
    below: 'Abaixo da média', near: 'Dentro do esperado', critical_90: 'Muitos sinistros (levemente significativo)',
    critical_95: 'Muitos sinistros (significativo)', critical_995: 'Muitos sinistros (altamente significativo)',
};
const format = (n: number, digits = 2) => n.toLocaleString('pt-BR', { maximumFractionDigits: digits });
const one = (n: number) => n.toLocaleString('pt-BR', { minimumFractionDigits: 1, maximumFractionDigits: 1 });
const titleCase = (name: string) => name.toLocaleLowerCase('pt-BR').replace(/(^|\s)\S/g, c => c.toLocaleUpperCase('pt-BR'));
export default function JourneyComparison({ comparison: c }: { comparison: RouteComparison }) {
    if (c.status === 'unavailable') return <p className="journey-note">{c.reason === 'outside_filter' ? 'O percurso não passa pelo município selecionado nos filtros; não há comparação com a média.' : 'Comparação com a média indisponível para este percurso.'}</p>;
    if (!c.level || c.expected === undefined || c.ratio === undefined || c.p_value === undefined || !c.critical_counts) return null;
    return <div className="journey-comparison" data-level={c.level}>
        <strong>{titles[c.level]}</strong>
        <p>{format(c.observed, 0)} registrados; {c.municipalities.length === 1 ? `a média de ${titleCase(c.municipalities[0].name)}` : `a média dos ${format(c.municipalities.length, 0)} municípios atravessados`} indicaria {one(c.expected)} em {format(c.route_km)} km ({one(c.ratio)}× a média).</p>
        <details><summary>Como é feita a comparação</summary>
            <ul>{c.municipalities.map(m => <li key={m.code}><strong>{titleCase(m.name)}</strong>: {format(m.route_km)} km da rota; {format(m.network_km)} km de malha; {format(m.reference_events, 0)} sinistros de referência; {format(m.density_per_km)} sinistros por km; esperado: {format(m.expected)}.</li>)}</ul>
            <p>Valor-p: {c.p_value.toLocaleString('pt-BR', { maximumSignificantDigits: 4 })}. Contagens críticas aproximadas do DNIT: 90%: {format(c.critical_counts.c90)}; 95%: {format(c.critical_counts.c95)}; 99,5%: {format(c.critical_counts.c995)}.</p>
            <p>A referência é a média de sinistros por quilômetro de via em cada município atravessado, no mesmo período e com os mesmos filtros. A rota é classificada como tendo muitos sinistros quando a contagem supera essa média além do acaso, pelo teste de Poisson usado no controle de qualidade da taxa, com os graus de confiança de 90%, 95% e 99,5% do método do DNIT. Com menos de 3 sinistros a rota não é classificada como crítica.</p>
            <p>Limites: a base não tem volume de tráfego, então a comparação é por extensão e não por veículo. Avenidas e rodovias movimentadas tendem a ficar acima da média municipal, que inclui ruas residenciais. Faixas mais largas incluem vias vizinhas e elevam a razão. Não é uma previsão nem uma medida de risco individual.</p>
            <p>Referências: Ministério dos Transportes (2002), Procedimentos para o Tratamento de Locais Críticos de Acidentes de Trânsito; DNIT/UFSC (2010), Método Homologado de Identificação e Priorização de Segmentos Críticos; AASHTO (2010), Highway Safety Manual, cap. 4. Malha viária: © OpenStreetMap.</p>
        </details>
    </div>;
}
