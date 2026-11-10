'use client';
import { apiURL } from '../lib/api-url';
import { useEffect, useRef, useState, useCallback } from 'react';
import { ArrowUpRight, ArrowRight, ArrowDownToLine, Search, SlidersHorizontal, X, ChevronDown, RotateCcw, LocateFixed, Plus, Minus, Info, Check, Route, FlaskConical, AlertCircle } from './icons';
import EventDetail from './EventDetail';
import JourneyPanel from './JourneyPanel';
import { CountUp } from './motion';
import DetailDialog from './DetailDialog';
import BenchmarkPanel from './BenchmarkPanel';
import BenchmarkResults from './BenchmarkResults';
import SampleBenchmarkPanel from './SampleBenchmarkPanel';
import DataStructureGuide, { type StructureGuide } from './DataStructureGuide';
import structureGuide from './data-structures-guide.json';
import StatisticsPanel from './StatisticsPanel';
import { resolvePointSelection, loadPointSelection } from './point-selection';
import { fitFrame, MIN_MAP_ZOOM, STATE_BOUNDS, HOME_BOUNDS, municipalityBounds, frameOptions, type MapBounds } from './map-view';
import { query, filterKey, findCity, isoDate, validateDates, makeExport, statisticsAreCurrent, type Filters, type Viewport } from './query-state';
import {BASEMAP_STYLE,improveMapContext,RoadDensityController,applyMapMode,type MapMode} from './map-context';
import { displayResolution, groupBounds, layoutGroups } from './map-layout';
import type { Map as MapLibreMap, GeoJSONSource } from 'maplibre-gl';
const fmt = (n: number | undefined) => n === undefined ? '—' : new Intl.NumberFormat('pt-BR').format(n);


