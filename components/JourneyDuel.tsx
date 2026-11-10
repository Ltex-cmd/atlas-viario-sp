import { useEffect, useRef, useState } from 'react';
import { X } from './icons';
import { CountUp } from './motion';
import { distanceLabel, durationLabel, type Journey, type Place, type TravelMode } from '../lib/journey';
import { duelHeadline, duelLabel, modeLabels, type DuelEntry } from '../lib/journey-insights';
import { apiURL } from '../lib/api-url';

export default function JourneyDuel({ active, journey, from, to, total, radius, filtersKey, entries, onEntries, onShow }: {
    active: boolean; journey: Journey | null; from: Place | null; to: Place | null; total: number;
    radius: number; filtersKey: string; entries: DuelEntry[];
    onEntries: (entries: DuelEntry[]) => void; onShow: (entry: DuelEntry) => void;
}) {
    const [busy, setBusy] = useState(false);
    const abort = useRef<AbortController | null>(null);
    const latest = useRef(entries); latest.current = entries;
    const update = (next: DuelEntry[]) => { latest.current = next; onEntries(next); };
    useEffect(() => {
        abort.current?.abort(); setBusy(false);
        return () => abort.current?.abort();
    }, [filtersKey, radius]);
    useEffect(() => { if (!journey) { abort.current?.abort(); setBusy(false); onEntries(latest.current.filter(entry => entry.total !== undefined || entry.error)); } }, [journey, onEntries]);
    const currentGeometry = journey ? JSON.stringify(journey.coordinates) : '';
    const same = (entry: DuelEntry) => entry.journey && JSON.stringify(entry.journey.coordinates) === currentGeometry && entry.mode === journey?.mode;
    const save = () => {
        if (!journey || !from || !to || latest.current.length >= 4 || latest.current.some(same)) return;
        update([...latest.current, { id: crypto.randomUUID(), journey, mode: journey.mode, from, to, total }]);
    };
    const compare = async () => {
        if (!from || !to || !journey || busy) return;
        save();
        const controller = new AbortController(); abort.current = controller; setBusy(true);
        const modes = (['foot', 'bike', 'car'] as TravelMode[]).filter(mode => !latest.current.some(e => e.mode === mode && e.from.coordinates.join() === from.coordinates.join() && e.to.coordinates.join() === to.coordinates.join()));
        const pending = modes.slice(0, 4 - latest.current.length).map(mode => ({ id: crypto.randomUUID(), mode, from, to }));
        update([...latest.current, ...pending]);
        const request = async <T,>(path: string, body?: unknown): Promise<T> => {
            const response = await fetch(apiURL(path), { signal: controller.signal, ...(body === undefined ? {} : { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }) });
            const data = await response.json() as T & { error?: string };
            if (!response.ok) throw Error(data.error || 'Não foi possível calcular este caminho.');
            return data;
        };
        for (const entry of pending) {
            if (controller.signal.aborted) break;
            try {
                const params = new URLSearchParams({ from: from.coordinates.join(','), to: to.coordinates.join(','), mode: entry.mode });
                const route: Journey = await request<Journey>('/api/v1/route?' + params);
                const count = await request<{ total: number }>('/api/v1/route-events', { coordinates: route.coordinates, corridor_m: radius, filters: filtersKey, offset: 0, limit: 1, include_map: false });
                if (!controller.signal.aborted) update(latest.current.map(e => e.id === entry.id ? { ...e, journey: route, total: count.total } : e));
            } catch (error) {
                if (!controller.signal.aborted) update(latest.current.map(e => e.id === entry.id ? { ...e, error: error instanceof Error && !(error instanceof TypeError) ? error.message : 'Não foi possível calcular este caminho. Tente novamente.' } : e));
            }
        }
        if (!controller.signal.aborted) setBusy(false);
    };
    const headline = duelHeadline(entries);
    return <section className="journey-insight" hidden={!active} aria-label="Duelo de caminhos">
        <h3>Duelo de caminhos</h3>
        <div className="journey-duel-actions">
            <button className="journey-text-button" disabled={busy || entries.length >= 4 || entries.some(same)} onClick={save}>Guardar este caminho para comparar</button>
            <button className="journey-text-button" disabled={busy || entries.length >= 4} onClick={() => void compare()}>Comparar a pé, bicicleta e carro</button>
        </div>
        <div aria-live="polite">{headline && <p>{headline}</p>}</div>
        {headline && <p className="journey-note">Caminhos mais longos tendem a ter mais registros; veja também o valor por km.</p>}
        <div className="journey-duel-grid" aria-live="polite">{entries.map((entry, index) => <article className="journey-duel-card" key={entry.id}>
            <div className="journey-duel-heading"><h4><span className={`journey-duel-mark duel-color-${index % 2}`} aria-hidden="true"/>{modeLabels[entry.mode]}{duelLabel(entries, index).startsWith('o caminho') && ` · Caminho ${index + 1}`}</h4><button className="journey-text-button" disabled={busy} aria-label={`Remover caminho ${index + 1}`} onClick={() => update(entries.filter(e => e.id !== entry.id))}><X size={18}/></button></div>
            <p className="journey-duel-endpoints" title={`${entry.from.label} → ${entry.to.label}`}>{entry.from.label} → {entry.to.label}</p>
            {entry.total !== undefined && entry.journey ? <><strong className="journey-counter"><CountUp value={entry.total} announce/>{entry.total === 1 ? ' sinistro' : ' sinistros'}</strong><p>{distanceLabel(entry.journey.distance_m)} · {durationLabel(entry.journey.duration_s)}</p><p>{(entry.journey.distance_m > 0 ? entry.total / (entry.journey.distance_m / 1000) : 0).toLocaleString('pt-BR', { minimumFractionDigits: 1, maximumFractionDigits: 1 })} por km</p>{same(entry) ? <p>Em exibição</p> : <button className="journey-text-button" onClick={() => onShow(entry)}>Ver este caminho</button>}</> : <p role="status">{entry.error || 'Calculando…'}</p>}
        </article>)}</div>
    </section>;
}
