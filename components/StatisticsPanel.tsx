'use client';
import { lazy, Suspense } from 'react';
import type { Filters } from './query-state';
const MonthlyChart = lazy(() => import('./MonthlyChart'));
import { StatCards } from './boardui/application/dashboard/stat-cards';
import { Skeleton } from './boardui/base/skeleton/skeleton';
import { CarFront, AlertCircle, Users, MapPin } from './icons';
import type { CSSProperties } from 'react';
import { CountUp, reveal } from './motion';
const fmt = (n: number) => n.toLocaleString('pt-BR');

export default function StatisticsPanel({ stats, ready, error, onRetry, meta, year, dateEnd, partial, filters }: {
    stats: any; ready: boolean; error: string; onRetry: () => void; meta: any; year: string; dateEnd: string; partial: boolean; filters?: Filters;
}) {
    const percentage = (n: number) => (stats.total ? n / stats.total * 100 : 0).toLocaleString('pt-BR', { maximumFractionDigits: 1 });
    return <section className="summary-section" aria-labelledby="summary-title" aria-busy={!ready && !error}>
        <h2 id="summary-title">Resumo dos filtros selecionados</h2>
        <p className="summary-scope">Os totais consideram os filtros selecionados e incluem os sinistros que não podem ser localizados no mapa.</p>
        {!ready ? <>
            <div className="statistics-state" role="status">{error ? <><p>Não foi possível carregar as estatísticas.</p><button onClick={onRetry}>Tentar novamente</button></> : <p>Carregando estatísticas…</p>}</div>
            {!error && <div className="atlas-stat-skeletons" aria-hidden="true">{[0,1,2,3].map(key=><Skeleton key={key} className="atlas-stat-skeleton"/>)}</div>}
        </> : <>
            <div ref={reveal} className="atlas-stat-reveal"><StatCards className="atlas-stat-cards" stats={[
                { icon: CarFront, label: 'Sinistros', value: <CountUp value={stats.total}/>, unit: 'eventos', description: 'Total dos filtros selecionados' },
                { icon: AlertCircle, label: 'Sinistros fatais', value: <CountUp value={stats.fatal_events}/>, unit: 'eventos', description: `${percentage(stats.fatal_events)}% dos sinistros da consulta` },
                { icon: Users, label: 'Vítimas fatais', value: <CountUp value={stats.fatal}/>, unit: 'pessoas', description: 'Pessoas que morreram nos sinistros' },
                { icon: MapPin, label: 'Localizáveis no mapa', value: <CountUp value={stats.located}/>, unit: 'eventos', description: `${percentage(stats.located)}% dos sinistros da consulta` },
            ]}/></div>
            {stats.total === 0 ? <p className="empty-statistics">Nenhum sinistro corresponde aos filtros selecionados.</p> : <div ref={reveal} className="bottom-grid">
                <Suspense fallback={<div className="atlas-chart-placeholder" role="status">Carregando gráfico…</div>}>
                    <MonthlyChart key={JSON.stringify(filters ?? year)} months={stats.months}
                        filters={filters ?? { ano: year, municipio: '', tipo: '', via: '', transporte: '', registro: '', inicio: '', fim: '' }}
                        dateStart={String(meta?.date_start ?? 20150101).replace(/^(\d{4})(\d{2})(\d{2})$/, '$1-$2-$3')} dateEnd={dateEnd}/>
                </Suspense>
                <section className="types-panel"><h3>Tipos de sinistro</h3><div className="type-list">{meta?.types.map((name: string, i: number) => ({ name, n: stats.types[i] })).sort((a: any, b: any) => b.n - a.n).map((item: any, i: number) => <div className="type-row" key={item.name} style={{ '--i': i } as CSSProperties}><span className="type-name">{item.name}<i style={{ width: `${item.n / stats.total * 100}%` }}/></span><b>{fmt(item.n)}</b><small>{percentage(item.n)}%</small></div>)}</div></section>
            </div>}
        </>}
    </section>;
}
