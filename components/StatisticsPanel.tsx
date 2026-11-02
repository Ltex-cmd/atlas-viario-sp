'use client';
import { lazy, Suspense } from 'react';
import type { Filters } from './query-state';
const MonthlyChart = lazy(() => import('./MonthlyChart'));
import { StatCards } from './boardui/application/dashboard/stat-cards';
import { Skeleton } from './boardui/base/skeleton/skeleton';
import { CarFront, AlertCircle, Users, MapPin } from './icons';
const fmt = (n: number) => n.toLocaleString('pt-BR');

export default function StatisticsPanel({ stats, ready, error, onRetry, meta, year, dateEnd, partial, filters }: {
    stats: any; ready: boolean; error: string; onRetry: () => void; meta: any; year: string; dateEnd: string; partial: boolean; filters?: Filters;
}) {
    const percentage = (n: number) => (stats.total ? n / stats.total * 100 : 0).toLocaleString('pt-BR', { maximumFractionDigits: 1 });
    return <section className="summary-section" aria-labelledby="summary-title" aria-busy={!ready && !error}>
        <h2 id="summary-title">Resumo dos filtros selecionados</h2>
        <p className="summary-scope">Os totais incluem sinistros sem localização, em toda a região selecionada nos filtros.</p>
        {!ready ? <>
            <div className="statistics-state" role="status">{error ? <><p>Não foi possível carregar as estatísticas.</p><button onClick={onRetry}>Tentar novamente</button></> : <p>Carregando estatísticas…</p>}</div>
            {!error && <div className="atlas-stat-skeletons" aria-hidden="true">{[0,1,2,3].map(key=><Skeleton key={key} className="atlas-stat-skeleton"/>)}</div>}
        </> : <>
            <StatCards className="atlas-stat-cards" stats={[
                { icon: CarFront, label: 'Sinistros', value: fmt(stats.total), unit: 'eventos', description: 'Total dos filtros selecionados' },
                { icon: AlertCircle, label: 'Sinistros fatais', value: fmt(stats.fatal_events), unit: 'eventos', description: `${percentage(stats.fatal_events)}% dos sinistros da consulta` },
                { icon: Users, label: 'Vítimas fatais', value: fmt(stats.fatal), unit: 'pessoas', description: 'Pessoas que morreram nos sinistros' },
                { icon: MapPin, label: 'Com localização', value: fmt(stats.located), unit: 'eventos', description: `${percentage(stats.located)}% dos registros com coordenadas` },
            ]}/>
            {stats.total === 0 ? <p className="empty-statistics">Nenhum sinistro corresponde aos filtros selecionados.</p> : <div className="bottom-grid">
                <Suspense fallback={<div className="atlas-chart-placeholder" role="status">Carregando gráfico…</div>}>
                    <MonthlyChart key={JSON.stringify(filters ?? year)} months={stats.months}
                        filters={filters ?? { ano: year, municipio: '', tipo: '', via: '', transporte: '', registro: '', inicio: '', fim: '' }}
                        dateStart={String(meta?.date_start ?? 20150101).replace(/^(\d{4})(\d{2})(\d{2})$/, '$1-$2-$3')} dateEnd={dateEnd}/>
                </Suspense>
                <section className="types-panel"><h3>Tipos de sinistro</h3><div className="type-list">{meta?.types.map((name: string, i: number) => ({ name, n: stats.types[i] })).sort((a: any, b: any) => b.n - a.n).map((item: any) => <div className="type-row" key={item.name}><span className="type-name">{item.name}<i style={{ width: `${item.n / stats.total * 100}%` }}/></span><b>{fmt(item.n)}</b><small>{percentage(item.n)}%</small></div>)}</div></section>
            </div>}
        </>}
    </section>;
}
