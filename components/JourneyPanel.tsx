'use client';
import { Button } from './AtlasControls';
import { apiURL } from '../lib/api-url';
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { createPortal } from 'react-dom';
import type { GeoJSONSource, Map as MapLibreMap } from 'maplibre-gl';
import { ArrowRight, LocateFixed, MapPin, Search, X } from './icons';
import { filterKey, type Filters } from './query-state';
import { distanceLabel, durationLabel, journeyNeedsConfirmation, parseCoordinateInput, routeBounds, validCoordinate, type Coordinate, type Journey, type Place, type TravelMode } from '../lib/journey';

import JourneyComparison, { type RouteComparison } from './JourneyComparison';

import JourneyGuess from './JourneyGuess';
import { CountUp, reducedMotion } from './motion';
import JourneyTransport from './JourneyTransport';
import JourneyDuel from './JourneyDuel';
import { guessFeedback, type DuelEntry } from '../lib/journey-insights';
import { buildComparison, routeLengthByMunicipality, type Rings } from '../lib/route-comparison';
import { routeLength } from '../lib/journey';

let referenceData: Promise<[Record<string, Rings>, Record<string, { main_km: number; local_km: number }>]> | undefined;
function loadReference() {
    return referenceData ??= Promise.all([import('../worker/municipal-polygons.json'), import('../worker/road-network.json')])
        .then(([polygons, network]): [Record<string, Rings>, Record<string, { main_km: number; local_km: number }>] => [polygons.default as Record<string, Rings>, network.default.municipalities])
        .catch(error => { referenceData = undefined; throw error; });
}

type Slot = 'from' | 'to';
type EventRow = { transport: number; id: number; date: number; type: number; latitude: number; longitude: number };
type Selection = { comparison?: RouteComparison; total: number; offset: number; limit: number; items: EventRow[]; has_more: boolean; points?: EventRow[]; map_shown?: number; map_limit: number };
const empty = { type: 'FeatureCollection' as const, features: [] };
const number = (n: number) => n.toLocaleString('pt-BR');
const exampleFrom: Place = { label: 'Ponto na USP São Carlos', coordinates: [-47.89661, -22.00854] };
const exampleTo: Place = { label: 'Mercado Municipal de São Carlos', coordinates: [-47.89139, -22.02000] };
async function responseJSON(url: string, signal: AbortSignal, body?: unknown) {
    const r = await fetch(apiURL(url), body === undefined ? { signal } : { signal, method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) });
    const data: any = await r.json();
    if (!r.ok) throw Error(data.error || 'Não foi possível concluir a consulta. Tente novamente.');
    return data;
}
function updateSource(map: MapLibreMap, id: string, data: any) { if (map.getStyle()) (map.getSource(id) as GeoJSONSource | undefined)?.setData(data); }

