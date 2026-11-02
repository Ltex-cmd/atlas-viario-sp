'use client';
import { Button } from './AtlasControls';
import { apiURL } from '../lib/api-url';
import { useCallback, useEffect, useMemo, useRef, useState } from 'react';
import { createPortal } from 'react-dom';
import type { GeoJSONSource, Map as MapLibreMap } from 'maplibre-gl';
import { ArrowRight, LocateFixed, MapPin, Search, X } from './icons';
import { filterKey, type Filters } from './query-state';
import { distanceLabel, durationLabel, journeyNeedsConfirmation, parseCoordinateInput, routeBounds, validCoordinate, type Coordinate, type Journey, type Place, type TravelMode } from '../lib/journey';

type Slot = 'from' | 'to';
type EventRow = { id: number; date: number; type: number; latitude: number; longitude: number };
type Selection = { total: number; offset: number; limit: number; items: EventRow[]; has_more: boolean; points?: EventRow[]; map_shown?: number; map_limit: number };
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
    const [listOpen, setListOpen] = useState(false);
    const [lookup, setLookup] = useState<{ slot: Slot; busy: boolean; places: Place[]; error: string } | null>(null);
    const routeAbort = useRef<AbortController | null>(null), countAbort = useRef<AbortController | null>(null), searchAbort = useRef<AbortController | null>(null);
    const pickingRef = useRef<Slot | null>(null), detailsRef = useRef(onDetail);
    detailsRef.current = onDetail;
    const filtersKey = filterKey(filters);
    const geometryKey = useMemo(() => route ? JSON.stringify(route.coordinates) : '', [route]);
    const currentKey = `${filtersKey}|${radius}|${geometryKey}`;
    const ready = !!selection && selectionKey === currentKey && !countBusy;
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
        map.addSource('journey-line', { type: 'geojson', data: empty });
        map.addSource('journey-corridor', { type: 'geojson', data: empty, tolerance: 0 });
        map.addSource('journey-events', { type: 'geojson', data: empty });
        map.addSource('journey-endpoints', { type: 'geojson', data: empty });
        map.addLayer({ id: 'journey-band', type: 'fill', source: 'journey-corridor', paint: { 'fill-color': '#2d7164', 'fill-opacity': 0.16, 'fill-antialias': false } });
        map.addLayer({ id: 'journey-halo', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'route'], layout: { 'line-cap': 'round', 'line-join': 'round' }, paint: { 'line-color': '#ffffff', 'line-width': 8 } });
        map.addLayer({ id: 'journey-path', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'route'], layout: { 'line-cap': 'round', 'line-join': 'round' }, paint: { 'line-color': '#2d7164', 'line-width': 4 } });
        map.addLayer({ id: 'journey-connectors', type: 'line', source: 'journey-line', filter: ['==', ['get', 'kind'], 'connector'], paint: { 'line-color': '#2d7164', 'line-width': 2, 'line-dasharray': [2, 2] } });
        map.addLayer({ id: 'journey-events', type: 'circle', source: 'journey-events', paint: { 'circle-color': '#2d7164', 'circle-radius': 6, 'circle-stroke-color': '#ffffff', 'circle-stroke-width': 2 } });
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
            for (const id of ['journey-labels', 'journey-endpoints', 'journey-hit', 'journey-events', 'journey-connectors', 'journey-path', 'journey-halo', 'journey-band']) if (map.getLayer(id)) map.removeLayer(id);
            for (const id of ['journey-endpoints', 'journey-events', 'journey-line', 'journey-corridor']) if (map.getSource(id)) map.removeSource(id);
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
        updateSource(map, 'journey-events', ready ? { type: 'FeatureCollection', features: (selection?.points || []).map(p => ({ type: 'Feature', properties: { id: p.id }, geometry: { type: 'Point', coordinates: [p.longitude, p.latitude] } })) } : empty);
    }, [map, ready, selection]);
    const count = useCallback(async (journey: Journey, offset: number, includeMap: boolean) => {
        countAbort.current?.abort(); const controller = new AbortController(); countAbort.current = controller;
        setCountBusy(true); setCountError('');
        try {
            const data: Selection = await responseJSON('/api/v1/route-events', controller.signal, { coordinates: journey.coordinates, corridor_m: radius, filters: filtersKey, offset, limit: 30, include_map: includeMap });
            if (!controller.signal.aborted) {
                setSelection(previous => includeMap ? data : { ...data, points: previous?.points, map_shown: previous?.map_shown });
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
    const clear = () => { invalidate(); searchAbort.current?.abort(); pick(null); setFrom(null); setTo(null); setTexts({ from: '', to: '' }); setLookup(null); };

    return <section className="journey-panel" aria-label="Sinistros no percurso">
        <div className="journey-intro"><p>Escolha dois lugares para consultar os sinistros registrados perto do caminho.</p><button className="journey-text-button" disabled={!map || routeBusy} onClick={runExample}>Testar exemplo em São Carlos <ArrowRight size={16}/></button></div>
        <div className="journey-inputs">{(['from', 'to'] as Slot[]).map((slot, i) => <div className="journey-endpoint" key={slot}>
            <form onSubmit={e => { e.preventDefault(); void search(slot); }}>
                <label htmlFor={'journey-' + slot}><span className="journey-letter">{i ? 'B' : 'A'}</span>{i ? 'Destino' : 'Origem'}</label>
                <div className="journey-search"><input id={'journey-' + slot} value={texts[slot]} onChange={e => edit(slot, e.target.value)} placeholder="Rua, lugar ou latitude, longitude" autoComplete="off" maxLength={160}/><button type="submit" disabled={lookup?.busy} aria-label={'Buscar ' + (i ? 'destino' : 'origem')}><Search size={18}/></button><button type="button" disabled={!map} aria-label={'Escolher ' + (i ? 'destino' : 'origem') + ' no mapa'} aria-pressed={picking === slot} onClick={() => pick(picking === slot ? null : slot)}><MapPin size={18}/></button></div>
            </form>
            {lookup?.slot === slot && <div className="journey-search-results" aria-live="polite">{lookup.busy ? <p className="journey-loading-text">Buscando lugares…</p> : lookup.error ? <p role="status">{lookup.error}</p> : <ul aria-label={'Lugares para ' + (i ? 'destino' : 'origem')}>{lookup.places.map((place, index) => <li key={index}><button onClick={() => choose(slot, place)}>{place.label}<ArrowRight size={16}/></button></li>)}</ul>}</div>}
        </div>)}</div>
        <div className="journey-options"><label>Como ir<select aria-label="Como percorrer o trajeto" value={mode} onChange={e => { invalidate(); setMode(e.target.value as TravelMode); }}><option value="foot">A pé</option><option value="bike">Bicicleta</option><option value="car">Carro</option></select></label><label>Proximidade<select aria-label="Distância dos sinistros até o percurso" value={radius} onChange={e => setRadius(Number(e.target.value))}><option value={25}>Até 25 m</option><option value={50}>Até 50 m</option><option value={100}>Até 100 m</option></select></label><button className="primary-button" disabled={!map || !from || !to || routeBusy} onClick={() => void trace()}>{routeBusy ? 'Traçando percurso…' : 'Traçar percurso'}<ArrowRight size={18}/></button>{(from || to) && <button className="journey-text-button" onClick={clear}>Limpar percurso</button>}</div>
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
        {route && <div className="journey-result" aria-live="polite"><div><strong className={countBusy ? 'journey-counter pending-counter' : 'journey-counter'}>{ready ? `${number(selection.total)} ${selection.total === 1 ? 'sinistro' : 'sinistros'}` : countError ? 'Contagem indisponível' : 'Contando sinistros…'}</strong><p>Até {radius} m do percurso · {period}</p></div><div className="journey-metrics"><span>{distanceLabel(route.distance_m)}</span><span>{durationLabel(route.duration_s)} estimados</span></div><div className="journey-result-actions"><button className="journey-text-button" onClick={() => fit(route)}><LocateFixed size={16}/>Ver percurso</button>{ready && selection.total > 0 && <button className="journey-text-button" onClick={() => setListOpen(v => !v)} aria-expanded={listOpen}>{listOpen ? 'Ocultar registros' : 'Ver registros'}<ArrowRight size={16}/></button>}</div></div>}
        {countError && <div className="journey-error" role="alert"><p>{countError}</p><button className="journey-text-button" onClick={() => setRetry(n => n + 1)}>Tentar contagem novamente</button></div>}
        {bandError && <p className="journey-note" role="status">Não foi possível desenhar a faixa sombreada. A contagem continua usando a distância de proximidade selecionada.</p>}
        {ready && selection.total === 0 && <p className="journey-note">Nenhum sinistro com localização disponível foi encontrado nessa faixa e com os filtros aplicados. Isso não significa ausência de risco.</p>}
        {ready && selection.total > (selection.map_shown || 0) && <p className="journey-note">O mapa mostra os primeiros {number(selection.map_shown || 0)} de {number(selection.total)} registros. A contagem inclui todos e a lista permite consultar os demais.</p>}
        {route && route.waypoints.some(w => w.distance_m > 5) && <p className="journey-note">A rota foi ajustada à rede de ruas. {route.waypoints.map((w, i) => w.distance_m > 5 ? `${i ? 'Destino B' : 'Origem A'} a ${distanceLabel(w.distance_m)} do ponto escolhido.` : '').filter(Boolean).join(' ')} As linhas pontilhadas não confirmam um caminho de acesso e não entram na distância, no tempo ou na contagem de sinistros.</p>}
        {listOpen && selection && <div className="journey-records" aria-busy={countBusy}><div className="journey-records-heading"><h3>Registros próximos ao percurso</h3><span>{ready ? `${number(selection.offset + 1)} a ${number(selection.offset + selection.items.length)} de ${number(selection.total)}` : 'Carregando registros…'}</span></div>{ready && <ul>{selection.items.map(row => <li key={row.id}><button onClick={() => onDetail(row.id)}><span><time>{String(row.date).slice(6, 8)}/{String(row.date).slice(4, 6)}/{String(row.date).slice(0, 4)}</time><span>{meta?.types?.[row.type] || 'Sinistro'} · #{row.id}</span></span><ArrowRight size={18}/></button></li>)}</ul>}<div className="journey-pagination"><button disabled={!ready || selection.offset === 0} onClick={() => route && void count(route, Math.max(0, selection.offset - selection.limit), false)}>Anteriores</button><button disabled={!ready || !selection.has_more} onClick={() => route && void count(route, selection.offset + selection.limit, false)}>Próximos</button></div></div>}
        <p className="journey-note">Os filtros aplicados também valem para o percurso. A faixa sombreada indica a proximidade consultada; os pontos mantêm as coordenadas da fonte. A contagem não mede a segurança do caminho.</p>
        <div className="journey-attribution"><span>Rotas: <a href="https://routing.openstreetmap.de/about.html" target="_blank" rel="noreferrer">OSRM / FOSSGIS</a> · Busca: <a href="https://photon.komoot.io" target="_blank" rel="noreferrer">Photon</a> · Dados: <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noreferrer">© OpenStreetMap</a></span><a href="https://www.openstreetmap.org/fixthemap" target="_blank" rel="noreferrer">Corrigir o mapa</a><details><summary>Sobre a busca e as rotas</summary><p>Os serviços externos recebem os lugares pesquisados e os pontos do percurso. O operador de rotas registra as consultas. Este teste usa serviços públicos com limites de uso e depende da disponibilidade deles.</p></details></div>
    </section>;
}
