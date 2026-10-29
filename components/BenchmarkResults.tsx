import DataTable from './DataTable';
import { AlertCircle, Check } from './icons';

const methods = ['Varredura sequencial', 'Índice temporal / municipal', 'Grade espacial', 'Quadtree', 'Planejador automático'];
const fmt = (n: number) => n.toLocaleString('pt-BR');

export default function BenchmarkResults({ result }: { result: any }) {
    const matches = result.methods.every((method: any) => method.equivalent && method.count === result.map_count);
    const hasTimes = result.methods.some((method: any) => typeof method.median_ms === 'number');
    const columns = [
        { key: 'count', label: 'Eventos encontrados' },
        { key: 'check', label: 'Conferência' },
        { key: 'examined', label: 'Registros examinados' },
        { key: 'nodes', label: 'Nós ou células' },
        ...(hasTimes ? [{ key: 'time', label: 'Tempo de execução' }] : [])
    ];
    return <div className="benchmark-result">
        <p className={matches ? 'equivalence' : 'equivalence error-text'} role="status">
            {matches ? <Check size={20}/> : <AlertCircle size={20}/>}
            {matches ? 'As contagens e a conferência dos registros coincidem nos cinco métodos.' : 'Os métodos retornaram resultados diferentes.'}
        </p>
        <DataTable caption="Resultado da consulta atual" columns={columns} rows={result.methods.map((method: any, i: number) => {
            const same = method.equivalent && method.count === result.map_count;
            return { id: method.method ?? i, label: methods[i], values: {
                count: fmt(method.count),
                check: <span className={same ? 'success' : 'error-text'}>{same ? 'Coincide' : 'Difere'}</span>,
                examined: fmt(method.examined), nodes: fmt(method.nodes),
                time: method.median_ms == null ? 'Indisponível' : `${method.median_ms.toLocaleString('pt-BR', { minimumFractionDigits: 3, maximumFractionDigits: 3 })} ms`
            } };
        })}/>
        <div className="comparison-explanation">
            <p>A conferência compara o total e um valor numérico calculado a partir dos registros. Valores iguais ajudam a detectar divergências, mas não provam que os conjuntos são idênticos.</p>
            {!hasTimes && <p>Esta consulta não tem uma medição isolada do tempo de execução. A seção seguinte mostra tempos medidos em cenários fixos.</p>}
        </div>
        <details className="request-details"><summary>Detalhes da resposta</summary>
            <dl><div><dt>Tempo de ida e volta</dt><dd>{fmt(Math.round(result.roundtrip_ms))} ms</dd></div><div><dt>Tamanho do JSON</dt><dd>{(result.http_body_bytes / 1024).toLocaleString('pt-BR', { minimumFractionDigits: 1, maximumFractionDigits: 1 })} KiB</dd></div></dl>
            <p>O tempo de ida e volta inclui o envio da consulta, a execução no servidor e o recebimento da resposta.</p>
            {result.timing_note && <p>{result.timing_note}</p>}
        </details>
    </div>;
}
