export type Filters = {
    ano: string;
    municipio: string;
    tipo: string;
    transporte: string;
    via: string;
    registro: string;
    inicio: string;
    fim: string;
};
export type Viewport = {
    bbox: string;
    zoom: number;
    resolution: number;
};
export function query(filters: Filters) {
    const params = new URLSearchParams();
    Object.entries(filters).sort(([a], [b]) => a.localeCompare(b)).forEach(([key, value]) => { if (value)
        params.set(key, value); });
    return params;
}
export const filterKey = (filters: Filters) => query(filters).toString();
export function statisticsAreCurrent(statistics: unknown, statisticsKey: string, filters: Filters, pending: boolean) {
    return statistics !== null && statistics !== undefined && !pending && statisticsKey === filterKey(filters);
}
export const normalizeCity = (value: string) => value.normalize('NFD').replace(/\p{M}/gu, '').trim().replace(/\s+/g, ' ').toLocaleLowerCase('pt-BR');
export function findCity(cities: {
    code: number;
    name: string;
}[], value: string) {
    const matches = cities.filter(city => normalizeCity(city.name) === normalizeCity(value));
    return matches.length === 1 ? matches[0] : undefined;
}
export function isoDate(value: number | string) {
    const s = String(value);
    return `${s.slice(0, 4)}-${s.slice(4, 6)}-${s.slice(6, 8)}`;
}
export function validateDates(filters: Filters, start: string, end: string) {
    for (const date of [filters.inicio, filters.fim]) {
        if (!date)
            continue;
        if (!/^\d{4}-\d{2}-\d{2}$/.test(date) || !Number.isFinite(Date.parse(date)) || new Date(date + 'T12:00:00Z').toISOString().slice(0, 10) !== date)
            return 'Informe uma data válida.';
        if (date < start || date > end)
            return `O período disponível vai de ${start.split('-').reverse().join('/')} a ${end.split('-').reverse().join('/')}.`;
    }
    return filters.inicio && filters.fim && filters.inicio > filters.fim ? 'A data inicial precisa ser anterior ou igual à data final.' : '';
}
export function makeExport({ meta, filters, statistics, statisticsKey, map, mapKey, viewport, busy }: {
    meta: any;
    filters: Filters;
    statistics: any;
    statisticsKey: string;
    map: any;
    mapKey: string;
    viewport: Viewport;
    busy: boolean;
}) {
    const key = filterKey(filters);
    if (busy || !statistics || !map || statisticsKey !== key || mapKey !== key)
        throw Error('Aguarde a atualização da consulta antes de exportar.');
    return { format: 'atlas-query-summary-v2', exported_at: new Date().toISOString(), source: meta.source_url, version: meta.version, filters: { ...filters }, scope: { statistics: 'Todos os eventos que correspondem aos filtros, inclusive sem coordenadas.', map: 'Pontos ou agrupamentos da última área exibida; não é uma lista completa de registros.', ...viewport }, statistics, map };
}
