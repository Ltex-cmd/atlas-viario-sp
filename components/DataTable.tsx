import type { ReactNode } from 'react';

export type DataColumn = { key: string; label: string };
export type DataRow = { id: string | number; label: ReactNode; values: Record<string, ReactNode> };

export default function DataTable({ caption, columns, rows, firstColumn = 'Método' }: { caption: string; columns: DataColumn[]; rows: DataRow[]; firstColumn?: string }) {
    return <div className="data-comparison">
        <div className="comparison-table results-desktop"><table>
            <caption>{caption}</caption>
            <thead><tr><th scope="col">{firstColumn}</th>{columns.map(column => <th key={column.key} scope="col">{column.label}</th>)}</tr></thead>
            <tbody>{rows.map(row => <tr key={row.id}><th scope="row">{row.label}</th>{columns.map(column => <td key={column.key}>{row.values[column.key]}</td>)}</tr>)}</tbody>
        </table></div>
        <section className="results-mobile" aria-label={caption}>
            <p className="comparison-caption">{caption}</p>
            {rows.map(row => <article className="method-result" key={row.id}>
                <h3>{row.label}</h3>
                <dl>{columns.map(column => <div key={column.key}><dt>{column.label}</dt><dd>{row.values[column.key]}</dd></div>)}</dl>
            </article>)}
        </section>
    </div>;
}
