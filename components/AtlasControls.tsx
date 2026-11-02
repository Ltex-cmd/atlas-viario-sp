'use client';

import { useMemo, type ReactNode } from 'react';
import { MotionConfig } from 'motion/react';
import { parseDate } from '@internationalized/date';
import { Button } from './boardui/base/buttons/button';
import { ComboBox, ComboBoxItem } from './boardui/base/combobox/combobox';
import { DatePicker } from './boardui/base/date-picker/date-picker';
import { Drawer, DrawerBody, DrawerFooter, DrawerHeader } from './boardui/base/drawer/drawer';
import { Select, SelectItem } from './boardui/base/select/select';
import { SegmentedControl, SegmentedControlItem } from './boardui/base/segmented-control/segmented-control';
import { DirectionProvider } from './boardui/foundations/direction/direction';
import { ArrowRight, RotateCcw, Search, X } from './icons';
import { findCity, type Filters } from './query-state';

export { Button };

export function AtlasUI({ children }: { children: ReactNode }) {
    return <MotionConfig reducedMotion="user"><DirectionProvider locale="pt-BR">{children}</DirectionProvider></MotionConfig>;
}

type Choice = readonly [string, string];

export function AtlasSegments({ label, value, options, onChange, className }: {
    label: string; value: string; options: readonly Choice[];
    onChange: (value: string) => void; className?: string;
}) {
    return <SegmentedControl aria-label={label} selectedKeys={[value]} className={className}
        onSelectionChange={keys => {
            const selected = [...keys][0];
            if (selected !== undefined) onChange(String(selected));
        }}>
        {options.map(([key, text]) => <SegmentedControlItem key={key} id={key}>{text}</SegmentedControlItem>)}
    </SegmentedControl>;
}

function DateField({ label, value, min, max, onChange }: {
    label: string; value: string; min: string; max: string; onChange: (value: string) => void;
}) {
    return <div className="atlas-date-field">
        <span className="atlas-field-label">{label}</span>
        <div className="atlas-date-control">
            <DatePicker aria-label={label} value={value ? parseDate(value) : null}
                minValue={parseDate(min)} maxValue={parseDate(max)}
                onChange={date => onChange(date?.toString() ?? '')}/>
            {value && <Button variant="ghost" size="small" iconOnly leadingIcon={X}
                aria-label={`Limpar ${label.toLocaleLowerCase('pt-BR')}`} onClick={() => onChange('')}/>}
        </div>
    </div>;
}

export function AtlasFilters({ open, onOpenChange, meta, draft, cityText, onCityTextChange,
    onChange, onApply, onReset, error, dateStart, dateEnd }: {
    open: boolean; onOpenChange: (open: boolean) => void; meta: any; draft: Filters;
    cityText: string; onCityTextChange: (text: string) => void;
    onChange: (key: keyof Filters, value: string) => void;
    onApply: () => void; onReset: () => void; error: string; dateStart: string; dateEnd: string;
}) {
    const cities = useMemo(() => (meta?.cities ?? []).map((city: { code: number; name: string }) => ({
        id: String(city.code), name: city.name.toLocaleLowerCase('pt-BR').replace(/(^|\s)\p{L}/gu, (letter: string) => letter.toUpperCase()),
    })), [meta?.cities]);
    const years = (meta?.years ?? [{ year: 2026 }]).slice().reverse() as { year: number }[];
    const categories: { key: keyof Filters; label: string; all: string; options: string[] }[] = [
        { key: 'tipo', label: 'Tipo de sinistro', all: 'Todos os tipos', options: meta?.types ?? [] },
        { key: 'transporte', label: 'Transporte', all: 'Todos os modos', options: meta?.transports ?? [] },
        { key: 'via', label: 'Tipo de via', all: 'Todas as vias', options: meta?.roads ?? [] },
        { key: 'registro', label: 'Classificação', all: 'Todos os registros', options: meta?.registrations ?? [] },
    ];
    return <Drawer placement="start" size="md" isOpen={open} onOpenChange={onOpenChange}
        shouldCloseOnInteractOutside={element => !element.closest('.atlas-filter-popover, .atlas-date-popover')}
        className="atlas-filter-drawer" aria-label="Filtros da consulta">
        <DrawerHeader title="Filtros da consulta" description="Escolha a região e o período para consultar."
            closeLabel="Fechar painel de filtros"/>
        <DrawerBody>
            <div className="atlas-filter-fields">
                <ComboBox label="Município" aria-label="Buscar município" placeholder="Todo o estado de São Paulo"
                    leadingIcon={Search} defaultItems={cities} allowsCustomValue
                    selectedKey={draft.municipio || null} inputValue={cityText}
                    emptyMessage="Nenhum município encontrado." popoverClassName="atlas-filter-popover"
                    onInputChange={text => {
                        onCityTextChange(text);
                        const city = findCity(meta?.cities ?? [], text);
                        onChange('municipio', city ? String(city.code) : '');
                    }}
                    onSelectionChange={key => {
                        const city = cities.find((item: { id: string }) => item.id === String(key));
                        if (city) {
                            onCityTextChange(city.name);
                            onChange('municipio', city.id);
                        }
                    }}>
                    {(city: { id: string; name: string }) => <ComboBoxItem id={city.id} textValue={city.name}>{city.name}</ComboBoxItem>}
                </ComboBox>
                {cityText && <Button variant="ghost" size="small" onClick={() => {
                    onCityTextChange(''); onChange('municipio', '');
                }}>Consultar todo o estado</Button>}
                <Select label="Ano" aria-label="Ano de referência" selectedKey={draft.ano || 'all'} popoverClassName="atlas-filter-popover"
                    onSelectionChange={key => { onChange('ano', key === 'all' ? '' : String(key)); onChange('inicio', ''); onChange('fim', ''); }}>
                    <SelectItem id="all">Todos os anos</SelectItem>
                    {years.map(({ year }) => <SelectItem key={year} id={String(year)} textValue={String(year)}>
                        {year}{String(year) === dateEnd.slice(0, 4) && !dateEnd.endsWith('12-31') ? ' · parcial' : ''}
                    </SelectItem>)}
                </Select>
                <div className="atlas-filter-dates">
                    <DateField label="Data inicial" value={draft.inicio} min={dateStart} max={dateEnd}
                        onChange={value => { onChange('inicio', value); onChange('ano', ''); }}/>
                    <DateField label="Data final" value={draft.fim} min={dateStart} max={dateEnd}
                        onChange={value => { onChange('fim', value); onChange('ano', ''); }}/>
                </div>
                <p className="atlas-date-hint">Cobertura de {dateStart.split('-').reverse().join('/')} a {dateEnd.split('-').reverse().join('/')}.</p>
                {categories.map(({ key, label, all, options }) => <Select key={key} label={label} popoverClassName="atlas-filter-popover"
                    selectedKey={draft[key] || 'all'} onSelectionChange={value => onChange(key, value === 'all' ? '' : String(value))}>
                    <SelectItem id="all">{all}</SelectItem>
                    {options.map((name, index) => <SelectItem key={index} id={String(index)}>{name}</SelectItem>)}
                </Select>)}
            </div>
        </DrawerBody>
        <DrawerFooter className="atlas-filter-footer">
            {error && <p className="filter-error" role="alert">{error}</p>}
            <Button onClick={onApply} trailingIcon={ArrowRight}>Aplicar filtros</Button>
            <Button variant="ghost" onClick={onReset} leadingIcon={RotateCcw}>Limpar seleção</Button>
        </DrawerFooter>
    </Drawer>;
}
