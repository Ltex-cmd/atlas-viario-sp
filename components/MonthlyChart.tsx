'use client';
import { useEffect, useState } from 'react';
import { OrdersChartCard } from './boardui/application/dashboard/orders-chart-card';
import { Button } from './AtlasControls';
import { apiURL } from '../lib/api-url';
import { filterKey, query, type Filters } from './query-state';
import { comparisonPeriod, monthlyPoints, selectedPeriod } from './monthly-comparison';

export default function MonthlyChart({ months, filters, dateStart, dateEnd }: {
    months: number[]; filters: Filters; dateStart: string; dateEnd: string;
}) {
    const [compare, setCompare] = useState(false), [retry, setRetry] = useState(0);
    const [result, setResult] = useState<{ key: string; months: number[] } | null>(null);
    const [error, setError] = useState('');
    const comparison = comparisonPeriod(filters, dateStart, dateEnd);
    const key = comparison ? filterKey(comparison.filters) : '';
    const ready = compare && result?.key === key;
    useEffect(() => {
        setError('');
        if (!compare || !comparison) return;
        const abort = new AbortController();
        setResult(null);
        fetch(apiURL('/api/v1/statistics?' + query(comparison.filters)), { signal: abort.signal })
            .then(async response => { const body = await response.json() as { months: number[]; error?: string }; if (!response.ok) throw Error(body.error || 'Consulta indisponível.'); return body; })
            .then(body => { if (!abort.signal.aborted) setResult({ key, months: body.months }); })
            .catch(reason => { if (!abort.signal.aborted) setError(reason.message); });
        return () => abort.abort();
    }, [compare, key, retry]);
    const period = selectedPeriod(filters, dateStart, dateEnd);
    return <div className="atlas-monthly-section">
        <div className="atlas-chart-controls">
            <span>{period.start.split('-').reverse().join('/')} a {period.end.split('-').reverse().join('/')}</span>
            {comparison && <Button variant="secondary" size="small" aria-pressed={compare} onClick={() => setCompare(value => !value)}>
                {compare ? 'Ocultar comparação' : `Comparar com ${comparison.previousYear}`}
            </Button>}
        </div>
        {compare && comparison && <p className="atlas-comparison-note" role="status">{error ? <>Não foi possível carregar a comparação. <Button variant="ghost" size="small" onClick={() => setRetry(n => n + 1)}>Tentar novamente</Button></> : ready ? `Comparação com ${comparison.previousStart.split('-').reverse().join('/')} a ${comparison.previousEnd.split('-').reverse().join('/')}, com os mesmos filtros.` : 'Carregando o período equivalente…'}</p>}
        <OrdersChartCard data={monthlyPoints(months, period, ready ? result!.months : undefined)} currentLabel={comparison?.year || filters.ano || 'Período selecionado'} previousLabel={ready ? comparison?.previousYear : undefined}/>
        <p className="atlas-chart-footnote">Meses fora do período ficam sem valor. Zero indica um mês coberto sem sinistros nos filtros.</p>
        {!comparison && <p className="atlas-chart-footnote">A comparação anual exige um intervalo dentro de um único ano e cobertura equivalente no ano anterior.</p>}
    </div>;
}
