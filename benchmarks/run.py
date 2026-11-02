#!/usr/bin/env python3
"""Build and measure both engines using exactly 1,000 unchanged CSV records.

All generated files stay in the website checkout. Original inputs are read-only.
"""
from pathlib import Path
import argparse
import datetime
import hashlib
import json
import platform
import random
import shutil
import struct
import subprocess
import sys

REPO = Path(__file__).resolve().parents[1]
HERE = REPO / 'benchmarks'
SEED = 20261030
SIZE = 1000
METHODS = [
    ('pilha', 'Pilha', 'original'), ('fila', 'Fila', 'original'),
    ('lista_seq', 'Lista sequencial', 'original'), ('lista_enc', 'Lista encadeada', 'original'),
    ('lista_cabeca', 'Lista com cabeça', 'original'), ('lista_ord', 'Lista ordenada', 'original'),
    ('lista_gen', 'Lista generalizada', 'original'), ('lista_cruz', 'Lista cruzada', 'original'),
    ('scan', 'Varredura sequencial', 'site'), ('sorted', 'Índice temporal / municipal', 'site'),
    ('grid', 'Grade espacial', 'site'), ('quadtree', 'Quadtree', 'site'),
    ('planner', 'Planejador automático', 'site'),
]
SCENARIOS = [
    {'id': 'all', 'name': 'Amostra inteira', 'description': 'Todos os 1.000 registros, inclusive os que não têm localização aceita.', 'filters': {}},
    {'id': 'period', 'name': 'Abril a junho', 'description': 'Sinistros entre 1 de abril e 30 de junho de 2026.', 'filters': {'inicio': '2026-04-01', 'fim': '2026-06-30'}},
    {'id': 'city', 'name': 'São Carlos', 'description': 'Registros cujo município informado é São Carlos, código IBGE 3548906.', 'filters': {'municipio': 3548906}},
    {'id': 'region', 'name': 'Área de São Carlos', 'description': 'Pontos no retângulo de longitude −48,15 a −47,65 e latitude −22,25 a −21,75. Essa área também inclui municípios vizinhos.', 'filters': {'bbox': [-48.15, -22.25, -47.65, -21.75]}},
]


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def command(arguments, **kwargs):
    return subprocess.run(list(map(str, arguments)), check=True, **kwargs)


def original_snapshot(root):
    """Keep verbatim source for reproducibility; compile the real checkout locally."""
    manifest_path = HERE / 'original-sha256.json'
    paths = sorted(root.glob('include/*.h')) + sorted(p for p in root.glob('source/*.c') if p.name != 'main.c')
    if not paths:
        raise RuntimeError('Original C sources not found')
    hashes = {str(path.relative_to(root)): digest(path) for path in paths}
    if manifest_path.exists() and json.loads(manifest_path.read_text()) != hashes:
        raise RuntimeError('Original sources differ from the benchmark baseline. Review before replacing its provenance.')
    for path in paths:
        target = HERE / 'original' / path.relative_to(root)
        target.parent.mkdir(parents=True, exist_ok=True)
        if not target.exists() or digest(target) != digest(path):
            shutil.copyfile(path, target)
    manifest_path.write_text(json.dumps(hashes, indent=2) + '\n')
    return hashes


