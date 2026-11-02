'use client';
import { useId, type ReactNode } from 'react';
import { Table, TableHeader, TableColumn, TableBody, TableRow, TableCell } from './boardui/base/table/table';

export type DataColumn = { key: string; label: string };
export type DataRow = { id: string | number; label: ReactNode; values: Record<string, ReactNode> };

export default function DataTable({ caption, columns, rows, firstColumn = 'Método' }: { caption: string; columns: DataColumn[]; rows: DataRow[]; firstColumn?: string }) {
    const captionId = useId();
    return <div className="data-comparison">
        <div className="atlas-comparison-table results-desktop">
            <p id={captionId} className="atlas-table-caption">{caption}</p>
            <Table aria-labelledby={captionId} size="sm">
                <TableHeader>
                    <TableColumn isRowHeader>{firstColumn}</TableColumn>
                    {columns.map(column => <TableColumn key={column.key}>{column.label}</TableColumn>)}
                </TableHeader>
                <TableBody>
                    {rows.map(row => <TableRow key={row.id}>
                        <TableCell>{row.label}</TableCell>
                        {columns.map(column => <TableCell key={column.key}>{row.values[column.key]}</TableCell>)}
                    </TableRow>)}
                </TableBody>
            </Table>
        </div>
        <section className="results-mobile" aria-label={caption}>
            <p className="comparison-caption">{caption}</p>
            {rows.map(row => <article className="method-result" key={row.id}>
                <h3>{row.label}</h3>
                <dl>{columns.map(column => <div key={column.key}><dt>{column.label}</dt><dd>{row.values[column.key]}</dd></div>)}</dl>
            </article>)}
        </section>
    </div>;
}
