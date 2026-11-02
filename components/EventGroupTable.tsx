'use client';
import { useMemo } from 'react';
import type { ColumnDef } from '@tanstack/react-table';
import { DataTable } from './boardui/application/data-table/data-table';
import { Button } from './AtlasControls';
import { Chip } from './boardui/base/badges/chip';
import { Skeleton } from './boardui/base/skeleton/skeleton';
import { toast } from './boardui/base/toast/toast';

type EventRow = { id: number; date: number; municipio: number; type: number; fatal: number; grave: number; leve: number; registration: number };
export default function EventGroupTable({ group, meta, loading, error, onOpen, onPage, onRetry }: {
    group: any; meta: any; loading: boolean; error: string; onOpen: (id: number) => void; onPage: (offset: number) => void; onRetry: () => void;
}) {
    const columns = useMemo<ColumnDef<EventRow>[]>(() => [
        { accessorKey: 'id', header: 'Registro', cell: ({ row }) => <Button size="small" variant="ghost" aria-label={`Ver detalhes do sinistro ${row.original.id}`} onClick={() => onOpen(row.original.id)}>#{row.original.id}</Button> },
        { accessorKey: 'date', header: 'Data', cell: ({ getValue }) => String(getValue()).replace(/^(\d{4})(\d{2})(\d{2})$/, '$3/$2/$1') },
        { id: 'city', header: 'Município', accessorFn: row => meta?.cities.find((city: any) => city.code === row.municipio)?.name ?? 'Não informado' },
        { id: 'type', header: 'Tipo', accessorFn: row => meta?.types[row.type] ?? 'Não informado' },
        { accessorKey: 'fatal', header: 'Vítimas fatais', cell: ({ row }) => <Chip color={row.original.fatal ? 'rose' : 'neutral'} variant="subtle">{row.original.fatal}</Chip> },
        { id: 'injured', header: 'Feridos', accessorFn: row => row.grave + row.leve },
    ], [meta, onOpen]);
    function exportSelection(rows: EventRow[]) {
        const blob = new Blob([JSON.stringify({ format: 'atlas-selected-events-v1', exported_at: new Date().toISOString(), filters: group.filters,
            scope: 'Registros selecionados na página carregada da lista do mapa; não representa todos os resultados.', count: rows.length, records: rows,
            legend: { cities: meta.cities, types: meta.types, registrations: meta.registrations } }, null, 2)], { type: 'application/json' });
        const url = URL.createObjectURL(blob), link = document.createElement('a');
        link.href = url; link.download = 'atlas-viario-selecao.json'; document.body.appendChild(link); link.click(); link.remove();
        setTimeout(() => URL.revokeObjectURL(url), 1000);
        toast.success(`${rows.length} ${rows.length === 1 ? 'registro exportado' : 'registros exportados'}.`);
    }
    if (loading) return <div className="atlas-record-loading" role="status"><p>Carregando registros…</p>{[0, 1, 2, 3].map(key => <Skeleton key={key} className="atlas-record-skeleton"/>)}</div>;
    if (error) return <Button variant="secondary" onClick={onRetry}>Tentar carregar a página novamente</Button>;
    if (!group.items.length) return <div className="atlas-record-empty"><h3>Nenhum sinistro nesta seleção</h3><p>Feche o painel e escolha outra área do mapa ou ajuste os filtros.</p></div>;
    return <><DataTable key={group.offset} data={group.items as EventRow[]} columns={columns} label="Sinistros da seleção no mapa" page={Math.floor(group.offset / group.limit) + 1} pageSize={group.limit} total={group.total} onPageChange={page => onPage((page - 1) * group.limit)} onExportSelection={exportSelection}/>
        <p className="coordinate-note">Sinistros com coordenadas iguais aparecem separados nesta lista. Abra o registro para consultar os atributos ou localizá-lo no mapa.</p></>;
}
