import type { Journey, Place, TravelMode } from './journey.ts';

const number = (n: number) => n.toLocaleString('pt-BR');
export function guessFeedback(guess: number, actual: number): string {
    if (guess === actual) return `Na mosca: ${number(actual)}.`;
    const difference = Math.abs(guess - actual);
    if ((actual < 5 && difference <= 1) || (actual > 0 && difference / actual <= 0.2)) return `Chegou perto: você disse ${number(guess)}, foram ${number(actual)}.`;
    return `Foram ${guess < actual ? 'mais' : 'menos'}: você disse ${number(guess)}, foram ${number(actual)}.`;
}
export function transportBreakdown(points: readonly { transport: number }[]): number[] {
    const counts = Array<number>(8).fill(0);
    for (const point of points) for (let bit = 0; bit < 8; bit++) if (point.transport & (1 << bit)) counts[bit]++;
    return counts;
}
export type DuelEntry = {
    id: string;
    mode: TravelMode;
    from: Place;
    to: Place;
    journey?: Journey;
    total?: number;
    error?: string;
};
export const modeLabels: Record<TravelMode, string> = { foot: 'A pé', bike: 'Bicicleta', car: 'Carro' };
export function duelLabel(entries: readonly DuelEntry[], index: number): string {
    const entry = entries[index];
    if (entries.filter(e => e.mode === entry.mode).length > 1) return `o caminho ${index + 1}`;
    return { foot: 'a pé', bike: 'de bicicleta', car: 'de carro' }[entry.mode];
}
export function duelHeadline(entries: readonly DuelEntry[]): string | null {
    const completed = entries.map((entry, index) => ({ entry, index })).filter(v => v.entry.total !== undefined);
    if (completed.length < 2) return null;
    completed.sort((a, b) => a.entry.total! - b.entry.total!);
    const low = completed[0], high = completed[completed.length - 1];
    const min = low.entry.total!, max = high.entry.total!;
    if (min === max || (min > 0 && max / min < 1.2)) return 'Os caminhos têm números parecidos.';
    const label = duelLabel(entries, high.index);
    const first = label[0].toLocaleUpperCase('pt-BR') + label.slice(1);
    if (min === 0) return `${first} tem ${number(max)} registros; ${duelLabel(entries, low.index)}, nenhum.`;
    return `${first} tem ${(max / min).toLocaleString('pt-BR', { minimumFractionDigits: 1, maximumFractionDigits: 1 })}× mais registros que ${duelLabel(entries, low.index)}.`;
}
