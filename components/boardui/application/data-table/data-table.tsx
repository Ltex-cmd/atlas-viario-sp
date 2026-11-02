'use client';
// BoardUI Data Table (MIT), adapted from its customer example to supplied
// records. TanStack owns sorting and selection; the API owns pagination.
import { useState, type ReactNode } from 'react';
import { flexRender, getCoreRowModel, getSortedRowModel, useReactTable, type ColumnDef, type SortingState } from '@tanstack/react-table';
import { Table, TableBody, TableCell, TableColumn, TableHeader, TableRow } from '@/components/boardui/base/table/table';
import { Checkbox } from '@/components/boardui/base/checkbox/checkbox';
import { Button } from '@/components/boardui/base/buttons/button';
import { Pagination } from '@/components/boardui/base/pagination/pagination';
import { ChevronSortDown } from '@/components/boardui/foundations/icons/chevrons';

export function DataTable<T extends { id: string | number }>({ data, columns, label, page, total, pageSize, onPageChange, onExportSelection, toolbar }: {
    data: T[]; columns: ColumnDef<T>[]; label: string; page: number; total: number; pageSize: number;
    onPageChange: (page: number) => void; onExportSelection: (rows: T[]) => void; toolbar?: ReactNode;
}) {
    const [sorting, setSorting] = useState<SortingState>([{ id: 'date', desc: false }]);
    const [rowSelection, setRowSelection] = useState<Record<string, boolean>>({});
    const table = useReactTable({ data, columns, getRowId: row => String(row.id),
        state: { sorting, rowSelection }, onSortingChange: setSorting, onRowSelectionChange: setRowSelection,
        getCoreRowModel: getCoreRowModel(), getSortedRowModel: getSortedRowModel(), enableRowSelection: true });
    const selected = table.getSelectedRowModel().rows;
    return <section className="atlas-record-table">
        <div className="atlas-record-toolbar">
            <p>{((page - 1) * pageSize + 1).toLocaleString('pt-BR')}–{Math.min(page * pageSize, total).toLocaleString('pt-BR')} de {total.toLocaleString('pt-BR')} sinistros</p>
            {toolbar}
            <Button variant="secondary" size="small" disabled={!selected.length} onClick={() => onExportSelection(selected.map(row => row.original))}>
                Exportar seleção{selected.length ? ` (${selected.length})` : ''}
            </Button>
        </div>
        <p className="atlas-table-scope">Ordenação e seleção aplicadas aos {data.length} registros desta página. As páginas seguem a data crescente.</p>
        <p className="atlas-table-mobile-hint">Deslize a tabela para ver todas as colunas.</p>
        <div className="atlas-table-scroll" role="region" aria-label="Tabela com rolagem horizontal" tabIndex={0}>
            <Table containerClassName="atlas-inner-table" aria-label={label} size="sm" selectionMode="none">
                <TableHeader>
                    <TableColumn id="selection"><Checkbox slot={null} aria-label="Selecionar todos desta página" isSelected={table.getIsAllRowsSelected()} isIndeterminate={table.getIsSomeRowsSelected() && !table.getIsAllRowsSelected()} onChange={value => table.toggleAllRowsSelected(value)}/></TableColumn>
                    {table.getHeaderGroups()[0].headers.map(header => {
                        const sorted = header.column.getIsSorted();
                        return <TableColumn key={header.id} id={header.id} isRowHeader={header.id === 'id'} aria-sort={sorted === 'asc' ? 'ascending' : sorted === 'desc' ? 'descending' : 'none'}>
                            {header.column.getCanSort() ? <button className="atlas-sort-button" onClick={header.column.getToggleSortingHandler()} aria-label={`Ordenar por ${String(header.column.columnDef.header)}${sorted ? sorted === 'asc' ? ', crescente' : ', decrescente' : ''}`}>
                                {flexRender(header.column.columnDef.header, header.getContext())}<ChevronSortDown className={sorted === 'asc' ? 'atlas-sort-ascending' : ''}/>
                            </button> : flexRender(header.column.columnDef.header, header.getContext())}
                        </TableColumn>;
                    })}
                </TableHeader>
                <TableBody>
                    {table.getRowModel().rows.map(row => <TableRow key={row.id} id={row.id} className={row.getIsSelected() ? 'atlas-row-selected' : ''}>
                        <TableCell><Checkbox slot={null} aria-label={`Selecionar sinistro ${row.id}`} isSelected={row.getIsSelected()} onChange={value => row.toggleSelected(value)}/></TableCell>
                        {row.getVisibleCells().map(cell => <TableCell key={cell.id}>{flexRender(cell.column.columnDef.cell, cell.getContext())}</TableCell>)}
                    </TableRow>)}
                </TableBody>
            </Table>
        </div>
        <div className="atlas-table-pagination"><Pagination page={page} totalPages={Math.ceil(total / pageSize)} onChange={onPageChange} previousLabel="Anteriores" nextLabel="Próximos" aria-label="Páginas dos sinistros" pageLabel={page => `Ir para página ${page}`}/></div>
    </section>;
}