def make_sample(original):
    data = REPO / 'public/data'
    metadata = json.loads((data / 'metadata.json').read_text())
    binary = (data / 'year-2026.bin').read_bytes()
    if len(binary) % 48:
        raise RuntimeError('Invalid binary partition')
    indices = sorted(random.Random(SEED).sample(range(len(binary) // 48), SIZE))
    selected = [binary[index * 48:(index + 1) * 48] for index in indices]
    ids = [struct.unpack_from('<I', record, 16)[0] for record in selected]
    if len(set(ids)) != SIZE:
        raise RuntimeError('Sample contains duplicate IDs')
    rows = {}
    set_ids = set(ids)
    source = original / 'dados/otimizados/por_ano/sinistros_2026.csv'
    with source.open('rb') as file:
        header = file.readline()
        for line in file:
            identifier = int(line.split(b';', 1)[0])
            if identifier in set_ids:
                if identifier in rows:
                    raise RuntimeError('Source contains repeated sample ID')
                rows[identifier] = line
    if set(rows) != set(ids):
        raise RuntimeError('CSV and binary partition have different IDs')
    sample_dir = HERE / 'sample'
    sample_dir.mkdir(exist_ok=True)
    csv_path, bin_path = sample_dir / 'sinistros-1000.csv', sample_dir / 'records-1000.bin'
    csv_path.write_bytes(header + b''.join(rows[identifier] for identifier in ids))
    bin_path.write_bytes(b''.join(selected))
    return {
        'size': SIZE, 'year': 2026, 'seed': SEED,
        'population': len(binary) // 48, 'dataset_version': metadata['version'],
        'source_sha256': metadata['source_sha256'], 'partition_sha256': digest(data / 'year-2026.bin'),
        'csv_sha256': digest(csv_path), 'binary_sha256': digest(bin_path),
        'selected_indices': indices, 'selected_ids': ids,
        'located': sum(record[42] != 0 for record in selected),
        'municipalities': len({struct.unpack_from('<I', record, 20)[0] for record in selected}),
        'selection': 'Amostra aleatória sem reposição da partição de 2026; índices em ordem de origem; random.Random do Python com semente fixa.',
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original-root', type=Path, default=REPO.parents[1])
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--use-snapshot', action='store_true', help='Use the byte-identical source copy and existing sample')
    options = parser.parse_args()
    original = HERE / 'original' if options.use_snapshot else options.original_root.resolve()
    hashes = original_snapshot(original)
    # The sample uses existing source records. No interpolation or generated locations.
    if options.use_snapshot:
        sample = json.loads((REPO / 'components/sample-benchmark.json').read_text())['sample']
        if digest(HERE / 'sample/sinistros-1000.csv') != sample['csv_sha256'] or digest(HERE / 'sample/records-1000.bin') != sample['binary_sha256']:
            raise RuntimeError('Saved sample has changed')
    else:
        sample = make_sample(original)
    build = REPO / '.benchmark-build' / ('sanitize' if options.sanitize else 'release')
    command(['cmake', '-S', HERE, '-B', build, '-DCMAKE_BUILD_TYPE=Release',
             f'-DATLAS_ORIGINAL_ROOT={original}', f'-DATLAS_SANITIZE={"ON" if options.sanitize else "OFF"}'])
    command(['cmake', '--build', build, '--parallel', '4'])
    csv_path, bin_path = HERE / 'sample/sinistros-1000.csv', HERE / 'sample/records-1000.bin'
    timing = json.loads(command([build / 'bench_timing', csv_path, bin_path], capture_output=True, text=True).stdout)
    memory = json.loads(command([build / 'bench_memory', csv_path, bin_path], capture_output=True, text=True).stdout)
    if options.sanitize or options.verify_only:
        print('PASS: 13 métodos, 4 consultas, conjuntos exatos de IDs e todas as agregações; nenhuma alocação rastreada ficou viva.')
        return
    if timing['records'] != SIZE or memory['records'] != SIZE:
        raise RuntimeError('Wrong sample size')
    for index, scenario in enumerate(SCENARIOS):
        timed, measured = timing['scenarios'][index], memory['scenarios'][index]
        if timed['sorted_ids'] != measured['sorted_ids']:
            raise RuntimeError('Instrumented and uninstrumented results differ')
        scenario.update({'count': timed['count'], 'checksum': timed['checksum'], 'sorted_ids': timed['sorted_ids'], 'methods': []})
        for row, (key, name, family) in enumerate(METHODS):
            result = dict(timed['methods'][row])
            result.pop('retained_structure_bytes'); result.pop('peak_structure_bytes')
            allocation = measured['methods'][row]
            if allocation['exact_ids_match'] is not True or allocation['aggregates_match'] is not True:
                raise RuntimeError('Instrumented query did not verify exact IDs and aggregates')
            record_bytes = memory['original_record_heap_bytes'] if family == 'original' else SIZE * 48
            scenario['methods'].append({**result, 'id': key, 'name': name, 'family': family, 'exact_ids_match': True,
                'record_bytes': record_bytes, 'retained_structure_bytes': allocation['retained_structure_bytes'],
                'heap_retained_bytes': record_bytes + allocation['retained_structure_bytes'],
                'heap_peak_bytes': record_bytes + allocation['peak_structure_bytes']})
    cache = {}
    for line in (build / 'CMakeCache.txt').read_text().splitlines():
        if line and not line.startswith(('#', '//')) and '=' in line:
            key, value = line.split('=', 1)
            cache[key.split(':', 1)[0]] = value
    compiler = command([cache['CMAKE_C_COMPILER'], '--version'], capture_output=True, text=True).stdout.splitlines()[0]
    cpu = platform.processor() or platform.machine()
    if sys.platform == 'darwin':
        cpu = command(['sysctl', '-n', 'machdep.cpu.brand_string'], capture_output=True, text=True).stdout.strip()
    report = {
        'format': 'atlas-original-sample-v1', 'measured_at': datetime.datetime.now(datetime.timezone.utc).isoformat(),
        'environment': {'os': platform.platform(), 'architecture': platform.machine(), 'cpu': cpu,
                        'compiler': compiler, 'optimization': (cache['CMAKE_C_FLAGS'] + ' ' + cache['CMAKE_C_FLAGS_RELEASE']).strip(), 'runtime': 'C nativo para os dois motores'},
        'methodology': {'repetitions': timing['repetitions'], 'statistic': 'mediana', 'warmups': 1,
            'query_scope': 'Percurso, filtros, contagem, soma das gravidades e histogramas por mês, tipo e município. O adaptador lê ITEMs pelos getters originais.',
            'clock': 'CLOCK_MONOTONIC_RAW, diferença em nanossegundos inteiros antes da conversão para milissegundos.',
            'validation': 'O executável instrumentado coleta os IDs aceitos no percurso de cada estrutura e no visitante de cada método do site. Confere os conjuntos ordenados, contagens, gravidades e histogramas. O executável cronometrado confere as agregações após a consulta.',
            'excluded': 'Leitura do CSV, conversão dos registros, interpretação da QUERY, validação dos resultados, criação da lista de saída, ordenação global da saída, gravação CSV, HTTP e desenho do mapa.',
            'allocation_scope': 'Bytes pedidos a malloc/calloc/realloc pelos registros e estruturas. A memória retida inclui registros e índices; o pico inclui buffers temporários da construção. Não inclui metadados do alocador, pilha, bibliotecas, buffers de QUERY e do ensaio, processo ou navegador.',
            'allocation_run': 'Executável separado com contadores de alocação e coleta dos IDs aceitos. Os tempos vêm do executável sem instrumentação.',
            'site_construction': 'Os cinco métodos compartilham a mesma construção de todos os índices. Essa etapa ocorre uma vez por repetição, fora das consultas.',
            'original_construction': 'Cada consulta recebe uma estrutura nova com os mesmos 1.000 ITEMs, inseridos na ordem da amostra. O percurso acontece uma vez antes da destruição.',
            'generalized_fields': ['ano_sinistro', 'mes_sinistro', 'cod_ibge'],
            'ordered_field': 'id_sinistro', 'crossed_fields': ['cod_ibge', 'id_sinistro'],
            'limits': 'Uma amostra de 1.000 registros não prevê o tempo da base completa. As representações e o custo dos filtros são diferentes. Este ensaio não isola o custo de cada estrutura nem mede WebAssembly em produção.'},
        'sample': sample, 'scenarios': SCENARIOS,
        'provenance': {'original_sha256': hashes,
            'site_engine_sha256': digest(REPO / 'engine-source/engine.c'),
            'site_header_sha256': digest(REPO / 'engine-source/engine.h'),
            'harness_sha256': digest(HERE / 'src/compare.c')},
    }
    output = REPO / 'components/sample-benchmark.json'
    output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n')
    print(json.dumps({'report': str(output), 'sample': SIZE, 'scenarios': {s['id']: s['count'] for s in SCENARIOS}, 'methods': 13}))


if __name__ == '__main__':
    main()
