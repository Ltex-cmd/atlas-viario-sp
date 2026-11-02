'use client';
import { useState } from 'react';
import DataTable from './DataTable';

const names = ['Varredura sequencial', 'Índice temporal / municipal', 'Grade espacial', 'Quadtree', 'Planejador automático'];
const fmt = (n: number) => n.toLocaleString('pt-BR');
const decimal = (n: number, digits: number) => n.toLocaleString('pt-BR', { minimumFractionDigits: digits, maximumFractionDigits: digits });
const ms = (n: number) => `${decimal(n, 3)} ms`;
const mib = (n: number) => `${decimal(n / 1048576, 2)} MiB`;
const scenarios = [
    ['spatial', 'São Paulo e motocicletas', 'São Paulo, 2026, motocicleta, região metropolitana.'],
    ['broad', 'Estado inteiro', 'Todos os eventos localizados de 2026, na caixa ampla do estado.'],
    ['narrow', 'Área central', '2026, retângulo central de São Paulo: −46,68/−23,58 a −46,60/−23,50.']
];

export default function BenchmarkPanel({ data, meta }: { data: any; meta: any }) {
    const [scenario, setScenario] = useState('spatial');
    if (!data) return <section className="native-panel"><h2>Tempos em cenários fixos</h2><p role="status">Carregando medições…</p></section>;
    const result = data[scenario], storage = data.storage;
    return <section className="native-panel" aria-labelledby="saved-times-title">
        <div className="section-title"><h2 id="saved-times-title">Tempos em cenários fixos</h2></div>
        <p className="muted">Estas medições foram salvas no computador de teste. Os filtros e a área do mapa não alteram os cenários abaixo.</p>
        <p className="methodology-note">Cada tempo é a mediana de sete execuções, com a base e os índices já carregados.</p>
        {data.dataset_version && meta?.version && data.dataset_version !== meta.version && <p className="coverage-warning">Estas medições usam outra versão da base, com {fmt(result.records)} registros. A consulta atual usa a base carregada no site.</p>}
        <div className="benchmark-scenarios" role="group" aria-label="Cenário da medição">{scenarios.map(([key, label]) => <button key={key} aria-pressed={scenario === key} onClick={() => setScenario(key)}>{label}</button>)}</div>
        <p>{scenarios.find(([key]) => key === scenario)?.[2]} <b>{fmt(result.methods[0].count)} eventos.</b></p>
        <div className="timing-bars">{result.methods.map((method: any, i: number) => <div key={i}><span>{names[i]}</span><div className="timing-track"><i style={{ width: `${Math.max(1, 100 * method.median_ms / Math.max(...result.methods.map((value: any) => value.median_ms)))}%` }}/></div><b>{ms(method.median_ms)}</b></div>)}</div>
        <DataTable caption="Trabalho realizado no cenário selecionado" columns={[{ key: 'count', label: 'Eventos encontrados' }, { key: 'examined', label: 'Registros examinados' }, { key: 'nodes', label: 'Nós ou células' }]} rows={result.methods.map((method: any, i: number) => ({ id: i, label: names[i], values: { count: fmt(method.count), examined: fmt(method.examined), nodes: fmt(method.nodes) } }))}/>
        <div className="benchmark-facts"><span><b>{mib(result.memory_bytes)}</b>Memória dos registros e índices</span><span><b>{decimal(result.startup_ms, 1)} ms</b>Inicialização do servidor</span><span><b>{fmt(result.http_body_bytes)} bytes</b>Tamanho da resposta</span><span><b>{fmt(result.records)}</b>Registros na base</span></div>
        <div className="measurement-notes">
            <p>Ambiente de teste: {data.environment}. Medido em {new Date(data.measured_at).toLocaleString('pt-BR', { timeZone: 'America/Sao_Paulo' })}.</p>
            <p>A memória soma as alocações do motor, não o consumo total do processo. O tamanho do JSON exclui cabeçalhos e compressão de transporte.</p>
        </div>
        <section className="benchmark-section"><h3>Preparação e inicialização</h3>
            <p>A preparação valida o CSV e gera os arquivos de consulta. Na inicialização, o servidor abre os dados e constrói os índices. Os tempos de consulta acima excluem essas etapas.</p>
            <div className="benchmark-facts"><span><b>{meta?.preparation ? decimal(meta.preparation.total_wall_ms / 1000, 2) + ' s' : 'Indisponível'}</b>Preparação completa da base</span><span><b>{result.mapping_ms == null ? 'Indisponível' : ms(result.mapping_ms)}</b>Abertura e mapeamento da fonte</span><span><b>{result.index_build_ms == null ? 'Indisponível' : decimal(result.index_build_ms, 1) + ' ms'}</b>Construção de todos os índices</span></div>
            {result.indexes && <DataTable firstColumn="Índice" caption="Construção e memória por índice" columns={[{ key: 'build', label: 'Construção' }, { key: 'memory', label: 'Memória alocada' }]} rows={result.indexes.map((index: any) => ({ id: index.name, label: index.name, values: { build: ms(index.build_ms), memory: mib(index.bytes) } }))}/>}
            <div className="measurement-notes">
                <p>A construção foi medida em uma inicialização, com o cache do sistema aquecido. A alocação comum custou {result.shared_allocation_ms == null ? 'Indisponível' : ms(result.shared_allocation_ms)}.</p>
                <p>A grade inclui a seleção dos pontos válidos que a quadtree reutiliza. Os registros ocupam {result.record_bytes ? mib(result.record_bytes) : 'um volume não informado nesta medição'}. A memória dos índices exclui posições no CSV, servidor HTTP e buffers temporários.</p>
            </div>
        </section>
        {storage && <details className="benchmark-details benchmark-section"><summary>Busca por ID e armazenamento</summary>
            <DataTable firstColumn="Operação" caption="Cada linha compara a mesma operação" columns={[{ key: 'a', label: 'Alternativa A' }, { key: 'b', label: 'Alternativa B' }]} rows={[
                { id: 'id', label: '50.000 buscas por ID', values: { a: `Binária: ${ms(storage.id_lookup_50000_binary_ms)}`, b: `Hash: ${ms(storage.id_lookup_50000_hash_ms)}` } },
                { id: 'scan', label: 'Varredura de data e município', values: { a: `AoS: ${ms(storage.aos_48byte_scan_ms)}`, b: `Duas colunas SoA: ${ms(storage.soa_two_columns_scan_ms)}` } },
                { id: 'read', label: 'Leitura com cache aquecido', values: { a: `mmap: ${ms(storage.warm_mmap_touch_ms)}`, b: `fread: ${ms(storage.warm_fread_touch_ms)}` } }
            ]}/>
            <div className="measurement-notes"><p>{storage.methodology}</p><p>A cópia SoA usa mais {mib(storage.soa_extra_bytes)}. Essa medição não cobre todas as agregações da aplicação nem leitura com disco frio.</p></div>
        </details>}
    </section>;
}
