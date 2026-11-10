import { transportBreakdown } from '../lib/journey-insights';
import type { TravelMode } from '../lib/journey';

const labels = ['Pedestre', 'Bicicleta', 'Motocicleta', 'Automóvel', 'Ônibus', 'Caminhão', 'Outros veículos'];
export default function JourneyTransport({ points, total, shown, mode }: { points: { transport: number }[]; total: number; shown: number; mode: TravelMode }) {
    const counts = transportBreakdown(points), max = Math.max(...counts.slice(0, 7), 1);
    const denominator = total > shown ? shown : total;
    const bit = { foot: 0, bike: 1, car: 3 }[mode], noun = { foot: 'pedestres', bike: 'bicicletas', car: 'automóveis' }[mode];
    const n = (value: number) => value.toLocaleString('pt-BR');
    return <section className="journey-insight" aria-label="Quem se envolveu">
        <h3>Quem se envolveu</h3>
        <p>{counts[bit] ? `${n(counts[bit])} de ${n(denominator)} envolveram ${noun}.` : `Nenhum dos ${n(denominator)} envolveu ${noun}.`}</p>
        <ul className="journey-transports">{labels.map((label, bit) => counts[bit] > 0 && <li key={label}>
            <span>{label}</span><div className="journey-bar" aria-hidden="true"><div style={{ width: `${counts[bit] / max * 100}%` }}/></div><span>{n(counts[bit])} de {n(denominator)}</span>
        </li>)}</ul>
        <p className="journey-note">Um sinistro pode envolver mais de um modo, então as barras não somam o total. {total > shown && `Calculado sobre os primeiros ${n(shown)} registros.`} {counts[7] > 0 && `${n(counts[7])} registros têm modo não disponível.`}</p>
    </section>;
}
