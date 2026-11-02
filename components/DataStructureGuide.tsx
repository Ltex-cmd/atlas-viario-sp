'use client';
import { useState } from 'react';
import DataTable from './DataTable';

export type StructureGuide = {
    title: string;
    introduction: string[];
    complexityIntroduction?: string[];
    notation: { symbol: string; meaning: string }[];
    entries: {
        id: string;
        name: string;
        family: 'original' | 'site' | 'planner';
        explanation: string[];
        operations: { operation: string; complexity: string; condition: string }[];
        inProgram: string[];
        measuredExample?: string;
        citations: { referenceId: string; locator: string }[];
        code: { path: string; function: string; lines: string | number }[];
    }[];
    references: { id: string; citationLabel: string; abnt: string; emphasis?: string; url?: string; accessed?: string }[];
};

const families = [
    { id: 'original', name: 'Projeto original' },
    { id: 'site', name: 'Motor do site' },
    { id: 'planner', name: 'Escolha do método' },
] as const;

function ReferenceText({ text, emphasis }: { text: string; emphasis?: string }) {
    const at = emphasis ? text.indexOf(emphasis) : -1;
    if (at < 0 || !emphasis) return <>{text}</>;
    return <>{text.slice(0, at)}<strong>{emphasis}</strong>{text.slice(at + emphasis.length)}</>;
}

export default function DataStructureGuide({ guide }: { guide: StructureGuide }) {
    const [selected, setSelected] = useState(guide.entries[0].id);
    const entry = guide.entries.find(item => item.id === selected) ?? guide.entries[0];
    const position = guide.entries.indexOf(entry);

    return <section className="structure-guide" aria-labelledby="structure-guide-title">
        <header>
            <h2 id="structure-guide-title">{guide.title}</h2>
            <div className="guide-prose">{guide.introduction.map(text => <p key={text}>{text}</p>)}</div>
        </header>

        <details className="benchmark-details guide-notation">
            <summary>Como ler as complexidades</summary>
            {guide.complexityIntroduction && <div className="guide-prose">{guide.complexityIntroduction.map(text => <p key={text}>{text}</p>)}</div>}
            <dl>{guide.notation.map(item => <div key={item.symbol}><dt>{item.symbol}</dt><dd>{item.meaning}</dd></div>)}</dl>
        </details>

        <div className="guide-controls" id="structure-reader">
            <label htmlFor="guide-structure">Estrutura ou método
                <select id="guide-structure" value={entry.id} onChange={event => setSelected(event.target.value)}>
                    {families.map(family => <optgroup key={family.id} label={family.name}>
                        {guide.entries.filter(item => item.family === family.id).map(item => <option key={item.id} value={item.id}>{item.name}</option>)}
                    </optgroup>)}
                </select>
            </label>
            <nav aria-label="Percorrer explicações">
                <button onClick={() => setSelected(guide.entries[position - 1].id)} disabled={position === 0}>Anterior</button>
                <span aria-live="polite">{position + 1} de {guide.entries.length}</span>
                <button onClick={() => setSelected(guide.entries[position + 1].id)} disabled={position === guide.entries.length - 1}>Próxima</button>
            </nav>
        </div>

        <article className="guide-entry" aria-labelledby="guide-entry-title" key={entry.id}>
            <h3 id="guide-entry-title">{entry.name}</h3>
            <div className="guide-prose">{entry.explanation.map(text => <p key={text}>{text}</p>)}</div>
            <div className="guide-complexities">
                <DataTable firstColumn="Operação" caption={`Complexidades de ${entry.name.toLocaleLowerCase('pt-BR')}`}
                    columns={[{ key: 'complexity', label: 'Custo' }, { key: 'condition', label: 'Condições' }]}
                    rows={entry.operations.map((operation, i) => ({ id: i, label: operation.operation,
                        values: { complexity: operation.complexity, condition: operation.condition } }))}/>
            </div>
            <h4>No programa</h4>
            <div className="guide-prose">{entry.inProgram.map(text => <p key={text}>{text}</p>)}</div>
            {entry.measuredExample && <section className="guide-measured-example" aria-label="Exemplo na amostra de 1.000 sinistros">
                <h4>Na amostra de 1.000 sinistros</h4>
                <p>{entry.measuredExample}</p>
            </section>}
            {entry.citations.length > 0 && <p className="guide-citations">Fontes consultadas: {entry.citations.map((citation, i) => {
                const reference = guide.references.find(item => item.id === citation.referenceId)!;
                return <span key={`${citation.referenceId}-${citation.locator}`}>
                    {i > 0 ? '; ' : ''}<a href={`#guide-ref-${reference.id}`}>{reference.citationLabel}{citation.locator ? `, ${citation.locator}` : ''}</a>
                </span>;
            })}.</p>}
            <details className="benchmark-details guide-code">
                <summary>Onde isso aparece no código</summary>
                <ul>{entry.code.map(item => <li key={`${item.path}-${item.function}`}>
                    <code>{item.path}</code>, {item.lines ? `linhas ${item.lines}. ` : ''}<code>{item.function}</code>
                </li>)}</ul>
            </details>
        </article>

        <section className="guide-references" aria-labelledby="guide-references-title">
            <h3 id="guide-references-title">Referências</h3>
            <ul>{guide.references.map(reference => <li key={reference.id} id={`guide-ref-${reference.id}`} tabIndex={-1}>
                <p><ReferenceText text={reference.abnt} emphasis={reference.emphasis}/></p>
                {reference.url && <a href={reference.url} target="_blank" rel="noopener noreferrer">Consultar fonte<span className="guide-source-name"> · {reference.citationLabel}</span></a>}
            </li>)}</ul>
            <a className="text-link" href="#structure-reader">Voltar à explicação selecionada</a>
        </section>
    </section>;
}
