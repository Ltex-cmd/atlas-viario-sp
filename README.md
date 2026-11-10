# Atlas Viário  São Paulo

Mapa de sinistros de trânsito do estado de São Paulo, com dados reais do Infosiga / Detran-SP e consultas implementadas em C. Projeto acadêmico independente para demonstração de algoritmos e estruturas de dados na USP São Carlos.

**Site:** https://ltex-cmd.github.io/atlas-viario-sp/

## O que está disponível

Filtros por município, ano, intervalo de datas, tipo de sinistro, transporte, via e classificação. O mapa permite explorar agrupamentos, pontos originais e densidade, consultar listas paginadas e os 50 atributos da fonte. A seleção de município enquadra sua região automaticamente.

O painel de percursos busca lugares ou recebe origem e destino diretamente no mapa. Calcula caminhos a pé, de bicicleta ou de carro e conta sinistros registrados a até 25, 50 ou 100 metros da geometria retornada pelo roteador. A contagem preserva ocorrências distintas na mesma coordenada e não multiplica uma ocorrência quando o percurso passa por ela novamente. Compara também a contagem com a média municipal por quilômetro de malha viária, no mesmo período e com os mesmos filtros, usando a cauda exata de Poisson e os graus de confiança do DNIT. A contagem e a comparação não são uma medida de risco ou uma classificação de caminho seguro.

O painel permite chutar a contagem antes de revelar, com preferência local, mostra os modos envolvidos e compara até quatro caminhos no duelo, com contagem por quilômetro. Quando a API não fornece o veredito, o navegador consulta as referências municipais com os mesmos filtros, para rotas de até seis municípios. As barras de transportes informam quando usam apenas os primeiros registros disponíveis.


## Arquitetura da publicação

O GitHub Pages hospeda a interface estática compilada com React, TypeScript, Vite e MapLibre. O motor C continua no servidor, compilado em WebAssembly na API hospedada no Sites. O navegador recebe somente resultados filtrados, pontos ou agrupamentos, sem baixar os 1,45 milhão de registros.

API da publicação: https://atlas-viario-api-lucas.flowy-swift-7745.chatgpt.site

- `web/`: entrada do frontend estático.
- `components/`, `app/globals.css`: interface compartilhada.
- `worker/`: transporte HTTP, validação, API de rotas e motor C/Wasm.
- `worker/municipal-polygons.json`, `worker/road-network.json`: polígonos IBGE e referência de extensão da malha OSM por município.
- `engine-source/`: fonte C exata do módulo compilado.
- `public/data/`: partições e detalhes consumidos exclusivamente pela API. Não entram no pacote do GitHub Pages.
- `tests/`: regressões de seleção, filtros, agrupamentos, projeção do percurso e acesso entre os domínios.
- `.github/workflows/pages.yml`: testes, compilação e publicação automática do frontend em cada alteração de `main`.
- `.openai/hosting.json`: identidade da API no Sites, sem credenciais.

O cliente usa `VITE_ATLAS_API_URL` no momento da compilação. Esse endereço é público e não contém segredo. Mudar o frontend publica pelo GitHub Actions; mudanças da API exigem publicar também sua versão no Sites. A publicação original do Atlas no Sites permanece independente.

## Executar localmente

Node 24 e npm.

```sh
npm ci
npm run dev
```

Em outro terminal:

```sh
npm run dev:pages
```

Abra http://127.0.0.1:5174/atlas-viario-sp/. O frontend encaminha as consultas para a API local na porta 5173. Os arquivos preparados estão neste repositório; não é preciso executar o projeto universitário nem alterar seus CSVs.

Para compilar o frontend usando a API publicada:

```sh
VITE_ATLAS_API_URL=https://atlas-viario-api-lucas.flowy-swift-7745.chatgpt.site npm run build:pages
```

O resultado está em `dist-pages/`. Para compilar a API usando a configuração Sites, use `npm run build` e o fluxo de publicação do Sites. Para recompilar o módulo C/Wasm, use o WASI SDK e os símbolos exportados documentados em `engine-source/README.md`.

## Verificar

```sh
npm run typecheck
npm test
```

O painel Desempenho compara os métodos sobre a mesma consulta. Tempos nativos gravados são cenários de referência, separados da consulta interativa. Não são apresentados tempos de CPU fictícios do Worker.

## Dados e limites

A base contém 1.450.950 eventos, com 50 colunas, entre 2015 e setembro de 2026. Antes de 2019 a cobertura inclui apenas fatais; 2026 é parcial. Eventos e vítimas são contagens diferentes. Registros sem coordenadas aceitas permanecem nas estatísticas, sem receber localização inventada. Coordenadas aproximadas da fonte limitam a interpretação de ocorrências em uma rua específica.

O mapa exibe até 12.000 pontos individuais antes de solicitar aproximação; não transforma agrupamentos em pontos fictícios. O percurso aceita até 100 km e 10.000 vértices. Sua contagem abrange todos os eventos correspondentes; o mapa do percurso mostra até 2.000 pontos, e os demais continuam disponíveis na lista paginada. A distância ao percurso usa projeção local aproximada.

A representação compacta e os detalhes preservam a precisão e os valores da fonte. `public/data/metadata.json` registra versão, proveniência, codificação, cobertura e SHA-256 do CSV usado. O repositório original da disciplina não foi alterado; esta publicação é um export independente do visualizador complementar.

## Serviços e atribuição

Mapa CARTO/OpenStreetMap, enquadramento municipal a partir da malha simplificada do IBGE, rotas OSRM/FOSSGIS e busca Photon. Buscas e pontos do percurso são enviados aos respectivos operadores. Esses serviços públicos têm limites de uso e dependem de sua disponibilidade. A aplicação não cria contas nem mantém um histórico de endereços. As respostas da API ficam em cache temporário de memória.

- Dados: https://infosiga.detran.sp.gov.br/mapas/
- Mapa: https://www.openstreetmap.org/copyright
- Rotas e termos: https://routing.openstreetmap.de/about.html
- Busca: https://github.com/komoot/photon
- Limites de hospedagem estática: https://docs.github.com/en/pages/getting-started-with-github-pages/about-github-pages
