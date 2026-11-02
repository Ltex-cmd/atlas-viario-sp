'use client';
import { useState } from 'react';
import { ArrowDownToLine, Check, AlertCircle } from './icons';
import DataTable from './DataTable';
import report from './sample-benchmark.json';

const fmt = (value: number, digits = 0) => value.toLocaleString('pt-BR', {
    minimumFractionDigits: digits, maximumFractionDigits: digits,
});
const structures = [
    { id: 'traversal', label: 'Armazenamento e percurso', values: {
        original: 'Pilha, fila, lista sequencial, lista encadeada e lista com cabeça armazenam ponteiros para os registros completos.',
        site: 'Um vetor de registros de 48 bytes reúne os campos usados nas consultas. A varredura sequencial percorre esse vetor.',
    } },
    { id: 'ordering', label: 'Ordenação', values: {
        original: 'A lista ordenada insere cada ponteiro na posição correspondente ao campo escolhido.',
        site: 'Vetores ordenados por data, município e ID permitem localizar intervalos com busca binária.',
    } },
    { id: 'groups', label: 'Agrupamento', values: {
        original: 'A lista generalizada organiza os registros em grupos hierárquicos por campo.',
        site: 'Vetores de contadores guardam os totais por mês, tipo e município. Células geográficas agrupam os pontos exibidos no mapa.',
    } },
    { id: 'dimensions', label: 'Vários campos', values: {
        original: 'A lista cruzada mantém uma ordenação dos mesmos registros para cada campo configurado.',
        site: 'Índices independentes de data e município reduzem os candidatos. O planejador também considera a quadtree quando há uma área selecionada.',
    } },
    { id: 'spatial', label: 'Consulta geográfica', values: {
        original: 'O percurso aplica os filtros de latitude e longitude aos registros. As oito estruturas não têm um índice espacial dedicado.',
        site: 'Uma grade de 128 por 128 células e uma quadtree selecionam candidatos que podem estar na área do mapa.',
    } },
    { id: 'identifier', label: 'Busca por ID', values: {
        original: 'O programa aplica o filtro de ID durante o percurso da estrutura escolhida.',
        site: 'O motor tem uma tabela hash e um vetor ordenado por ID. Na API publicada, os detalhes são organizados por faixa de IDs.',
    } },
];

