import type { CSSProperties } from 'react';
import { CountUp, reveal } from './motion';

const fmt = (n: number) => n.toLocaleString('pt-BR');
const months = ['Jan', 'Fev', 'Mar', 'Abr', 'Mai', 'Jun', 'Jul', 'Ago', 'Set', 'Out', 'Nov', 'Dez'];

export default function StatisticsPanel({ stats, ready, error, onRetry, meta, year, dateEnd, partial }: {
    stats: any; ready: boolean; error: string; onRetry: () => void; meta: any; year: string; dateEnd: string; partial: boolean;
}) {
    const percentage = (n: number) => (stats.total ? n / stats.total * 100 : 0).toLocaleString('pt-BR', { maximumFractionDigits: 1 });
    return <section className="summary-section" aria-labelledby="summary-title" aria-busy={!ready && !error}>
        <h2 id="summary-title">Resumo dos filtros selecionados</h2>
        <p className="summary-scope">Os totais consideram os filtros selecionados e incluem os sinistros que não podem ser localizados no mapa.</p>
        {!ready ? <>
            <div className="statistics-state" role="status">{error ? <><p>Não foi possível carregar as estatísticas.</p><button onClick={onRetry}>Tentar novamente</button></> : <p>Carregando estatísticas…</p>}</div>
            {!error && <div className="statistics-placeholder" aria-hidden="true"><div/><div/><div/><div/></div>}
        </> : <>
            <div ref={reveal} className="stats-grid" aria-label="Indicadores da consulta">
                <article><div className="stat-label">Sinistros</div><div className="stat-value"><CountUp value={stats.total}/><span className="unit">eventos</span></div></article>
                <article><div className="stat-label">Sinistros fatais</div><div className="stat-value"><CountUp value={stats.fatal_events}/><span className="unit">eventos</span></div><div className="stat-note">{percentage(stats.fatal_events)}% dos sinistros da consulta</div></article>
                <article><div className="stat-label">Vítimas fatais</div><div className="stat-value"><CountUp value={stats.fatal}/><span className="unit">pessoas</span></div><div className="stat-note">Pessoas que morreram nos sinistros</div></article>
                <article><div className="stat-label">Localizáveis no mapa</div><div className="stat-value"><CountUp value={stats.located}/><span className="unit">eventos</span></div><div className="stat-note">{percentage(stats.located)}% dos sinistros da consulta</div></article>
            </div>
            {stats.total === 0 ? <p className="empty-statistics">Nenhum sinistro corresponde aos filtros selecionados.</p> : <div ref={reveal} className="bottom-grid">
                <section className="chart-panel"><div className="section-title"><h3>Sinistros por mês</h3><span className="muted">{year || 'Todos os anos'}</span></div>
                    <div className="month-chart">{months.map((name, i) => {
                        const n = stats.months[i], max = Math.max(1, ...stats.months);
                        const outsideCoverage = partial && i >= Number(dateEnd.slice(5, 7));
                        return <div key={name} tabIndex={0} style={{ '--i': i } as CSSProperties} className={'month-column ' + (outsideCoverage ? 'no-data' : '')} title={outsideCoverage ? `${name}: fora da cobertura` : `${name}: ${fmt(n)} ${n===1?'sinistro':'sinistros'}`}>
                            <div className="bar-track"><div className="bar" style={{ height: `${Math.max(n ? 3 : 0, n / max * 100)}%` }}><span>{fmt(n)}</span></div></div><span>{name}</span>
                        </div>;
                    })}</div>
                </section>
                <section className="types-panel"><h3>Tipos de sinistro</h3><div className="type-list">{meta?.types.map((name: string, i: number) => ({ name, n: stats.types[i] })).sort((a: any, b: any) => b.n - a.n).map((item: any, i: number) => <div className="type-row" key={item.name} style={{ '--i': i } as CSSProperties}><span className="type-name">{item.name}<i style={{ width: `${item.n / stats.total * 100}%` }}/></span><b>{fmt(item.n)}</b><small>{percentage(item.n)}%</small></div>)}</div></section>
            </div>}
        </>}
    </section>;
}
