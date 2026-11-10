import type { Filters } from './query-state';

export const MONTHS = ['Jan', 'Fev', 'Mar', 'Abr', 'Mai', 'Jun', 'Jul', 'Ago', 'Set', 'Out', 'Nov', 'Dez'];
export function selectedPeriod(filters: Filters, coverageStart: string, coverageEnd: string) {
    const start = filters.inicio || (filters.ano ? `${filters.ano}-01-01` : coverageStart);
    const end = filters.fim || (filters.ano ? `${filters.ano}-12-31` : coverageEnd);
    return { start: start < coverageStart ? coverageStart : start, end: end > coverageEnd ? coverageEnd : end };
}
function previousDate(date: string) {
    const [year, month, day] = date.split('-').map(Number);
    const lastDay = new Date(Date.UTC(year - 1, month, 0)).getUTCDate();
    return `${year - 1}-${String(month).padStart(2, '0')}-${String(Math.min(day, lastDay)).padStart(2, '0')}`;
}
export function comparisonPeriod(filters: Filters, coverageStart: string, coverageEnd: string) {
    const period = selectedPeriod(filters, coverageStart, coverageEnd);
    if (period.start > period.end || period.start.slice(0, 4) !== period.end.slice(0, 4)) return null;
    const start = previousDate(period.start), end = previousDate(period.end);
    if (start < coverageStart || end > coverageEnd) return null;
    // Infosiga expands to nonfatal events in 2019; those cohorts are not equivalent.
    if (period.start >= '2019-01-01' && start < '2019-01-01') return null;
    return { ...period, previousStart: start, previousEnd: end, year: period.start.slice(0, 4), previousYear: start.slice(0, 4),
        filters: { ...filters, ano: '', inicio: start, fim: end } };
}
export function monthlyPoints(months: number[], period: { start: string; end: string }, previous?: number[]) {
    return MONTHS.map((label, index) => {
        const startYear = Number(period.start.slice(0, 4)), endYear = Number(period.end.slice(0, 4));
        const month = index + 1;
        const covered = period.start <= period.end && Array.from({ length: Math.max(0, endYear - startYear + 1) }, (_, n) => startYear + n)
            .some(year => `${year}-${String(month).padStart(2, '0')}-01` <= period.end && `${year}-${String(month).padStart(2, '0')}-31` >= period.start);
        return { label, current: covered ? months[index] ?? 0 : null, previous: previous && covered ? previous[index] ?? 0 : null };
    });
}
