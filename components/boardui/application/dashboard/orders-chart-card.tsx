'use client';
// Adapted from BoardUI's free Orders Chart Card (MIT): real observations,
// Portuguese labels, nullable coverage and no fabricated demo comparisons.
import { useState } from 'react';
import { useReducedMotion } from 'motion/react';
import { Bar, BarChart, ResponsiveContainer, Tooltip, XAxis, YAxis } from 'recharts';
import { Chip } from '@/components/boardui/base/badges/chip';
import { cx } from '@/utils/boardui/cx';
import { CountUp } from '@/components/motion';

export type OrdersPoint = { label: string; current: number | null; previous: number | null };
const fmt = (n: number) => n.toLocaleString('pt-BR');
export function OrdersChartCard({ data, title = 'Sinistros por mês', currentLabel, previousLabel, className }: {
    data: OrdersPoint[]; title?: string; currentLabel: string; previousLabel?: string; className?: string;
}) {
    const [activeIndex, setActiveIndex] = useState<number | null>(null);
    const reducedMotion = useReducedMotion();
    const point = activeIndex === null ? null : data[activeIndex];
    const total = data.reduce((sum, p) => sum + (p.current ?? 0), 0);
    const previous = data.reduce((sum, p) => sum + (p.previous ?? 0), 0);
    const value = point ? point.current : total;
    const comparison = point ? point.previous : previous;
    const delta = value !== null && comparison !== null && comparison > 0 ? (value - comparison) / comparison * 100 : null;
    return <section className={cx('atlas-monthly-card', className)} aria-label={title}>
        <div className="atlas-chart-heading">
            <div><h3>{point ? `${title} · ${point.label}` : title}</h3>
                <div className="atlas-chart-value"><strong>{value === null ? 'Sem cobertura' : <CountUp value={value} duration={500}/>}</strong>
                    {previousLabel && delta !== null && <Chip variant="subtle" color={delta > 0 ? 'rose' : 'neutral'}>{delta > 0 ? '+' : ''}{delta.toLocaleString('pt-BR', { maximumFractionDigits: 1 })}%</Chip>}
                </div>
                <p>{previousLabel ? (comparison === null ? 'Mês fora do período comparado' : `${fmt(comparison)} no período equivalente de ${previousLabel}`) : 'Eventos nos filtros selecionados'}</p>
            </div>
            <div className="atlas-chart-legend"><span><i/>{currentLabel}</span>{previousLabel && <span><i className="previous"/>{previousLabel}</span>}</div>
        </div>
        <div className="atlas-monthly-plot">
            <ResponsiveContainer width="100%" height="100%" minWidth={0}>
                <BarChart data={data} margin={{ top: 12, right: 8, bottom: 0, left: 0 }} barCategoryGap="24%" barGap={3}
                    onMouseMove={state => { const index = Number(state?.activeTooltipIndex); if (state?.isTooltipActive && Number.isInteger(index)) setActiveIndex(index); }}
                    onMouseLeave={() => setActiveIndex(null)} accessibilityLayer>
                    <YAxis width={48} tickFormatter={n => new Intl.NumberFormat('pt-BR', { notation: 'compact' }).format(n)} tickLine={false} axisLine={false} tick={{ fontSize: 12, fill: 'var(--muted)' }}/>
                    <XAxis dataKey="label" tickLine={false} axisLine={false} tickMargin={12} tick={{ fontSize: 12, fill: 'var(--muted)' }}/>
                    <Tooltip cursor={{ fill: 'var(--soft)' }} content={({ active, payload, label }) => active && payload?.length ? <div className="atlas-chart-tooltip"><b>{label}</b>{payload.map(item => <p key={String(item.dataKey)}>{item.name}: {typeof item.value === 'number' ? fmt(item.value) : 'Sem cobertura'}</p>)}</div> : null}/>
                    {previousLabel && <Bar dataKey="previous" name={previousLabel} fill="var(--atlas-chart-previous)" radius={[4, 4, 0, 0]} isAnimationActive={!reducedMotion} animationDuration={900} animationEasing="ease-out"/>}
                    <Bar dataKey="current" name={currentLabel} fill="var(--green)" radius={[4, 4, 0, 0]} isAnimationActive={!reducedMotion} animationDuration={900} animationEasing="ease-out"/>
                </BarChart>
            </ResponsiveContainer>
        </div>
        <details className="atlas-monthly-values"><summary>Consultar valores mensais</summary>
            <table><caption>Sinistros por mês nos períodos selecionados</caption><thead><tr><th scope="col">Mês</th><th scope="col">{currentLabel}</th>{previousLabel && <th scope="col">{previousLabel}</th>}</tr></thead>
                <tbody>{data.map(p => <tr key={p.label}><th scope="row">{p.label}</th><td>{p.current === null ? 'Sem cobertura no período' : fmt(p.current)}</td>{previousLabel && <td>{p.previous === null ? 'Sem cobertura no período' : fmt(p.previous)}</td>}</tr>)}</tbody>
            </table>
        </details>
    </section>;
}