export default function SampleBenchmarkPanel() {
    const [selected, setSelected] = useState('all');
    const scenario = report.scenarios.find(value => value.id === selected)!;
    const verified = scenario.methods.every(method => method.exact_ids_match && method.aggregates_match);
    const download = new URL('./sample-benchmark.json', import.meta.url).href;
    return <>
        <section className="sample-benchmark" aria-labelledby="sample-title">
            <div className="sample-heading">
                <div>
                    <h2 id="sample-title">Comparação com 1.000 sinistros</h2>
                    <p>Oito estruturas do projeto original e cinco métodos do site consultam os mesmos registros. Os dois motores executam em C no mesmo computador.</p>
                </div>
                <a className="text-link sample-download" href={download} download="atlas-comparacao-1000.json"><ArrowDownToLine size={16}/>Baixar medições</a>
            </div>
            <p className="sample-scope">Amostra fixa de 2026, com {fmt(report.sample.municipalities)} municípios e {fmt(report.sample.located)} registros localizados. Os filtros do mapa não alteram estes resultados.</p>
            <div className="benchmark-scenarios" role="group" aria-label="Consulta na amostra de 1.000 sinistros">
                {report.scenarios.map(value => <button key={value.id} aria-pressed={value.id === selected} onClick={() => setSelected(value.id)}>{value.name}</button>)}
            </div>
            <div className="sample-outcome" aria-live="polite" aria-atomic="true">
                <p>{scenario.id === 'all' ? 'Todos os 1.000 registros, inclusive os que não podem ser localizados no mapa.' : scenario.description}</p>
                <p className={verified ? 'sample-verification success' : 'sample-verification error-text'}>
                    {verified ? <Check size={18}/> : <AlertCircle size={18}/>}
                    {verified ? `${fmt(scenario.count)} sinistros. Os 13 métodos retornaram os mesmos IDs e as mesmas estatísticas.` : 'Há uma divergência nos resultados desta consulta.'}
                </p>
            </div>
            <div className="comparison-table sample-table" tabIndex={0} role="region" aria-label="Tempos e memória dos 13 métodos">
                <table>
                    <caption>Mediana de {report.methodology.repetitions} execuções.</caption>
                    <thead><tr><th scope="col">Método</th><th scope="col">Construção <span>ms</span></th><th scope="col">Consulta <span>µs</span></th><th scope="col">Examinados</th><th scope="col">Memória <span>KiB</span></th></tr></thead>
                    {(['original', 'site'] as const).map(family => <tbody key={family}>
                        <tr className="sample-family"><th scope="rowgroup" colSpan={5}>{family === 'original' ? 'Estruturas originais' : 'Métodos do site'}</th></tr>
                        {scenario.methods.filter(method => method.family === family).map(method => <tr key={method.id}>
                            <th scope="row">{method.name}</th>
                            <td>{fmt(method.build_ms, 3)}</td>
                            <td>{fmt(method.query_ms * 1000, 2)}</td>
                            <td>{fmt(method.examined)}</td>
                            <td>{fmt(method.heap_retained_bytes / 1024, 1)}</td>
                        </tr>)}
                    </tbody>)}
                </table>
            </div>
            <div className="measurement-notes">
                <p>As medições começam após uma execução de aquecimento. A construção está em milissegundos e a consulta em microssegundos.</p>
                <p>Os cinco métodos do site compartilham a construção de todos os índices e a mesma memória. Esses valores aparecem em cada linha para referência. A memória inclui os registros e as estruturas, sem o consumo total do processo.</p>
                {selected !== 'region' && <p>Sem um filtro geográfico, os métodos Grade espacial e Quadtree consultam o índice temporal ou municipal. O cenário Área de São Carlos aciona os índices espaciais.</p>}
                <p>Os resultados desta amostra não preveem o tempo da base completa. Tempos próximos, sobretudo abaixo de um microssegundo, podem mudar de ordem em outra execução.</p>
            </div>
            <details className="benchmark-details sample-methodology">
                <summary>Como medimos e conferimos os resultados</summary>
                <div className="measurement-notes">
                    <p>A seleção sorteia 1.000 dos {fmt(report.sample.population)} sinistros disponíveis de 2026, sem repetição, com a semente {report.sample.seed}. A inserção segue a ordem desses registros na fonte. O ano está incompleto na base usada no ensaio.</p>
                    <p>A construção cria a estrutura e insere os registros já carregados. A consulta aplica os filtros e calcula contagens, gravidades e totais por mês, tipo e município. Um adaptador usa os acessores originais para produzir essas mesmas estatísticas. Seu custo entra no tempo das estruturas originais.</p>
                    <p>A leitura do CSV, a interpretação da consulta, a criação e a gravação de arquivos de saída ficam fora desses tempos. Também ficam fora a comunicação com o servidor e o desenho do mapa. Uma execução separada mede a memória e coleta os IDs aceitos por cada método para conferir os resultados.</p>
                    <p>A lista ordenada usa o ID. A generalizada agrupa por ano, mês e código do município. A cruzada usa município e ID, com percurso pelo município. Cada consulta recebe uma estrutura original nova; seus arquivos de código permanecem intactos.</p>
                    <p>O ensaio compara as representações dos registros, os filtros e as agregações de cada motor. Ele não isola apenas o custo da estrutura de dados. O motor publicado executa em WebAssembly no servidor; os números desta tabela vêm dos dois motores compilados para execução nativa.</p>
                    <p>{report.environment.cpu}, {report.environment.os}. Compilador {report.environment.compiler}, opções <code>{report.environment.optimization}</code>. Medição de {new Date(report.measured_at).toLocaleString('pt-BR', { timeZone: 'America/Sao_Paulo' })}.</p>
                    <p>O arquivo de medições inclui os IDs da amostra, as execuções individuais, o pico de memória e os hashes do código e dos dados.</p>
                </div>
            </details>
        </section>

        <section className="engine-comparison" aria-labelledby="engine-choice-title">
            <h2 id="engine-choice-title">Por que o site usa outro motor</h2>
            <div className="engine-explanation">
                <p>O programa original lê o CSV a cada execução, guarda os campos completos em cada registro e percorre a estrutura escolhida para aplicar a consulta. O programa também gera arquivos com os resultados.</p>
                <p>O mapa precisa responder a filtros, deslocamentos e mudanças de zoom. O site prepara registros compactos, carrega uma partição por ano e reutiliza seus índices enquanto ela permanece em memória. A API envia os totais, os agrupamentos ou os registros solicitados para aquela região.</p>
                <p>O leitor de CSV e a lista sequencial originais continuam na preparação dos dados. As consultas interativas usam o motor próprio em C. A comparação acima executa as oito estruturas originais sobre uma amostra pequena para estudar seus custos.</p>
            </div>
            <div className="structure-comparison">
                <DataTable caption="Estruturas e funções nos dois projetos" firstColumn="Função" columns={[{ key: 'original', label: 'Projeto original' }, { key: 'site', label: 'Site' }]} rows={structures}/>
            </div>
            <p className="methodology-note">O planejador automático é uma política de escolha entre índices. Ele não é uma estrutura de dados adicional.</p>
        </section>
    </>;
}
