# Comparação com 1.000 sinistros

O ensaio usa oito estruturas originais e cinco métodos do motor do site. Os arquivos de `original/` são cópias byte a byte dos arquivos C e H do projeto acadêmico. `original-sha256.json` registra seus hashes. O programa principal original não é necessário para executar o ensaio.

`sample/` contém exatamente 1.000 registros de 2026 em CSV e no formato compacto de 48 bytes usado no site. A semente é 20261030. A seleção não repete registros e preserva a ordem dos índices sorteados na partição de origem. O relatório registra os índices, IDs e hashes para reproduzir a seleção.

## Reproduzir

Na raiz do site, com Python 3, CMake e um compilador C:

```sh
python3 benchmarks/run.py --use-snapshot --verify-only
python3 benchmarks/run.py --use-snapshot --sanitize --verify-only
```

O primeiro comando compila as cópias preservadas e confere os resultados. O segundo também usa AddressSanitizer e UndefinedBehaviorSanitizer. Os executáveis e arquivos intermediários ficam em `.benchmark-build/`.

Para compilar os arquivos do projeto original diretamente, passando seu diretório como entrada somente para leitura:

```sh
python3 benchmarks/run.py --original-root /caminho/para/projeto-2-alg
```

Esse comando recria a mesma amostra a partir de `dados/otimizados/por_ano/sinistros_2026.csv` e `public/data/year-2026.bin`. Ele recusa fontes originais que não coincidam com o manifesto. As saídas ficam no checkout do site. Não execute o Makefile do projeto original.

Sem `--verify-only`, o comando grava novas medições em `components/sample-benchmark.json`. Os números mudam conforme o computador e as condições da execução. O comando com `--sanitize` nunca substitui as medições publicadas.

Também é possível executar somente o C, sem regenerar dados:

```sh
cmake -S benchmarks -B .benchmark-build/check -DCMAKE_BUILD_TYPE=Release -DATLAS_SANITIZE=ON
cmake --build .benchmark-build/check --parallel 4
ctest --test-dir .benchmark-build/check --output-on-failure
```

## O que os tempos incluem

Cada mediana usa 21 execuções, após um aquecimento descartado. O relógio é `CLOCK_MONOTONIC_RAW`. As diferenças são calculadas em nanossegundos inteiros. A ordem dos métodos alterna entre repetições.

Construção significa criar a estrutura e inserir os registros já carregados. Cada estrutura original é nova e percorrida uma vez, pois nem todas oferecem uma operação para reiniciar o cursor. A lista ordenada usa `id_sinistro`; a generalizada usa ano, mês e código do município; a cruzada mantém município e ID e percorre a dimensão município.

A consulta original chama `estrutura_percorrer` e `item_satisfaz_filter` dos arquivos originais. Um adaptador lê os atributos pelos acessores originais e calcula as mesmas contagens, somas de gravidade e histogramas que `engine_query`. O custo desse adaptador entra no tempo da consulta. Por isso os números comparam as representações, os filtros e as agregações dos dois motores, sem isolar o custo da estrutura de dados.

Os cinco métodos do site compartilham uma construção de todos os índices por repetição. Grade e quadtree usam o índice temporal ou municipal quando o cenário não tem filtro geográfico, conforme o código usado em produção. O quarto cenário aciona os índices espaciais. Os valores de construção e memória repetidos nas cinco linhas não são cinco alocações simultâneas.

A leitura do CSV, a conversão dos registros, a interpretação de QUERY, a criação da lista de saída, a gravação CSV, o HTTP e o mapa ficam fora das medições. Todos os tempos são de execução nativa em C. Não representam o tempo de WebAssembly em produção nem permitem extrapolar o desempenho da base completa.

## Conferência e memória

O executável de tempo usa os fontes originais e o motor do site sem instrumentação. Ele confere as agregações depois de cada consulta.

O executável de memória substitui as chamadas de alocação durante a compilação e coleta os IDs que cada método aceita. Para o motor do site, o CMake gera uma cópia de `engine.c` na pasta de build e acrescenta uma chamada ao observador depois do predicado real. A cópia mantém os algoritmos e nunca substitui `engine-source/engine.c`. Isso permite conferir os conjuntos exatos de IDs de cada caminho, inclusive grade e quadtree, fora da execução cronometrada.

A memória retida soma os registros e os bytes pedidos pelos índices ou estruturas. O pico inclui buffers temporários da construção. Os contadores não medem metadados do alocador, pilha, bibliotecas, QUERY, buffers do ensaio, processo ou navegador. O programa verifica que todas as alocações rastreadas foram liberadas.

Os resultados só são gravados após a igualdade dos IDs, contagens, gravidades e histogramas nas 52 combinações de método e cenário. O arquivo publicado também guarda as 21 medições de consulta de cada combinação.