export default function JourneyPanel({ map, filters, period, meta, onDetail, onPickingChange }: {
    map: MapLibreMap | null; filters: Filters; period: string; meta: any;
    onDetail: (id: number) => void; onPickingChange: (picking: boolean) => void;
}) {
    const [from, setFrom] = useState<Place | null>(null), [to, setTo] = useState<Place | null>(null);
    const [texts, setTexts] = useState({ from: '', to: '' });
    const [mode, setMode] = useState<TravelMode>('foot'), [radius, setRadius] = useState(50);
    const [picking, setPicking] = useState<Slot | null>(null), [route, setRoute] = useState<Journey | null>(null);
    const [proposal, setProposal] = useState<Journey | null>(null);
    const displayRoute = route ?? proposal;
    const [routeBusy, setRouteBusy] = useState(false), [routeError, setRouteError] = useState('');
    const [bandError, setBandError] = useState(false);
    const [selection, setSelection] = useState<Selection | null>(null), [selectionKey, setSelectionKey] = useState('');
    const [countBusy, setCountBusy] = useState(false), [countError, setCountError] = useState(''), [retry, setRetry] = useState(0);
    const [guessEnabled, setGuessEnabled] = useState(() => { try { return localStorage.getItem('atlas-journey-guess') === 'on'; } catch { return false; } });
    const [revealed, setRevealed] = useState<Record<string, number | null>>({});
    const drawnRoute = useRef('');
    const [duelEntries, setDuelEntries] = useState<DuelEntry[]>([]);
    const [fallback, setFallback] = useState<{ key: string; comparison: RouteComparison } | null>(null);
    const [listOpen, setListOpen] = useState(false);
    const [lookup, setLookup] = useState<{ slot: Slot; busy: boolean; places: Place[]; error: string } | null>(null);
    const routeAbort = useRef<AbortController | null>(null), countAbort = useRef<AbortController | null>(null), searchAbort = useRef<AbortController | null>(null);
    const pickingRef = useRef<Slot | null>(null), detailsRef = useRef(onDetail);
    detailsRef.current = onDetail;
    const filtersKey = filterKey(filters);
    const geometryKey = useMemo(() => route ? JSON.stringify(route.coordinates) : '', [route]);
    const currentKey = `${filtersKey}|${radius}|${geometryKey}`;
    const ready = !!selection && selectionKey === currentKey && !countBusy;
    const hiddenAnswer = guessEnabled && !(geometryKey in revealed);
    const visible = ready && !hiddenAnswer;
    const comparison = selection?.comparison || (fallback?.key === currentKey ? fallback.comparison : undefined);
    useEffect(() => { setDuelEntries([]); }, [filtersKey, radius]);
    useEffect(() => {
        if (!ready || !route || selection.comparison !== undefined) return;
        const controller = new AbortController();
        setFallback(null);
        const unavailable: RouteComparison = { status: 'unavailable', reason: 'no_reference', observed: selection.total, route_km: routeLength(route.coordinates) / 1000, level: null, municipalities: [] };
        void (async () => {
            try {
                const [polygons, roads] = await loadReference();
                if (controller.signal.aborted) return;
                const lengths = routeLengthByMunicipality(route.coordinates, polygons);
                const all = Object.entries(lengths.metres).filter(([, metres]) => metres >= 1).sort((a, b) => b[1] - a[1]);
                if (all.length > 6) { setFallback({ key: currentKey, comparison: unavailable }); return; }
                const selected = all.filter(([code]) => !filters.municipio || code === filters.municipio);
                const counts: Record<string, number> = {};
                for (const [code] of selected) {
                    const params = new URLSearchParams(filtersKey); params.set('municipio', code);
                    const data = await responseJSON('/api/v1/statistics?' + params, controller.signal);
                    if (!Number.isFinite(data.located)) throw Error('Referência indisponível');
                    counts[code] = data.located;
                }
                if (!controller.signal.aborted) setFallback({ key: currentKey, comparison: buildComparison({ lengths, selected, counts, route_m: routeLength(route.coordinates), outsideFilter: !!filters.municipio && !selected.length }, selection.total, radius, meta?.cities || [], roads) });
            } catch { if (!controller.signal.aborted) setFallback({ key: currentKey, comparison: unavailable }); }
        })();
        return () => controller.abort();
    }, [ready, route, selection?.comparison, selection?.total, currentKey, filtersKey, filters.municipio, radius, meta]);
    const pick = useCallback((slot: Slot | null) => { pickingRef.current = slot; setPicking(slot); onPickingChange(slot !== null); }, [onPickingChange]);
    const invalidate = useCallback(() => {
        routeAbort.current?.abort(); countAbort.current?.abort();
        setRoute(null); setProposal(null); setSelection(null); setRouteBusy(false); setCountBusy(false); setRouteError(''); setCountError(''); setListOpen(false);
    }, []);
    const choose = useCallback((slot: Slot, place: Place) => {
        searchAbort.current?.abort(); invalidate();
        (slot === 'from' ? setFrom : setTo)(place);
        setTexts(t => ({ ...t, [slot]: place.label })); setLookup(null); pick(null);
    }, [invalidate, pick]);
    const chooseRef = useRef(choose); chooseRef.current = choose;
    useEffect(() => {
        if (!map?.getStyle()) return;
        map.addSource('journey-duel', { type: 'geojson', data: empty });
        map.addSource('journey-line', { type: 'geojson', data: empty });
        map.addSource('journey-corridor', { type: 'geojson', data: empty, tolerance: 0 });
        map.addSource('journey-events', { type: 'geojson', data: empty });
        map.addSource('journey-endpoints', { type: 'geojson', data: empty });
        map.addLayer({ id: 'journey-band', type: 'fill', source: 'journey-corridor', paint: { 'fill-color': '#2d7164', 'fill-opacity': 0.16, 'fill-antialias': false } });
        map.addLayer({ id: 'journey-halo', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'route'], layout: { 'line-cap': 'round', 'line-join': 'round' }, paint: { 'line-color': '#ffffff', 'line-width': 8 } });
        const colors = getComputedStyle(document.documentElement);
        map.addLayer({ id: 'journey-duel', type: 'line', source: 'journey-duel', paint: { 'line-color': ['case', ['==', ['get', 'color'], 0], colors.getPropertyValue('--journey-duel-1').trim(), colors.getPropertyValue('--journey-duel-2').trim()], 'line-width': 2, 'line-dasharray': [3, 2] } });
        map.addLayer({ id: 'journey-path', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'route'], layout: { 'line-cap': 'round', 'line-join': 'round' }, paint: { 'line-color': '#2d7164', 'line-width': 4 } });
        map.addLayer({ id: 'journey-connectors', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'connector'], paint: { 'line-color': '#2d7164', 'line-width': 2, 'line-dasharray': [2, 2] } });
        map.addLayer({ id: 'journey-events', type: 'circle', source: 'journey-events', paint: { 'circle-color': '#2d7164', 'circle-radius': 0, 'circle-radius-transition': { duration: 600, delay: 0 }, 'circle-stroke-color': '#ffffff', 'circle-stroke-width': 0, 'circle-stroke-width-transition': { duration: 600, delay: 0 } } });
        map.addLayer({ id: 'journey-hit', type: 'circle', source: 'journey-events', paint: { 'circle-radius': 12, 'circle-opacity': 0 } });
        map.addLayer({ id: 'journey-endpoints', type: 'circle', source: 'journey-endpoints', paint: { 'circle-color': '#ffffff', 'circle-radius': 14, 'circle-stroke-color': '#2d7164', 'circle-stroke-width': 3 } });
        map.addLayer({ id: 'journey-labels', type: 'symbol', source: 'journey-endpoints', layout: { 'text-field': ['get', 'label'], 'text-size': 14, 'text-font': ['Montserrat SemiBold', 'Open Sans Bold', 'Noto Sans Regular'], 'text-allow-overlap': true }, paint: { 'text-color': '#2d7164' } });
        const click = (e: any) => {
            const slot = pickingRef.current;
            if (slot) {
                const coordinates: Coordinate = [e.lngLat.lng, e.lngLat.lat];
                if (validCoordinate(coordinates)) chooseRef.current(slot, { label: `Ponto no mapa · ${coordinates[1].toFixed(5)}, ${coordinates[0].toFixed(5)}`, coordinates });
                return;
            }
            const features = map.queryRenderedFeatures(e.point, { layers: ['journey-hit'] });
            const id = Number(features[0]?.properties?.id);
            if (id) detailsRef.current(id);
        };
        map.on('click', click);
        return () => {
            map.off('click', click);
            // Atlas may already have disposed the map during navigation or refresh.
            if (!map.getStyle()) return;
            for (const id of ['journey-duel', 'journey-labels', 'journey-endpoints', 'journey-hit', 'journey-events', 'journey-connectors', 'journey-path', 'journey-halo', 'journey-band']) if (map.getLayer(id)) map.removeLayer(id);
            for (const id of ['journey-duel', 'journey-endpoints', 'journey-events', 'journey-line', 'journey-corridor']) if (map.getSource(id)) map.removeSource(id);
            map.getCanvas().style.cursor = '';
        };
    }, [map]);
    useEffect(() => {
        if (map) {
            map.getCanvas().style.cursor = picking ? 'crosshair' : '';
            if (picking) map.getContainer().scrollIntoView({ block: 'center', behavior: matchMedia('(prefers-reduced-motion: reduce)').matches ? 'instant' : 'smooth' });
        }
        const escape = (e: KeyboardEvent) => { if (e.key === 'Escape' && picking) pick(null); };
        document.addEventListener('keydown', escape);
        return () => document.removeEventListener('keydown', escape);
    }, [map, picking, pick]);
    useEffect(() => () => { routeAbort.current?.abort(); countAbort.current?.abort(); searchAbort.current?.abort(); onPickingChange(false); }, [onPickingChange]);
    useEffect(() => {
        if (!map?.getStyle()) return;
        updateSource(map, 'journey-endpoints', { type: 'FeatureCollection', features: [from, to].flatMap((p, i) => p ? [{ type: 'Feature', properties: { label: i ? 'B' : 'A' }, geometry: { type: 'Point', coordinates: p.coordinates } }] : []) });
        const features: any[] = displayRoute ? [{ type: 'Feature', properties: { kind: 'route' }, geometry: { type: 'LineString', coordinates: displayRoute.coordinates } }] : [];
        if (displayRoute) {
            [from, to].forEach((place, i) => { if (place && displayRoute.waypoints[i].distance_m > 5) features.push({ type: 'Feature', properties: { kind: 'connector' }, geometry: { type: 'LineString', coordinates: [place.coordinates, displayRoute.waypoints[i].coordinates] } }); });
        }
        if (route && drawnRoute.current !== geometryKey && !reducedMotion()) {
            // A new route is drawn from A to B at constant speed along its length.
            drawnRoute.current = geometryKey;
            const line = route.coordinates, along = [0];
            for (let i = 1; i < line.length; i++) along.push(along[i - 1] + Math.hypot((line[i][0] - line[i - 1][0]) * Math.cos(line[i][1] * Math.PI / 180), line[i][1] - line[i - 1][1]));
            const total = along[line.length - 1], begin = performance.now(), duration = 1100;
            let frame = 0, index = 0;
            const step = (now: number) => {
                if (!map.getStyle()) return;
                const t = Math.min(1, (now - begin) / duration), reached = total * (1 - (1 - t) ** 3);
                while (index < line.length - 2 && along[index + 1] < reached) index++;
                const a = line[index], b = line[index + 1], span = along[index + 1] - along[index], f = span ? Math.min(1, (reached - along[index]) / span) : 1;
                const partial = [...line.slice(0, index + 1), [a[0] + (b[0] - a[0]) * f, a[1] + (b[1] - a[1]) * f]];
                updateSource(map, 'journey-line', { type: 'FeatureCollection', features: t < 1 ? [{ type: 'Feature', properties: { kind: 'route' }, geometry: { type: 'LineString', coordinates: partial } }] : features });
                if (t < 1) frame = requestAnimationFrame(step);
            };
            updateSource(map, 'journey-line', { type: 'FeatureCollection', features: [] });
            frame = requestAnimationFrame(step);
            return () => cancelAnimationFrame(frame);
        }
        if (!route) drawnRoute.current = '';
        updateSource(map, 'journey-line', { type: 'FeatureCollection', features });
    }, [map, displayRoute, from, to]);
    useEffect(() => {
        if (!map?.getStyle()) return;
        let cancelled = false;
        setBandError(false);
        updateSource(map, 'journey-corridor', empty);
        if (!displayRoute) return;
        // A fresh worker can be terminated even while a complex union is running.
        // Keep JSTS out of the main bundle and never block map/input interaction.
        let worker: Worker | undefined;
        try {
            worker = new Worker(new URL('../lib/journey-corridor.worker.ts', import.meta.url), { type: 'module' });
            worker.onmessage = event => {
                if (!cancelled) updateSource(map, 'journey-corridor', event.data);
                worker?.terminate();
            };
            worker.onerror = () => { if (!cancelled) setBandError(true); worker?.terminate(); };
            worker.postMessage({ coordinates: displayRoute.coordinates, radius });
        } catch { worker?.terminate(); setBandError(true); }
        return () => { cancelled = true; worker?.terminate(); };
    }, [map, displayRoute, radius]);
    useEffect(() => {
        if (!map?.getStyle()) return;
        updateSource(map, 'journey-events', visible ? { type: 'FeatureCollection', features: (selection?.points || []).map(p => ({ type: 'Feature', properties: { id: p.id }, geometry: { type: 'Point', coordinates: [p.longitude, p.latitude] } })) } : empty);
        // The points grow in when the answer is shown.
        map.setPaintProperty('journey-events', 'circle-radius', visible ? 6 : 0);
        map.setPaintProperty('journey-events', 'circle-stroke-width', visible ? 2 : 0);
    }, [map, visible, selection]);
    useEffect(() => {
        if (!map?.getStyle()) return;
        updateSource(map, 'journey-duel', { type: 'FeatureCollection', features: visible ? duelEntries.flatMap((entry, index) => entry.journey && JSON.stringify(entry.journey.coordinates) !== geometryKey ? [{ type: 'Feature', properties: { color: index % 2 }, geometry: { type: 'LineString', coordinates: entry.journey.coordinates } }] : []) : [] });
    }, [map, duelEntries, geometryKey, visible]);
    const count = useCallback(async (journey: Journey, offset: number, includeMap: boolean) => {
        countAbort.current?.abort(); const controller = new AbortController(); countAbort.current = controller;
        setCountBusy(true); setCountError('');
        try {
            const data: Selection = await responseJSON('/api/v1/route-events', controller.signal, { coordinates: journey.coordinates, corridor_m: radius, filters: filtersKey, offset, limit: 30, include_map: includeMap });
            if (!controller.signal.aborted) {
                setSelection(previous => includeMap ? data : { ...data, points: previous?.points, map_shown: previous?.map_shown, comparison: previous?.comparison });
                setSelectionKey(currentKey);
            }
        } catch (e: any) { if (e.name !== 'AbortError') setCountError(e.message || 'Não foi possível contar os sinistros. Tente novamente.'); }
        finally { if (!controller.signal.aborted) setCountBusy(false); }
    }, [radius, filtersKey, currentKey]);
    useEffect(() => {
        setSelection(null); setListOpen(false);
        if (route) void count(route, 0, true);
        return () => countAbort.current?.abort();
    }, [route, count, retry]);
    const fit = (journey: Journey, origin = from, destination = to) => {
        if (!map) return;
        const b = routeBounds([...journey.coordinates, ...[origin, destination].flatMap(place => place ? [place.coordinates] : [])], Math.max(radius, 100));
        map.fitBounds([[b[0], b[1]], [b[2], b[3]]], { padding: 48, maxZoom: 16, duration: matchMedia('(prefers-reduced-motion: reduce)').matches ? 0 : 500 });
    };
    const trace = async (origin = from, destination = to) => {
        invalidate(); pick(null); setLookup(null);
        if (!origin || !destination) { setRouteError('Escolha a origem e o destino pela busca ou no mapa.'); return; }
        const controller = new AbortController(); routeAbort.current = controller; setRouteBusy(true);
        try {
            const params = new URLSearchParams({ from: origin.coordinates.join(','), to: destination.coordinates.join(','), mode });
            const data: Journey = await responseJSON('/api/v1/route?' + params, controller.signal);
            if (!controller.signal.aborted) {
                if (journeyNeedsConfirmation(data)) setProposal(data); else setRoute(data);
                fit(data, origin, destination);
            }
        } catch (e: any) { if (e.name !== 'AbortError') setRouteError(e.message || 'Não foi possível traçar o percurso. Tente novamente.'); }
        finally { if (!controller.signal.aborted) setRouteBusy(false); }
    };
    const search = async (slot: Slot) => {
        searchAbort.current?.abort(); const controller = new AbortController(); searchAbort.current = controller;
        pick(null); const text = texts[slot].trim();
        const coordinates = parseCoordinateInput(text);
        if (coordinates) { choose(slot, { label: `Ponto no mapa · ${coordinates[1].toFixed(5)}, ${coordinates[0].toFixed(5)}`, coordinates }); return; }
        if (text.length < 3) { setLookup({ slot, busy: false, places: [], error: 'Escreva uma rua, escola ou praça e inclua o município.' }); return; }
        setLookup({ slot, busy: true, places: [], error: '' });
        try {
            const center = map?.getCenter();
            const params = new URLSearchParams({ q: text, ...(center && validCoordinate([center.lng, center.lat]) ? { lon: String(center.lng), lat: String(center.lat) } : {}) });
            const data = await responseJSON('/api/v1/places?' + params, controller.signal);
            if (!controller.signal.aborted) setLookup({ slot, busy: false, places: data.places, error: data.places.length ? '' : 'Nenhum lugar encontrado. Inclua o município ou escolha no mapa.' });
        } catch (e: any) { if (e.name !== 'AbortError') setLookup({ slot, busy: false, places: [], error: e.message || 'A busca falhou. Escolha o ponto no mapa.' }); }
    };
    const edit = (slot: Slot, text: string) => { searchAbort.current?.abort(); invalidate(); setLookup(null); (slot === 'from' ? setFrom : setTo)(null); setTexts(t => ({ ...t, [slot]: text })); };
    const runExample = () => { setFrom(exampleFrom); setTo(exampleTo); setTexts({ from: exampleFrom.label, to: exampleTo.label }); void trace(exampleFrom, exampleTo); };
    const clear = () => { setDuelEntries([]); invalidate(); searchAbort.current?.abort(); pick(null); setFrom(null); setTo(null); setTexts({ from: '', to: '' }); setLookup(null); };

    return <section className="journey-panel" aria-label="Sinistros no percurso">
        <div className="journey-intro"><p>Escolha dois lugares para consultar os sinistros registrados perto do caminho.</p><button className="journey-text-button" disabled={!map || routeBusy} onClick={runExample}>Testar exemplo em São Carlos <ArrowRight size={16}/></button></div>
        <div className="journey-inputs">{(['from', 'to'] as Slot[]).map((slot, i) => <div className="journey-endpoint" key={slot}>
            <form onSubmit={e => { e.preventDefault(); void search(slot); }}>
                <label htmlFor={'journey-' + slot}><span className="journey-letter">{i ? 'B' : 'A'}</span>{i ? 'Destino' : 'Origem'}</label>
                <div className="journey-search"><input id={'journey-' + slot} value={texts[slot]} onChange={e => edit(slot, e.target.value)} placeholder="Rua, lugar ou latitude, longitude" autoComplete="off" maxLength={160}/><button type="submit" disabled={lookup?.busy} aria-label={'Buscar ' + (i ? 'destino' : 'origem')}><Search size={18}/></button><button type="button" disabled={!map} aria-label={'Escolher ' + (i ? 'destino' : 'origem') + ' no mapa'} aria-pressed={picking === slot} onClick={() => pick(picking === slot ? null : slot)}><MapPin size={18}/></button></div>
            </form>
            {lookup?.slot === slot && <div className="journey-search-results" aria-live="polite">{lookup.busy ? <p className="journey-loading-text">Buscando lugares…</p> : lookup.error ? <p role="status">{lookup.error}</p> : <ul aria-label={'Lugares para ' + (i ? 'destino' : 'origem')}>{lookup.places.map((place, index) => <li key={index}><button onClick={() => choose(slot, place)}>{place.label}<ArrowRight size={16}/></button></li>)}</ul>}</div>}
        </div>)}</div>
        <div className="journey-options"><label className="journey-guess-toggle"><input type="checkbox" checked={guessEnabled} onChange={e => { setGuessEnabled(e.target.checked); try { localStorage.setItem('atlas-journey-guess', e.target.checked ? 'on' : 'off'); } catch {} }}/>Chutar antes de ver</label><label>Como ir<select aria-label="Como percorrer o trajeto" value={mode} onChange={e => { invalidate(); setMode(e.target.value as TravelMode); }}><option value="foot">A pé</option><option value="bike">Bicicleta</option><option value="car">Carro</option></select></label><label>Proximidade<select aria-label="Distância dos sinistros até o percurso" value={radius} onChange={e => setRadius(Number(e.target.value))}><option value={25}>Até 25 m</option><option value={50}>Até 50 m</option><option value={100}>Até 100 m</option></select></label><button className="primary-button" disabled={!map || !from || !to || routeBusy} onClick={() => void trace()}>{routeBusy ? 'Traçando percurso…' : 'Traçar percurso'}<ArrowRight size={18}/></button>{(from || to) && <button className="journey-text-button" onClick={clear}>Limpar percurso</button>}</div>
        {picking && map && createPortal(<div className="journey-pick-status" role="status"><span>Toque no mapa para escolher {picking === 'from' ? 'a origem A' : 'o destino B'}.</span><button onClick={() => pick(null)} aria-label="Cancelar escolha no mapa"><X size={18}/></button></div>, map.getContainer())}
        {routeError && <div className="journey-error" role="alert"><p>{routeError}</p><div className="journey-error-actions">
            <Button variant="secondary" size="small" disabled={!map} onClick={() => { setRouteError(''); pick('from'); }}>Ajustar origem no mapa</Button>
            <Button variant="secondary" size="small" disabled={!map} onClick={() => { setRouteError(''); pick('to'); }}>Ajustar destino no mapa</Button>
        </div></div>}
        {proposal && <div className="journey-proposal" role="region" aria-label="Revisar rota encontrada" aria-live="polite">
            <h3>Rota encontrada em vias próximas</h3>
            <p>Para conectar os dois lados, ampliamos a busca por vias. Confira onde o trajeto começa e termina:</p>
            <ul>{proposal.waypoints.map((waypoint, i) => <li key={i}><strong>{i ? 'Destino B' : 'Origem A'}:</strong> via a {distanceLabel(waypoint.distance_m)} do ponto escolhido.</li>)}</ul>
            <p><strong>{distanceLabel(proposal.distance_m)} · {durationLabel(proposal.duration_s)}</strong> nas vias do traçado.</p>
            <p>As linhas pontilhadas mostram a distância até os pontos, sem confirmar um caminho de acesso. Elas não entram na distância, no tempo ou na contagem de sinistros.</p>
            <div className="journey-error-actions">
                <Button size="small" onClick={() => { setRoute(proposal); setProposal(null); }}>Usar este traçado</Button>
                <Button variant="secondary" size="small" disabled={!map} onClick={() => { invalidate(); pick('from'); }}>Ajustar origem no mapa</Button>
                <Button variant="secondary" size="small" disabled={!map} onClick={() => { invalidate(); pick('to'); }}>Ajustar destino no mapa</Button>
            </div>
        </div>}
        {route && <div className="journey-result" aria-live="polite"><div>{ready && hiddenAnswer ? <JourneyGuess key={geometryKey} radius={radius} period={period} onReveal={guess => setRevealed(previous => ({ ...previous, [geometryKey]: guess ?? null }))}/> : <><strong className={countBusy ? 'journey-counter pending-counter' : 'journey-counter'}>{ready ? <><CountUp value={selection.total} announce/>{selection.total === 1 ? ' sinistro' : ' sinistros'}</> : countError ? 'Contagem indisponível' : 'Contando sinistros…'}</strong><p>Até {radius} m do percurso · {period}</p>{visible && revealed[geometryKey] != null && <p>{guessFeedback(revealed[geometryKey], selection.total)}</p>}</>}</div><div className="journey-metrics"><span>{distanceLabel(route.distance_m)}</span><span>{durationLabel(route.duration_s)} estimados</span></div><div className="journey-result-actions"><button className="journey-text-button" onClick={() => fit(route)}><LocateFixed size={16}/>Ver percurso</button>{visible && selection.total > 0 && <button className="journey-text-button" onClick={() => setListOpen(v => !v)} aria-expanded={listOpen}>{listOpen ? 'Ocultar registros' : 'Ver registros'}<ArrowRight size={16}/></button>}</div></div>}
        {visible && comparison && <JourneyComparison comparison={comparison}/>}
        {visible && selection.total > 0 && <JourneyTransport points={selection.points || []} shown={selection.map_shown || 0} total={selection.total} mode={route!.mode}/>}
        <JourneyDuel active={visible} journey={route} from={from} to={to} total={selection?.total || 0} radius={radius} filtersKey={filtersKey} entries={duelEntries} onEntries={setDuelEntries} onShow={entry => {
            if (!entry.journey) return;
            routeAbort.current?.abort(); setRouteBusy(false); setRouteError(''); pick(null);
            setFrom(entry.from); setTo(entry.to); setTexts({ from: entry.from.label, to: entry.to.label }); setMode(entry.mode);
            setRevealed(previous => ({ ...previous, [JSON.stringify(entry.journey!.coordinates)]: null })); setRoute(entry.journey); fit(entry.journey);
        }}/>
        {countError && <div className="journey-error" role="alert"><p>{countError}</p><button className="journey-text-button" onClick={() => setRetry(n => n + 1)}>Tentar contagem novamente</button></div>}
        {bandError && <p className="journey-note" role="status">Não foi possível desenhar a faixa sombreada. A contagem continua usando a distância de proximidade selecionada.</p>}
        {visible && selection.total === 0 && <p className="journey-note">Nenhum sinistro com localização disponível foi encontrado nessa faixa e com os filtros aplicados. Isso não significa ausência de risco.</p>}
        {!hiddenAnswer && listOpen && selection && <div className="journey-records" aria-busy={countBusy}><div className="journey-records-heading"><h3>Registros próximos ao percurso</h3><span>{ready ? `${number(selection.offset + 1)} a ${number(selection.offset + selection.items.length)} de ${number(selection.total)}` : 'Carregando registros…'}</span></div>{ready && <ul>{selection.items.map(row => <li key={row.id}><button onClick={() => onDetail(row.id)}><span><time>{String(row.date).slice(6, 8)}/{String(row.date).slice(4, 6)}/{String(row.date).slice(0, 4)}</time><span>{meta?.types?.[row.type] || 'Sinistro'} · #{row.id}</span></span><ArrowRight size={18}/></button></li>)}</ul>}<div className="journey-pagination"><button disabled={!ready || selection.offset === 0} onClick={() => route && void count(route, Math.max(0, selection.offset - selection.limit), false)}>Anteriores</button><button disabled={!ready || !selection.has_more} onClick={() => route && void count(route, selection.offset + selection.limit, false)}>Próximos</button></div></div>}
        <details className="journey-about"><summary>Sobre a contagem e o mapa</summary>
            <p className="journey-note">Os filtros aplicados também valem para o percurso. A faixa sombreada indica a proximidade consultada; os pontos mantêm as coordenadas da fonte. A contagem e a comparação não medem a segurança do caminho.</p>
            {visible && selection.total > (selection.map_shown || 0) && <p className="journey-note">O mapa mostra os primeiros {number(selection.map_shown || 0)} de {number(selection.total)} registros. A contagem inclui todos e a lista permite consultar os demais.</p>}
            {route && route.waypoints.some(w => w.distance_m > 5) && <p className="journey-note">A rota foi ajustada à rede de ruas. {route.waypoints.map((w, i) => w.distance_m > 5 ? `${i ? 'Destino B' : 'Origem A'} a ${distanceLabel(w.distance_m)} do ponto escolhido.` : '').filter(Boolean).join(' ')} As linhas pontilhadas não confirmam um caminho de acesso e não entram na distância, no tempo ou na contagem de sinistros.</p>}
        </details>
        <div className="journey-attribution"><span>Rotas: <a href="https://routing.openstreetmap.de/about.html" target="_blank" rel="noreferrer">OSRM / FOSSGIS</a> · Busca: <a href="https://photon.komoot.io" target="_blank" rel="noreferrer">Photon</a> · Dados: <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noreferrer">© OpenStreetMap</a></span><a href="https://www.openstreetmap.org/fixthemap" target="_blank" rel="noreferrer">Corrigir o mapa</a><details><summary>Sobre a busca e as rotas</summary><p>Os serviços externos recebem os lugares pesquisados e os pontos do percurso. O operador de rotas registra as consultas. Este teste usa serviços públicos com limites de uso e depende da disponibilidade deles.</p></details></div>
    </section>;
}