const initial = { ano: '2026', municipio: '', tipo: '', transporte: '', via: '', registro: '', inicio: '', fim: '' };
function labelCase(s: string) { return s.toLocaleLowerCase('pt-BR').replace(/(^|\s)\p{L}/gu, x => x.toUpperCase()); }
async function json(url: string, signal?: AbortSignal) {
    const r = await fetch(apiURL(url), { signal });
    const text = await r.text();
    const o: any = JSON.parse(text);
    if (o && typeof o === 'object')
        Object.defineProperty(o, 'response_bytes', { value: new TextEncoder().encode(text).length, enumerable: false });
    if (!r.ok)
        throw Error(o.error || 'Não foi possível obter os dados.');
    return o;
}
export default function Atlas() {
    const [meta, setMeta] = useState<any>(null), [stats, setStats] = useState<any>(null), [draft, setDraft] = useState(initial), [active, setActive] = useState(initial), [cityText, setCityText] = useState(''), [tab, setTab] = useState('map'), [error, setError] = useState(''), [loading, setLoading] = useState(true), [mapLoading, setMapLoading] = useState(true), [mobile, setMobile] = useState(false), [mapMode,setMapMode]=useState<MapMode>('groups'),[showOriginal,setShowOriginal]=useState(false),[roadReach,setRoadReach]=useState(300),[roadState,setRoadState]=useState<any>({reason:'loading'}), [mapInfo, setMapInfo] = useState<any>({}), [detail, setDetail] = useState<any>(null), [detailLoading, setDetailLoading] = useState(false), [searchId, setSearchId] = useState(''), [bench, setBench] = useState<any>(null), [native, setNative] = useState<any>(null), [benchmarking, setBenchmarking] = useState(false), [mapReady, setMapReady] = useState(false), [notice, setNotice] = useState(''), [group, setGroup] = useState<any>(null), [groupLoading, setGroupLoading] = useState(false), [mapSelection, setMapSelection] = useState<any>(null), [cameraZoom, setCameraZoom] = useState(5), [statsKey, setStatsKey] = useState(''), [dialogError, setDialogError] = useState('');
    const mapRef = useRef<MapLibreMap | null>(null), mapElement = useRef<HTMLDivElement>(null), mapAbort = useRef<AbortController | null>(null), activeRef = useRef(active), lastGeo = useRef<any>(null), statsAbort = useRef<AbortController | null>(null), detailAbort = useRef<AbortController | null>(null), groupAbort = useRef<AbortController | null>(null), displayedFilters = useRef<Filters>(active), zoomTarget = useRef<number | null>(null), viewportRef = useRef<Viewport | null>(null), modeRef=useRef<MapMode>(mapMode),showOriginalRef=useRef(showOriginal),roadOverlay=useRef<RoadDensityController|null>(null), benchmarkAbort = useRef<AbortController | null>(null), sidebarRef = useRef<HTMLElement>(null);
    const [statsError,setStatsError]=useState(''),[statsRetry,setStatsRetry]=useState(0),[filterError,setFilterError]=useState('');
    const [journeyOpen,setJourneyOpen]=useState(false);
    const journeyOpenRef=useRef(false),journeyPickingRef=useRef(false);
    const onJourneyPicking=useCallback((value:boolean)=>{journeyPickingRef.current=value;},[]);
    const overviewRef=useRef(true),framedBoundsRef=useRef<MapBounds>(HOME_BOUNDS),lastFramedCity=useRef('');
    const density=mapMode==='density'||mapMode==='roads';
    const pointsLimited=mapMode==='points'&&!mapLoading&&mapInfo.mode==='clusters';
    const dateStart = isoDate(meta?.date_start || 20150101), dateEnd = isoDate(meta?.date_end || 20260930);
    const statisticsReady = statisticsAreCurrent(stats,statsKey,active,loading);
    const currentKey = filterKey(active), snapshotReady = statisticsReady && mapInfo.filterKey === currentKey && !loading && !mapLoading;
    const closeDetails = useCallback(() => { detailAbort.current?.abort(); groupAbort.current?.abort(); setDetail(null); setGroup(null); setDetailLoading(false); setGroupLoading(false); setDialogError(''); }, []);
    useEffect(() => {
        json('/api/v1/metadata').then(m => {
            setMeta(m);
            if (!m.years.some((y: any) => String(y.year) === initial.ano)) {
                const f = { ...initial, ano: String(m.years.at(-1)?.year || '') };
                setDraft(f);
                setActive(f);
            }
        }).catch(e => setError(e.message));
    }, []);
    useEffect(() => {
        activeRef.current = active;
        benchmarkAbort.current?.abort();
        setBenchmarking(false);
        setBench(null);
        statsAbort.current?.abort();
        const a = new AbortController();
        statsAbort.current = a;
        setLoading(true);
        setStatsError('');
        setError('');
        json('/api/v1/statistics?' + query(active), a.signal).then(r => {
            if (!a.signal.aborted) {
                setStats(r);
                setStatsKey(filterKey(active));
            }
        }).catch(e => {
            if (e.name !== 'AbortError') {
                setStatsError(e.message);
                setError(e.message);
            }
        }).finally(() => {
            if (!a.signal.aborted)
                setLoading(false);
        });
        return () => a.abort();
    }, [active,statsRetry]);
    const showDetail = useCallback(async (id: number) => {
        detailAbort.current?.abort();
        const a = new AbortController();
        detailAbort.current = a;
        setDetail(null);
        setDialogError('');
        setDetailLoading(true);
        try {
            const r = await json('/api/v1/sinistros/' + id, a.signal);
            if (!a.signal.aborted)
                setDetail(r);
        }
        catch (e: any) {
            if (e.name !== 'AbortError')
                setDialogError(e.message);
        }
        finally {
            if (!a.signal.aborted)
                setDetailLoading(false);
        }
    }, []);
    const showGroup = useCallback(async (props: any, filters: Filters, offset = 0) => {
        groupAbort.current?.abort();
        detailAbort.current?.abort();
        setDetail(null);
        setDetailLoading(false);
        const a = new AbortController();
        groupAbort.current = a;
        setGroup({ items: [], total: Number(props.count) || 0, offset, limit: 30, context: props, filters });
        setGroupLoading(true);
        setDialogError('');
        const p = query(filters);
        p.set('bbox', props.bbox);
        if (props.grid) {
            p.set('grade', String(props.grid));
            p.set('celula', String(props.cell));
        }
        p.set('offset', String(offset));
        p.set('limit', '30');
        try {
            let context=props,page;
            if(props.ids){
                const items=props.selectedItems||await loadPointSelection(props.ids,async(start,limit)=>{
                    const selectedQuery=new URLSearchParams(p);selectedQuery.set('offset',String(start));selectedQuery.set('limit',String(limit));
                    return json('/api/v1/events?'+selectedQuery,a.signal);
                });
                context={...props,selectedItems:items};
                page={items:items.slice(offset,offset+30),total:items.length,offset,limit:30,has_more:offset+30<items.length};
            }else page=await json('/api/v1/events?'+p,a.signal);
            if (!a.signal.aborted)
                setGroup({ ...page, context, filters });
        }
        catch (e: any) {
            if (e.name !== 'AbortError')
                setDialogError(e.message);
        }
        finally {
            if (!a.signal.aborted)
                setGroupLoading(false);
        }
    }, []);
    const locateEvent = (lon: number, lat: number) => {
        overviewRef.current=false;
        setMapSelection(null);
        setTab('map');
        setDetail(null);
        setGroup(null);
        setTimeout(() => {
            const map = mapRef.current;
            if (!map)
                return;
            map.resize();
            map.flyTo({ center: [lon, lat], zoom: 17, speed: 1.2 });
            const data: any = { type: 'FeatureCollection', features: [{ type: 'Feature', geometry: { type: 'Point', coordinates: [lon, lat] }, properties: {} }] };
            if (map.getSource('selected-event'))
                (map.getSource('selected-event') as GeoJSONSource).setData(data);
            else {
                map.addSource('selected-event', { type: 'geojson', data });
                map.addLayer({ id: 'selected-event', type: 'circle', source: 'selected-event', paint: { 'circle-radius': 12, 'circle-color': '#ffffff', 'circle-opacity': 0.2, 'circle-stroke-width': 3, 'circle-stroke-color': '#1b6d63' } });
            }
        }, 100);
        setNotice('Localização oficial do registro destacada no mapa.');
    };
    const updateMap = useCallback(async () => {
        const map = mapRef.current;
        if (!map || !map.getSource('events'))
            return;
        const container = map.getContainer();
        if (container.clientWidth > 0 && container.clientHeight > 0) {
            const b = map.getBounds();
            viewportRef.current = { bbox: [Math.max(-180, b.getWest()), Math.max(-90, b.getSouth()), Math.min(180, b.getEast()), Math.min(90, b.getNorth())].join(','), zoom: Math.floor(map.getZoom()), resolution: displayResolution(modeRef.current,container.clientWidth,container.clientHeight) };
        }
        const viewport = viewportRef.current;
        if (!viewport)
            return;
        mapAbort.current?.abort();
        benchmarkAbort.current?.abort();
        setBenchmarking(false);
        setBench(null);
        const controller = new AbortController();
        mapAbort.current = controller;
        const filters = { ...activeRef.current }, p = query(filters);
        p.set('bbox', viewport.bbox);
        p.set('zoom', String(viewport.zoom));
        p.set('resolucao', String(viewport.resolution));
        if(modeRef.current==='roads'||modeRef.current==='points')p.set('pontos','1');
        roadOverlay.current?.invalidate();
        setMapLoading(true);
        const t = performance.now();
        try {
            const g = await json('/api/v1/map?' + p, controller.signal);
            if (controller.signal.aborted)
                return;
            lastGeo.current = g;
            displayedFilters.current = filters;
            (map.getSource('events') as GeoJSONSource).setData(layoutGroups(g, p => map.project(p), p => map.unproject([p.x, p.y])));
            (map.getSource('density-events') as GeoJSONSource).setData(g);
            map.setPaintProperty('density', 'heatmap-weight', ['/', ['get', 'count'], Math.max(1, ...g.features.map((f: any) => f.properties.count))]);
            applyMapMode(map,modeRef.current,showOriginalRef.current,g.mode==='points',journeyOpenRef.current);
            roadOverlay.current?.setData(g,viewport.bbox);
            setMapInfo({ count: g.count, features: g.features.length, ms: performance.now() - t, bytes: g.response_bytes, examined: g.examined, mode: g.mode, ...viewport, filterKey: filterKey(filters) });
        }
        catch (e: any) {
            if (e.name !== 'AbortError')
                setError(e.message);
        }
        finally {
            if (!controller.signal.aborted)
                setMapLoading(false);
        }
    }, []);
    const zoomBy = (delta: number) => {
        const map = mapRef.current;
        if (!map)
            return;
        const target = Math.max(map.getMinZoom(), Math.min(map.getMaxZoom(), (zoomTarget.current ?? map.getZoom()) + delta));
        overviewRef.current=false;
        map.stop();
        zoomTarget.current = target;
        map.easeTo({ zoom: target, duration: 260 });
    };
    const resetMap = () => { setMapSelection(null);zoomTarget.current=null;overviewRef.current=true;framedBoundsRef.current=municipalityBounds(activeRef.current.municipio)||STATE_BOUNDS;if(mapRef.current)fitFrame(mapRef.current,framedBoundsRef.current,500); };
    useEffect(() => {
        if (!mapElement.current || mapRef.current)
            return;
        // A refreshed map must load before child overlays bind to its sources.
        setMapReady(false);
        let disposed = false;
        let timer: ReturnType<typeof setTimeout>;
        import('maplibre-gl').then(({ default: ml }) => {
            if (disposed || !mapElement.current)
                return;
            const map = new ml.Map({ container: mapElement.current, style: BASEMAP_STYLE, bounds: HOME_BOUNDS, fitBoundsOptions: frameOptions(), minZoom: MIN_MAP_ZOOM, maxZoom: 18, dragRotate: false, touchPitch: false, attributionControl: { compact: true }, maxBounds: [[-59, -31], [-39, -14]] });
            mapRef.current = map;
            map.touchZoomRotate.disableRotation();
            map.keyboard.disableRotation();
            map.on('load', () => {
                const empty: any = { type: 'FeatureCollection', features: [] };
                map.addSource('events', { type: 'geojson', data: empty });
                map.addSource('density-events', { type: 'geojson', data: empty });
                map.addLayer({ id: 'density', type: 'heatmap', source: 'density-events', layout: { visibility: 'none' }, paint: { 'heatmap-weight': ['interpolate', ['linear'], ['get', 'count'], 1, 0.05, 100, 1], 'heatmap-radius': ['interpolate',['linear'],['zoom'],5,26,11,22,15,16,18,12], 'heatmap-opacity': 0.82, 'heatmap-intensity': 1.3, 'heatmap-color': ['interpolate', ['linear'], ['heatmap-density'], 0, 'rgba(251,172,90,0)', 0.25, '#ffd0a3', 0.5, '#f5a25d', 0.75, '#e57241', 1, '#a93424'] } });
                map.addLayer({ id: 'events-glow', type: 'circle', source: 'events', paint: { 'circle-radius': ['+', ['get', 'radius'], 2], 'circle-color': '#e89165', 'circle-opacity': 0.10 } });
                map.addLayer({ id: 'events', type: 'circle', source: 'events', paint: { 'circle-radius': ['get', 'radius'], 'circle-color': ['step', ['get', 'count'], '#e3a275', 30, '#d38051', 150, '#bc6239', 500, '#a74e2d'], 'circle-opacity': 0.80, 'circle-stroke-color': '#fff', 'circle-stroke-width': 1.3 } });
                map.addLayer({ id: 'counts', type: 'symbol', source: 'events', filter: ['>', ['get', 'count'], 1], layout: { 'text-field': ['get', 'label'], 'text-size': 11, 'text-font': ['Montserrat SemiBold', 'Open Sans Bold', 'Noto Sans Regular'], 'text-allow-overlap': false }, paint: { 'text-color': '#ffffff' } });
                map.addLayer({ id: 'event-hit', type: 'circle', source: 'events', paint: { 'circle-radius': ['get', 'hit_radius'], 'circle-opacity': 0 } });
                roadOverlay.current=new RoadDensityController(map,setRoadState);
                for(const id of ['events','counts','event-hit'])map.moveLayer(id);
                improveMapContext(map);
                map.addControl(new ml.ScaleControl({maxWidth:110,unit:'metric'}),'bottom-left');
                map.on('click', 'event-hit', ev => {
                    if(journeyOpenRef.current||journeyPickingRef.current)return;
                    overviewRef.current=false;
                    const features = ev.features || [], feature = features[0];
                    if (!feature)
                        return;
                    const props = feature.properties!, filters = { ...displayedFilters.current };
                    if (Number(props.count) > 1) {
                        // Exploration keeps the map available. Listing is an explicit, separate action.
                        setMapSelection({ props, filters });
                        const b = groupBounds(props);
                        if (b && map.getZoom() < map.getMaxZoom() - .1) {
                            const camera = map.cameraForBounds([[b[0], b[1]], [b[2], b[3]]], { padding: 48, maxZoom: map.getMaxZoom() });
                            const zoom = Math.min(map.getMaxZoom(), Math.max(map.getZoom() + 1, Math.min(map.getZoom() + 2.5, camera?.zoom ?? map.getZoom() + 2)));
                            map.easeTo({ center: camera?.center || (feature.geometry as any).coordinates, zoom, duration: 420 });
                        }
                        else
                            showGroup(props, filters);
                    }
                    else {
                        const selected=resolvePointSelection(features,lastGeo.current);
                        if(!selected){setNotice('O mapa está atualizando. Aguarde e selecione novamente.');return;}
                        if(selected.ids.length===1){setGroup(null);showDetail(selected.ids[0]);}
                        else showGroup(selected,filters);
                    }
                });
                map.on('mouseenter', 'event-hit', () => map.getCanvas().style.cursor = 'pointer');
                map.on('mouseleave', 'event-hit', () => map.getCanvas().style.cursor = '');
                map.on('movestart', event => {
                    clearTimeout(timer);
                    mapAbort.current?.abort();
                    roadOverlay.current?.invalidate();
                    setMapLoading(true);
                    if (event.originalEvent){overviewRef.current=false;zoomTarget.current=null;}
                });
                map.on('moveend', () => { zoomTarget.current = null; setCameraZoom(map.getZoom()); clearTimeout(timer); timer = setTimeout(updateMap, 150); });
                map.on('resize', () => { if(overviewRef.current)fitFrame(map,framedBoundsRef.current);clearTimeout(timer);timer=setTimeout(updateMap,150); });
                setCameraZoom(map.getZoom());
                setMapReady(true);
                updateMap();
            });
            map.on('error', ev => {
                if ((ev.error?.message || '').includes('WebGL'))
                    setError('Não foi possível iniciar o mapa: WebGL indisponível. As estatísticas continuam acessíveis.');
            });
        }).catch(e => setError(e.message));
        return () => { disposed = true; clearTimeout(timer); mapAbort.current?.abort(); roadOverlay.current?.destroy(); roadOverlay.current=null; mapRef.current?.remove(); mapRef.current = null; };
    }, [updateMap, showDetail, showGroup]);
    useEffect(()=>{
        if(!mapReady||!mapRef.current||lastFramedCity.current===active.municipio)return;
        lastFramedCity.current=active.municipio;
        const box=active.municipio?municipalityBounds(active.municipio):STATE_BOUNDS;
        if(!box)return;
        overviewRef.current=true;framedBoundsRef.current=box;zoomTarget.current=null;
        mapRef.current.stop();fitFrame(mapRef.current,box,500);
    },[active.municipio,mapReady]);
    useEffect(()=>{modeRef.current=mapMode;if(mapReady){roadOverlay.current?.setMode(mapMode==='roads'&&!journeyOpen);updateMap();}},[active,mapReady,mapMode,journeyOpen,updateMap]);
    useEffect(()=>{showOriginalRef.current=showOriginal;if(mapReady&&mapRef.current)applyMapMode(mapRef.current,mapMode,showOriginal,mapInfo.mode==='points',journeyOpen);},[showOriginal,mapMode,mapReady,mapInfo.mode,journeyOpen]);
    useEffect(()=>{roadOverlay.current?.setReach(roadReach);},[roadReach]);
    const toggleJourney=()=>{
        const open=!journeyOpen;journeyOpenRef.current=open;setJourneyOpen(open);journeyPickingRef.current=false;
        overviewRef.current=false;setMapSelection(null);closeDetails();
    };
    const selectMode=(mode:MapMode)=>{
        setMapMode(mode);setMapSelection(null);
        if(mode==='roads'&&active.via!=='1'){
            const filters={...active,via:'1'};setActive(filters);setDraft(d=>({...d,via:'1'}));setNotice('Filtro de estradas e rodovias aplicado.');
        }
    };
    useEffect(() => {
        if (tab === 'map')
            setTimeout(() => mapRef.current?.resize(), 80);
        if (tab === 'lab' && !native)
            json('/api/v1/native-benchmarks').then(setNative).catch(e => setError(e.message));
    }, [tab, native]);
    useEffect(() => {
        if (!notice)
            return;
        const t = setTimeout(() => setNotice(''), 5000);
        return () => clearTimeout(t);
    }, [notice]);
    const change = (k: keyof Filters, v: string) => {setFilterError('');setDraft(d => ({...d,[k]:v}));};
    const apply = () => {
        const dateError = validateDates(draft, dateStart, dateEnd);
        if (dateError) {
            setFilterError(dateError);
            return;
        }
        const city = cityText.trim() ? findCity(meta?.cities || [], cityText) : undefined;
        if (cityText.trim() && !city) {
            setFilterError('Selecione um município válido na lista.');
            return;
        }
        setMapSelection(null);
        closeDetails();
        if(mapMode==='roads'&&draft.via!=='1')setMapMode('groups');
        setFilterError('');
        setActive({ ...draft, municipio: city ? String(city.code) : '' });
        setMobile(false);
        setBench(null);
    };
    const reset = () => { setMapMode('groups'); const f = { ...initial, ano: String(meta?.years.at(-1)?.year || initial.ano) }; setMapSelection(null); closeDetails(); setDraft(f); setActive(f); setCityText(''); setError(''); setFilterError(''); setBench(null); };
    const runBenchmark = async () => {
        if (!snapshotReady)
            return;
        benchmarkAbort.current?.abort();
        const controller = new AbortController();
        benchmarkAbort.current = controller;
        setBenchmarking(true);
        setError('');
        try {
            const p = query(active);
            p.set('bbox', mapInfo.bbox);
            const t = performance.now();
            const r = await json('/api/v1/benchmarks?' + p, controller.signal);
            if (!controller.signal.aborted)
                setBench({ ...r, roundtrip_ms: performance.now() - t, http_body_bytes: r.response_bytes, map_count: mapInfo.count, bbox: mapInfo.bbox, filters: { ...active } });
        }
        catch (e: any) {
            if (e.name !== 'AbortError')
                setError(e.message);
        }
        finally {
            if (!controller.signal.aborted)
                setBenchmarking(false);
        }
    };
    const download = () => {
        try {
            const output = makeExport({ meta, filters: active, statistics: stats, statisticsKey: statsKey, map: lastGeo.current, mapKey: mapInfo.filterKey, viewport: { bbox: mapInfo.bbox, zoom: mapInfo.zoom, resolution: mapInfo.resolution }, busy: !snapshotReady });
            const visualization = { mode: mapMode, response_mode: lastGeo.current?.mode, display_status: mapMode==='roads'?roadState.reason:pointsLimited?'zoom_required':'ready', ...(mapMode === 'roads' ? {
                diffusion_m: roadReach, max_association_m: 80, status: roadState.reason,
                ...(roadState.reason === 'ready' ? { matched: roadState.matched, unmatched: roadState.unmatched } : {})
            } : {}) };
            const blob = new Blob([JSON.stringify({ ...output, visualization }, null, 2)], { type: 'application/json' }), url = URL.createObjectURL(blob), a = document.createElement('a');
            a.href = url;
            a.download = 'atlas-viario-resumo.json';
            document.body.appendChild(a);
            a.click();
            a.remove();
            setTimeout(() => URL.revokeObjectURL(url), 1000);
            setNotice('Resumo exportado: filtros, estatísticas e mapa da área exibida.');
        }
        catch (e: any) {
            setError(e.message);
        }
    };
    useEffect(() => {
        if (!mobile || !sidebarRef.current)
            return;
        const panel = sidebarRef.current, previous = document.activeElement as HTMLElement, overflow = document.body.style.overflow;
        const background = Array.from(document.querySelectorAll<HTMLElement>('.topbar,main'));
        background.forEach(el => el.inert = true);
        document.body.style.overflow = 'hidden';
        const focusable = () => Array.from(panel.querySelectorAll<HTMLElement>('button:not(:disabled),input,select,a[href]')).filter(el => el.getClientRects().length);
        focusable()[0]?.focus();
        const handle = (e: KeyboardEvent) => {
            if (e.key === 'Escape') {
                e.preventDefault();
                setMobile(false);
            }
            if (e.key === 'Tab') {
                const all = focusable(), first = all[0], last = all.at(-1);
                if (e.shiftKey && document.activeElement === first) {
                    e.preventDefault();
                    last?.focus();
                }
                else if (!e.shiftKey && document.activeElement === last) {
                    e.preventDefault();
                    first?.focus();
                }
            }
        };
        document.addEventListener('keydown', handle);
        return () => {
            document.removeEventListener('keydown', handle);
            background.forEach(el => el.inert = false);
            document.body.style.overflow = overflow;
            if (previous?.isConnected)
                previous.focus({ preventScroll: true });
        };
    }, [mobile]);
    const selectedCity = meta?.cities.find((c: any) => String(c.code) === active.municipio);
    const rangeStart = active.inicio || (active.ano ? `${active.ano}-01-01` : dateStart), rangeEnd = active.fim || (active.ano ? `${active.ano}-12-31` : dateEnd);
    const periodStart = rangeStart < dateStart ? dateStart : rangeStart, periodEnd = rangeEnd > dateEnd ? dateEnd : rangeEnd;
    const period = `${periodStart.split('-').reverse().join('/')} a ${periodEnd.split('-').reverse().join('/')}`;
    const partial = !!active.ano && dateEnd.startsWith(active.ano) && !dateEnd.endsWith('12-31');
    
    return <div className="atlas-app">
 <a className="skip-link" href="#conteudo">Ir para o conteúdo</a>
 <header className="topbar"><a href={import.meta.env.BASE_URL} className="brand" aria-label="Atlas Viário início"><span className="brand-symbol"><Route size={24}/></span><span>atlas<span className="brand-light">viário</span></span></a><nav aria-label="Navegação principal">{[['map','Mapa'],['lab','Desempenho'],['about','Sobre os dados']].map(([key,label])=><button key={key} className={tab===key?'selected':''} aria-current={tab===key?'page':undefined} onClick={()=>setTab(key)}>{label}</button>)}</nav></header>
 <div className="workspace">
 {mobile && <button className="sidebar-scrim" onClick={() => setMobile(false)} aria-label="Fechar filtros"/>}
 <aside id="filtros" ref={sidebarRef} role={mobile ? "dialog" : undefined} aria-modal={mobile ? true : undefined} aria-label="Filtros da consulta" className={'sidebar ' + (mobile ? 'open' : '')}><div className="filter-heading"><span><SlidersHorizontal size={17}/>Filtros</span><button className="mobile-close-filters" onClick={() => setMobile(false)} aria-label="Fechar painel de filtros"><X size={18}/></button><button onClick={reset} title="Restaurar filtros" aria-label="Restaurar filtros"><RotateCcw size={15}/></button></div><div className="filter-fields">
 <label><span>Município</span><div className="input-icon"><Search size={15}/><input aria-label="Buscar município" placeholder="Todo o estado de São Paulo" list="cities" value={cityText} onChange={e => { setCityText(e.target.value); const c = findCity(meta?.cities || [], e.target.value); change('municipio', c ? String(c.code) : ''); }}/><datalist id="cities">{meta?.cities.map((c: any) => <option key={c.code} value={labelCase(c.name)}/>)}</datalist>{cityText && <button onClick={() => { setCityText(''); change('municipio', ''); }} aria-label="Limpar município"><X size={12}/></button>}</div></label>
 <label><span>Ano</span><div className="select-wrap"><select aria-label="Ano de referência" value={draft.ano} onChange={e => { change('ano', e.target.value); change('inicio', ''); change('fim', ''); }}><option value="">Todos os anos</option>{(meta?.years || [{ year: 2026 }]).slice().reverse().map((y: any) => <option key={y.year} value={y.year}>{y.year}{String(y.year) === dateEnd.slice(0, 4) && !dateEnd.endsWith('12-31') ? ' · parcial' : ''}</option>)}</select><ChevronDown size={14}/></div></label>
 <div className="date-fields"><label><span>De</span><input type="date" aria-label="Data inicial" min={dateStart} max={dateEnd} value={draft.inicio} onChange={e => { change('inicio', e.target.value); change('ano', ''); }}/></label><label><span>Até</span><input type="date" aria-label="Data final" min={dateStart} max={dateEnd} value={draft.fim} onChange={e => { change('fim', e.target.value); change('ano', ''); }}/></label></div>
 <div className="filter-divider"/>
 {[['tipo', 'Tipo de sinistro', 'Todos os tipos', meta?.types], ['transporte', 'Transporte', 'Todos os modos', meta?.transports], ['via', 'Tipo de via', 'Todas as vias', meta?.roads], ['registro', 'Classificação', 'Todos os registros', meta?.registrations]].map(([key, title, placeholder, options]: any) => <label key={key}><span>{title}</span><div className="select-wrap"><select aria-label={title} value={draft[key as keyof Filters]} onChange={e => change(key, e.target.value)}><option value="">{placeholder}</option>{options?.map((name: string, i: number) => <option key={i} value={i}>{name}</option>)}</select><ChevronDown size={14}/></div></label>)}
 </div><div className="filter-actions">{filterError&&<p role="alert" className="filter-error">{filterError}</p>}<button className="apply-button" onClick={apply}>Aplicar filtros <ArrowRight size={20}/></button><button className="clear-button" onClick={reset}>Limpar seleção</button></div></aside>
 <main id="conteudo" tabIndex={-1}><div className="page-heading"><h1 key={tab}>{tab==='map'?'Mapa de sinistros':tab==='lab'?'Comparação de algoritmos':'Dados e metodologia'}</h1>{tab!=='about'&&<div className="page-actions"><button className="filter-toggle" onClick={()=>{setFilterError('');setMobile(true);}} aria-label="Abrir filtros" aria-controls="filtros" aria-expanded={mobile}><SlidersHorizontal size={20}/>Filtros</button><button hidden={journeyOpen&&tab==='map'} className="export-button" onClick={download} disabled={!snapshotReady} aria-label="Exportar resumo da consulta em JSON" title={snapshotReady?'Baixar filtros, estatísticas e dados do mapa em JSON':'Aguarde a atualização dos dados'}><ArrowDownToLine size={20}/>Exportar</button></div>}</div>
 {error && <div role="alert" className="error"><AlertCircle size={16}/>{error}<button onClick={() => setError('')} aria-label="Fechar erro"><X size={15}/></button></div>}
 {tab!=='about'&&<div className="context-line"><span className="region-name">{selectedCity?labelCase(selectedCity.name):'Estado de São Paulo'}</span><span className="period">{period}</span>{partial&&<span className="partial">Ano incompleto</span>}{[['tipo',meta?.types],['transporte',meta?.transports],['via',meta?.roads],['registro',meta?.registrations]].map(([key,labels]:any)=>active[key as keyof Filters]!==''&&<span key={key}>{labels?.[Number(active[key as keyof Filters])]}</span>)}</div>}
 {tab!=='about' && periodStart < '2019-01-01' && <p className="coverage-warning">2015 a 2018 incluem apenas sinistros fatais. A cobertura ampliada a partir de 2019 limita comparações diretas entre esses períodos.</p>}

 <section hidden={tab!=='map'} className="map-section"><div className="map-toolbar"><div className="map-title"><b>{journeyOpen?'Sinistros próximos ao percurso':mapLoading?'Atualizando mapa':<><CountUp value={mapInfo.count} duration={500}/>{mapInfo.count===1?' sinistro na área':' sinistros na área'}</>}</b></div><div className="map-actions"><button className="area-list-button journey-toggle" onClick={toggleJourney} aria-expanded={journeyOpen}><Route size={18}/>{journeyOpen?'Voltar ao mapa':'Traçar percurso'}</button><button hidden={journeyOpen} className="area-list-button" disabled={mapLoading || !mapInfo.count} onClick={() => showGroup({ bbox: mapInfo.bbox, count: mapInfo.count, area: true }, displayedFilters.current)}>Ver lista da área</button><div hidden={journeyOpen} className="map-switch" role="group" aria-label="Visualização do mapa">{([['groups','Agrupamentos'],['points','Pontos'],['density','Densidade'],['roads','Nas rodovias']] as [MapMode,string][]).map(([mode,label])=><button key={mode} className={mapMode===mode?'active':''} aria-pressed={mapMode===mode} onClick={()=>selectMode(mode)}>{label}</button>)}</div></div></div>{!journeyOpen&&mapMode==='roads'&&<div className="road-analysis-controls"><label>Alcance<select aria-label="Alcance da difusão nas rodovias" value={roadReach} onChange={e=>setRoadReach(Number(e.target.value))}><option value={100}>100 m</option><option value={300}>300 m</option><option value={600}>600 m</option></select></label><label className="road-point-toggle"><input type="checkbox" checked={showOriginal} onChange={e=>setShowOriginal(e.target.checked)}/>Pontos originais</label></div>}
 {!journeyOpen&&pointsLimited&&<div className="point-limit-status" role="status"><span>Mais de 12.000 registros nesta área. Aproxime para exibir cada ponto ou veja os agrupamentos.</span><div><button onClick={()=>zoomBy(1)}>Aproximar área <Plus size={16}/></button><button onClick={()=>selectMode('groups')}>Ver agrupamentos</button></div></div>}
 {!journeyOpen&&mapMode==='roads'&&<div className="road-status" role="status">{mapLoading||['loading','working'].includes(roadState.reason)?'Preparando a concentração nas vias…':['zoom_level','point_limit','network_too_large'].includes(roadState.reason)?<><span>{roadState.reason==='zoom_level'?'Aproxime a região para visualizar o traçado detalhado das rodovias.':roadState.reason==='point_limit'?'Esta área tem mais de 12.000 eventos. Aproxime para mostrar todos os pontos, sem amostragem.':'A malha viária desta área é muito extensa. Aproxime para calcular a difusão.'}</span><button onClick={()=>mapRef.current?.easeTo({zoom:Math.min(18,Math.max(10,mapRef.current.getZoom()+1)),duration:450})}>Aproximar área <Plus size={14}/></button></>:roadState.reason==='ready'?<span>{fmt(roadState.matched)} de {fmt(roadState.total)} registros próximos ao traçado.{roadState.unmatched>0?` ${fmt(roadState.unmatched)} sem correspondência. Consulte os pontos originais ou a lista.`:''}</span>:roadState.reason==='empty'?'Nenhum registro de rodovia nesta área.':<span>Não foi possível associar os registros ao traçado disponível nesta área. Consulte os pontos originais ou aproxime o mapa.</span>}</div>}
 {!journeyOpen&&mapSelection&&<div className="selected-group-strip" aria-live="polite"><span>{fmt(Number(mapSelection.props.count))} sinistros no grupo selecionado</span><button onClick={()=>showGroup(mapSelection.props,mapSelection.filters)}>Ver registros <ArrowRight size={16}/></button><button className="dismiss-group" aria-label="Limpar grupo selecionado" onClick={()=>setMapSelection(null)}><X size={16}/></button></div>}<div hidden={!journeyOpen}>{journeyOpen&&<JourneyPanel map={mapReady?mapRef.current:null} filters={active} period={period} meta={meta} onDetail={showDetail} onPickingChange={onJourneyPicking}/>}</div><div className="map-wrap"><div ref={mapElement} className="map-canvas"/><div className="map-controls"><button onClick={() => zoomBy(1)} disabled={cameraZoom >= 18} aria-label="Aproximar mapa"><Plus size={18}/></button><button onClick={() => zoomBy(-1)} disabled={cameraZoom <= MIN_MAP_ZOOM} aria-label="Afastar mapa"><Minus size={18}/></button><output className="map-zoom" aria-label="Nível de zoom">{cameraZoom.toFixed(1).replace('.', ',')}</output><div /><button onClick={resetMap} title={active.municipio?'Enquadrar município':'Ver todo o estado'} aria-label={active.municipio?'Enquadrar município':'Ver todo o estado'}><LocateFixed size={18}/></button></div>{mapLoading && <div className="map-busy"><span className="loading-bar"/>Consultando a região</div>}</div><div hidden={journeyOpen} className="map-accessories">{!pointsLimited&&<div className="map-legend"><span>{mapMode==='roads'?'Concentração ao longo da via':density ? 'Concentração relativa' : mapMode==='points'||mapInfo.mode === 'points' ? 'Sinistros individuais' : 'Sinistros por agrupamento'}</span>{density ? <div><small>Menor</small><span className="density-scale" aria-hidden="true"><i/><i/><i/><i/></span><small>Maior</small></div> : mapInfo.mode === 'points' ? <div><i className="legend-dot one"/><small>Cada ponto representa um registro</small></div> : <div><i className="legend-dot one"/><small>1</small><i className="legend-dot two"/><small>30</small><i className="legend-dot three"/><small>150</small><i className="legend-dot four"/><small>500+</small></div>}<p>{mapMode==='roads'?'Associação ao traçado em até 80 m. Os pontos preservam as coordenadas da fonte. A intensidade varia com a área exibida e não mede risco.':density ? 'Concentração suavizada da área. Para acompanhar o traçado das estradas, escolha Nas rodovias.' : mapInfo.mode === 'points' ? 'Coordenadas originais. Clique em um ponto para ver os detalhes.' : 'Clique em um grupo para aproximar.'}</p></div>}<form className="id-search" onSubmit={e => {
            e.preventDefault();
            if (/^\d+$/.test(searchId)) {
                groupAbort.current?.abort();
                setGroup(null);
                showDetail(Number(searchId));
            }
            else
                setError('Informe um identificador numérico.');
        }}><Search size={15}/><input aria-label="Buscar sinistro por ID" placeholder="Buscar sinistro por ID" value={searchId} onChange={e => setSearchId(e.target.value)}/><button aria-label="Buscar sinistro"><ArrowRight size={16}/></button></form></div><div className="map-footer"><span><Info size={16}/>{statisticsReady?`${fmt(stats.unlocated)} ${stats.unlocated===1?'sinistro dos filtros selecionados não aparece':'sinistros dos filtros selecionados não aparecem'} no mapa: as coordenadas estão ausentes, são inválidas ou estão fora do recorte adotado.`:statsError?'Estatísticas indisponíveis.':'Carregando estatísticas…'}</span></div></section>
 {tab==='map'&&<StatisticsPanel stats={stats} ready={statisticsReady} error={statsError} onRetry={()=>setStatsRetry(n=>n+1)} meta={meta} year={active.ano} dateEnd={dateEnd} partial={partial}/>}
 {tab === 'lab' && <div className="lab-content"><nav className="lab-navigation" aria-label="Seções de desempenho"><a href="#sample-title">Amostra de 1.000 sinistros</a><a href="#structure-guide-title">Guia de estruturas</a><a href="#current-benchmark-title">Consulta atual</a><a href="#saved-times-title">Outras medições</a></nav><SampleBenchmarkPanel/><DataStructureGuide guide={structureGuide as StructureGuide}/><section className="lab-intro"><div><h2 id="current-benchmark-title">Consulta atual</h2><p>Compare os métodos com os filtros e a última área selecionada no mapa. {snapshotReady ? `Área selecionada: ${fmt(mapInfo.count)} ${mapInfo.count===1?'sinistro':'sinistros'}.` : 'Aguarde a consulta da área terminar.'}</p></div><button className="primary-button" onClick={runBenchmark} disabled={benchmarking || !snapshotReady}><FlaskConical size={16}/>{benchmarking ? 'Comparando…' : 'Comparar métodos'}</button></section>{bench&&<BenchmarkResults result={bench}/>}<details className="request-details map-request-details"><summary>Detalhes da última consulta do mapa</summary>{snapshotReady?<><dl><div><dt>Tempo da resposta</dt><dd>{fmt(Math.round(mapInfo.ms))} ms</dd></div><div><dt>Tamanho do JSON</dt><dd>{(mapInfo.bytes/1024).toLocaleString('pt-BR',{minimumFractionDigits:1,maximumFractionDigits:1})} KiB</dd></div><div><dt>Sinistros na área</dt><dd>{fmt(mapInfo.count)}</dd></div></dl><p>O tempo inclui a comunicação com o servidor. As estatísticas consideram os filtros; o mapa considera também a área exibida.</p></>:<p>Aguarde a consulta do mapa terminar.</p>}</details>
 <BenchmarkPanel data={native} meta={meta}/></div>}
 {tab === 'about' && <div className="about-grid"><section className="about-main"><h2>Fonte e cobertura</h2><p>O Atlas Viário é um projeto acadêmico independente para estudar consultas geográficas, algoritmos e estruturas de dados. Os registros vêm do Infosiga / Detran-SP e foram preservados nos arquivos do projeto universitário.</p><div className="coverage-list">{meta?.coverage_notes.map((n: string, i: number) => <div key={n}><p>{n}</p></div>)}</div><a className="text-link" href="https://infosiga.detran.sp.gov.br/mapas/" target="_blank" rel="noreferrer">Consultar o InfoMapa oficial <ArrowUpRight size={16}/></a></section><section className="about-tech"><h2>Processamento dos dados</h2><p>O leitor e a lista sequencial do projeto original processam os dados em lotes. O motor usa registros compactos e índices para selecionar candidatos às consultas.</p><ol><li><b>Fonte preservada</b><span>A preparação lê a base em CSV; SHA-256 identifica sua versão.</span></li><li><b>Motor escrito em C</b><span>Servidor nativo com CivetWeb; no Sites, o mesmo motor executa em WebAssembly no servidor.</span></li><li><b>Mapa sob demanda</b><span>O navegador recebe somente agrupamentos ou pontos da região visível, com filtros aplicados.</span></li></ol><div className="version"><span>Versão da base</span><code>{meta?.version || '…'}</code></div><p className="small">As coordenadas usam WGS84 e mantêm a precisão da fonte. A seleção usa um retângulo amplo em torno de São Paulo; ela não verifica o limite administrativo do estado.</p><p className="geography-credit">O enquadramento dos municípios usa a <a href="https://servicodados.ibge.gov.br/api/docs/malhas?versao=3" target="_blank" rel="noreferrer">malha simplificada do IBGE</a>. Os registros seguem o município informado na fonte.</p></section></div>}
 <footer className="page-footer"><span>Fonte: Infosiga / Detran-SP</span><span>Projeto acadêmico independente</span></footer>
 </main></div>
 {(detail || detailLoading || group || dialogError) && <DetailDialog onClose={closeDetails} focusKey={detail?.id ?? (detailLoading ? 'loading' : group ? `group-${group.offset}` : 'error')}><div className="detail-heading"><button className="icon-button" onClick={closeDetails} aria-label="Fechar detalhes"><X size={20}/></button></div>{dialogError && <div role="alert" className="error">{dialogError}</div>}{dialogError && !group && !detail && !detailLoading && <h2 id="detail-title">Registro indisponível</h2>}{detailLoading ? <h2 id="detail-title">Buscando os detalhes do sinistro…</h2> : detail ? <>{group && <button className="back-to-group" onClick={() => setDetail(null)}>← Voltar aos {fmt(group.total)} sinistros do grupo</button>}<EventDetail record={detail} meta={meta} onLocate={locateEvent}/></> : group && <><h2 id="detail-title">{fmt(group.total)} {group.total===1?'sinistro':'sinistros'} {group.context.area ? 'nesta área' : group.context.ids ? 'nos pontos selecionados' : 'neste grupo'}</h2><p>Selecione um registro para consultar o local, os envolvidos e a gravidade.</p><div className="group-meta"><span>{groupLoading ? 'Carregando registros…' : group.total ? `${fmt(group.offset + 1)}–${fmt(group.offset + group.items.length)} de ${fmt(group.total)}` : 'Nenhum sinistro nesta seleção'}</span><span>Data crescente</span></div><div className="event-list">{group.items.map((r: any) => <button key={r.id} className="event-card" onClick={() => showDetail(r.id)} aria-label={`Ver detalhes do sinistro ${r.id}`}><div><span>#{r.id}</span><time>{String(r.date).slice(6, 8)}/{String(r.date).slice(4, 6)}/{String(r.date).slice(0, 4)}</time></div><h3>{meta?.types[r.type]}<ArrowUpRight size={16}/></h3><p>{labelCase(meta?.cities.find((c: any) => c.code === r.municipio)?.name || 'Município não informado')}</p><div className="event-card-bottom"><span className={r.fatal ? 'fatal-tag' : 'normal-tag'}>{r.fatal ? `${r.fatal} vítima${r.fatal > 1 ? 's' : ''} ${r.fatal > 1 ? 'fatais' : 'fatal'}` : meta?.registrations[r.registration]}</span><span>{r.grave + r.leve} feridos</span></div></button>)}</div><div className="group-pagination"><button disabled={groupLoading || group.offset === 0} onClick={() => showGroup(group.context, group.filters, Math.max(0, group.offset - group.limit))}>← Anteriores</button><button disabled={groupLoading || !group.has_more} onClick={() => showGroup(group.context, group.filters, group.offset + group.limit)}>Próximos →</button></div><p className="coordinate-note"><Info size={14}/>Sinistros com coordenadas iguais aparecem separados nesta lista.</p></>}</DetailDialog>}

 {notice && <div className="toast" role="status"><Check size={16}/>{notice}</div>}
 </div>;
}
