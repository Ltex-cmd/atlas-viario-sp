'use client';
import { TagGroup, Tag } from './boardui/base/tag-group/tag-group';
import { Button } from './AtlasControls';
import type { Filters } from './query-state';

export default function ActiveFilters({ filters, meta, onRemove }: {
    filters: Filters; meta: any; onRemove: (keys: (keyof Filters)[]) => void;
}) {
    const category: Record<string, string[] | undefined> = { tipo: meta?.types, transporte: meta?.transports, via: meta?.roads, registro: meta?.registrations };
    const tags = Object.entries(filters).filter(([, value]) => value !== '').map(([key, value]) => {
        let label = value;
        if (key === 'municipio') label = meta?.cities.find((city: any) => String(city.code) === value)?.name ?? value;
        else if (key === 'inicio' || key === 'fim') label = `${key === 'inicio' ? 'Desde' : 'Até'} ${value.split('-').reverse().join('/')}`;
        else if (category[key]) label = category[key]![Number(value)] ?? value;
        return { id: key as keyof Filters, label };
    });
    if (!tags.length) return null;
    return <div className="atlas-active-filters">
        <TagGroup aria-label="Filtros ativos" items={tags} onRemove={keys => onRemove([...keys] as (keyof Filters)[])}>
            {tag => <Tag id={tag.id} textValue={tag.label} className="atlas-filter-tag">{tag.label}</Tag>}
        </TagGroup>
        <Button size="small" variant="ghost" onClick={() => onRemove(tags.map(tag => tag.id))}>Remover todos</Button>
    </div>;
}
