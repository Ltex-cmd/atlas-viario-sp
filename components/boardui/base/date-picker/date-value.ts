import { CalendarDate } from '@internationalized/date';

/** Reject impossible typed dates instead of letting CalendarDate constrain them. */
export function parseChipDate(text: string): CalendarDate | null {
    const match = /^(\d{1,2})\/(\d{1,2})\/(\d{4})$/.exec(text.trim());
    if (!match) return null;
    const day = Number(match[1]), month = Number(match[2]), year = Number(match[3]);
    if (year < 1 || month < 1 || month > 12 || day < 1 || day > 31) return null;
    const date = new CalendarDate(year, month, day);
    return date.year === year && date.month === month && date.day === day ? date : null;
}
