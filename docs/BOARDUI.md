# BoardUI no Atlas Viário

A interface usa código dos componentes gratuitos do BoardUI, instalado com a CLI `boardui@0.5.9` em 2 de novembro de 2026. Os arquivos pertencem ao projeto e não exigem conta, chave ou acesso ao BoardUI durante a execução do site.

O inventário em [boardui-free.json](boardui-free.json) registra os 37 itens resolvidos, incluindo suas dependências internas. Todos foram conferidos no registro público gratuito, `https://www.boardui.com/r/registry.json`. A licença MIT original está em [vendor/boardui.LICENSE](../vendor/boardui.LICENSE). Nenhum componente ou template Pro foi instalado.

## Componentes usados

| Componente gratuito | Uso no Atlas |
| --- | --- |
| Button | Aplicar e limpar filtros, exportar, buscar ID e comparar métodos |
| Segmented Control | Seções do site e modos de exibição do mapa |
| Drawer | Painel de filtros com rolagem interna, ações fixas e gerenciamento de foco |
| ComboBox | Busca de municípios com sugestões, seleção por teclado e filtro de texto |
| Select | Ano, tipo de sinistro, transporte, via e classificação |
| Date Picker | Datas inicial e final com calendário em português |
| Input | Busca de sinistro por ID |
| Stat Cards e Skeleton | Indicadores reais da consulta e carregamento |
| Table | Tabelas acessíveis de comparação, mantendo os cartões existentes em telas pequenas |
| Tag Group | Filtros ativos removíveis por clique, Backspace ou Delete |
| Data Table, Checkbox e Pagination | Lista de sinistros com ordenação, seleção, exportação e paginação |
| Orders Chart Card | Evolução mensal e comparação com intervalo equivalente do ano anterior |
| Toast e Notification | Confirmações, mensagens de erro e ações de recuperação |

O mapa MapLibre, suas camadas, a API e o motor de consultas continuam com suas implementações existentes. O gráfico mensal usa o Orders Chart Card gratuito, adaptado aos dados do Infosiga. Os gráficos e templates Pro do BoardUI não são necessários.

## Integração e ajustes

`boardui.json` direciona os arquivos para `components/boardui`, `styles/boardui` e `utils/boardui`, sem sobrescrever os componentes antigos em `components/ui`. As duas entradas, `web/main.tsx` e `app/layout.tsx`, importam `styles/atlas.css`.

`styles/atlas.css` coloca o CSS existente em uma camada anterior às utilidades do BoardUI. A integração não carrega o reset global do Tailwind, para preservar o mapa e os demais elementos do Atlas. A geração das utilidades considera apenas os arquivos BoardUI e sua integração. `styles/atlas-boardui.css` aplica o verde original, as superfícies existentes e o comportamento dos popovers; a fonte continua Manrope.

Os componentes copiados receberam estes ajustes:

- O botão primário usa superfícies verdes planas, com estados de hover, pressão e desabilitado.
- O calendário aceita os limites reais de cobertura, tem nomes acessíveis e ações em português, respeita movimento reduzido e rejeita datas impossíveis digitadas.
- Os popovers ficam acima do drawer e permitem confirmar uma seleção sem fechar o painel de filtros.
- Os indicadores aceitam unidades e descrições sem exigir comparações ou variações fictícias.
- As tabelas do calendário não recebem o espaçamento das tabelas antigas do Atlas.

Para atualizar os componentes, preserve esses ajustes e confira o inventário gratuito antes de instalar novos itens. O instalador não é uma dependência de execução.

## Executar uma prévia isolada

Use portas diferentes das de outros checkouts:

```sh
npm ci --include=optional
npm run dev -- --port 5187
```

Em outro terminal:

```sh
ATLAS_DEV_API_URL=http://127.0.0.1:5187 npm run dev:pages -- --port 5186 --strictPort
```

Abra `http://127.0.0.1:5186/atlas-viario-sp/`. A API publicada aceita o domínio do GitHub Pages; a prévia deve usar a API local.

```sh
npm test
npm run test:ui
npm run typecheck
VITE_ATLAS_API_URL=https://atlas-viario-api-lucas.flowy-swift-7745.chatgpt.site npm run build:pages
```

## Validação no navegador

Testes de UI feitos por um agente Terra 6.1 com raciocínio high em 1280 × 720 e 375 × 812, com a API local e os dados reais:

- Navegação entre Mapa, Desempenho e Sobre os dados.
- Drawer com campos roláveis, ações visíveis, Escape e retorno de foco ao botão de abertura.
- Seleção de ano e categorias e autocomplete de São Carlos por Enter, mantendo o drawer aberto.
- Calendário sem cortes, confirmação sem fechar o drawer, validação de intervalo invertido e limpeza dos filtros.
- São Carlos, data inicial 17/09/2026 e Colisão retornaram 23 registros, dos quais 21 localizados; a lista exibiu 21 e abriu o detalhe do registro 2852387.
- Exportação de `atlas-viario-resumo.json` com confirmação visível.
- Percurso de exemplo concluído, com 1,8 km, 24 minutos e um sinistro nos filtros testados.
- Troca de cenário e execução da comparação interativa, com resultados coincidentes nos cinco métodos.

Não houve teste em dispositivo físico. O navegador do subagente não disponibiliza janela visível; a inspeção usou a automação e as capturas do navegador.

A versão integrada também foi conferida no navegador da thread principal: a tabela da amostra contém 13 métodos; trocar de Amostra inteira para Área de São Carlos altera a contagem de 1.000 para cinco sinistros. Em tela estreita, o documento não tem overflow horizontal e somente a tabela oferece rolagem lateral. A tabela BoardUI de estruturas renderiza suas seis linhas. A suíte automatizada terminou com 37 testes aprovados, além da verificação de renderização dos indicadores, checagem de tipos e build do frontend.

## Segunda etapa: exploração dos resultados

- As tags removem um filtro de cada vez ou todos, sincronizando a consulta e o formulário. Remover todos inclui o ano, consultando a cobertura integral; a ação Limpar seleção do formulário continua retornando ao ano mais recente.
- A tabela de sinistros usa os dados reais da página de até 30 registros. Ordenação e seleção são locais a essa página, com aviso visível; a paginação permanece cronológica no servidor. Abrir um detalhe e voltar preserva seleção e ordenação. Trocar de página reinicia a seleção.
- Exportar seleção baixa `atlas-viario-selecao.json` com os registros escolhidos, filtros, legenda e escopo explícito. Exportar no cabeçalho mantém o resumo completo da consulta.
- O drawer de registros e detalhes usa gerenciamento de foco do React Aria. No celular a tabela rola horizontalmente dentro do painel.
- O gráfico mensal é carregado em um módulo separado. A comparação faz uma consulta adicional somente quando solicitada, com os mesmos filtros de município e categorias e as mesmas datas no ano anterior. Requisições antigas são canceladas. A tabela de valores mensais oferece uma alternativa ao gráfico para teclado e leitura de tela.
- Meses fora do período aparecem sem cobertura, nunca como zero. A comparação não é oferecida para intervalos que abrangem mais de um ano, para anos sem antecedente disponível, nem entre 2019 e 2018, cuja cobertura difere. O dia 29 de fevereiro corresponde a 28 de fevereiro no ano anterior, e as datas efetivas são exibidas.
- O exemplo de clientes e os dados fictícios dos blocos BoardUI foram substituídos por propriedades obrigatórias e dados do Atlas. Os avatares de demonstração não são distribuídos.

### Verificação da segunda etapa

- Terra 6.1 high testou em Safari desktop: tags individuais e remoção completa, sincronização com formulário, ordenação por município/data/vítimas fatais, seleção, exportação, paginação, abrir detalhe, voltar e localizar no mapa.
- A comparação inicial exibiu 126.272 eventos em janeiro a setembro de 2026 e 140.916 no mesmo intervalo de 2025, com variação de -10,4%. Outubro a dezembro permaneceram sem cobertura nas duas séries.
- O teste encontrou uma perda de estado ao voltar dos detalhes. Após a correção, seleção e ordenação permaneceram na mesma página.
- 41 testes de lógica passaram, incluindo limites de cobertura, período parcial e data bissexta. A verificação de renderização mantém distintos dados pendentes, falhas e resultados realmente vazios.
- A thread principal complementou o teste em 375 × 812 e 1280 × 800: gráfico e detalhes legíveis, paginação móvel 1–30 → 31–60 e tabela com rolagem horizontal contida. O documento permaneceu sem overflow horizontal. Escape devolveu o foco a Ver lista da área.
- O arquivo baixado pelo Safari foi inspecionado: `atlas-selected-events-v1`, um registro, ID 2728084, correspondente à seleção testada.
- Capturas locais estão em `outputs/boardui-qa-v2/`. O gráfico é carregado separadamente; o build ainda avisa sobre o tamanho dos módulos principais e do MapLibre.
- O percurso de exemplo em São Carlos foi concluído na thread principal: 1,8 km, 24 minutos e três sinistros para janeiro a setembro de 2026, em faixa de 50 m. O console do navegador não registrou erros durante a conferência final.

### Correção da mensagem de rotas

O OSRM pode responder HTTP 400 com `NoRoute` ou `NoSegment`. O adaptador tratava qualquer status HTTP de erro como indisponibilidade, antes de ler o JSON. Agora esses dois casos são apresentados como seleção sem caminho ou sem via próxima, com HTTP 422; limites de uso e falhas do serviço continuam distintos. A UI oferece ações para reposicionar a origem ou o destino.

Referência do contrato: https://project-osrm.org/docs/v5.24.0/api/#general-options

Verificação: a seleção que originou o relato exibiu a mensagem de caminho a pé não encontrado; Ajustar destino ativou a escolha no mapa e Escape a cancelou sem mudar os pontos. O exemplo em São Carlos continuou retornando HTTP 200, 1.812 m, 24 minutos e três sinistros. Foram aprovados 46 testes, checagem de tipos e builds do servidor e do frontend.


### Correção do cálculo para pontos fora da rede conectada

O diagnóstico com as coordenadas exibidas no relato (`A: -21.71571, -48.09051`; `B: -21.79387, -47.90249`, latitude/longitude, a pé) reproduziu `NoRoute` com `radiuses=500;500`. A consulta de vias próximas encontrou A a 318,98 m e B a 407,90 m, mas o roteador só retornou um caminho conectado ao ampliar o raio: B foi associado a uma via a 1.054,16 m. Com `radiuses=2000;2000`, o mesmo serviço e modo retornaram 38.281,7 m, 32.662,6 s e 757 vértices. Isso indica que o limite fixo de 500 m bloqueava uma alternativa na rede disponível; não comprova acesso físico entre o clique e a via escolhida.

O adaptador agora tenta 500 m primeiro e, somente após `NoRoute` ou `NoSegment`, faz uma tentativa limitada a 2.000 m. Erros de disponibilidade e limite de consultas não disparam essa segunda tentativa. As chamadas continuam no mesmo provedor, com os mesmos pontos, modo e espaçamento mínimo de 1,1 segundo.

A UI desenha a alternativa e pede revisão se algum ponto ficar a mais de 500 m da via. Os marcadores A/B permanecem nas coordenadas escolhidas. As linhas pontilhadas indicam apenas o deslocamento; não viram geometria de rota nem entram nas medidas ou na contagem. A consulta de sinistros começa depois de **Usar este traçado**. Origem e destino também aceitam coordenadas no formato `latitude, longitude`, permitindo repetir a consulta sem depender da precisão de um clique.

Verificação em 02/11/2026:

- A prévia local encontrou a proposta de 38,3 km / 9 h 4 min, com origem a 319 m e destino a 1,1 km. Antes da aceitação não apareceu contagem de sinistros.
- Após aceitar: 11 sinistros em faixa de 50 m e 13 em faixa de 100 m, com período de 01/01 a 30/09/2026. A lista exibiu os 11 registros da primeira consulta.
- Alterar o modo removeu o traçado e a contagem anterior. Ajustar destino ativou a seleção no mapa; Escape cancelou sem mudar os pontos.
- O exemplo São Carlos continuou funcionando sem confirmação extra: 1,8 km, 24 min, três sinistros na faixa de 50 m.
- 54 testes, checagem de tipos, verificação de renderização e builds do servidor/frontend aprovados. Permanecem os avisos de tamanho dos módulos do frontend.
- Terra 6.1 high revisou a lógica da proposta e da invalidação. A validação interativa acima foi feita pela thread principal no IAB, porque o IAB do subagente não estava disponível e o Safari estava em uso.
- Evidências locais: `outputs/route-diagnosis/` e `outputs/boardui-qa-v2/route-proposal.jpg`.

Referências: [opções de associação a vias do OSRM](https://project-osrm.org/docs/v5.24.0/api/#general-options) e [condições do servidor FOSSGIS](https://routing.openstreetmap.de/about.html).


### Faixa de proximidade sem artefatos de sobreposição

A camada `journey-band` usava uma linha larga com opacidade. As faces sobrepostas nas curvas criavam triângulos e áreas mais escuras; o problema foi reproduzido no exemplo de São Carlos, com proximidade de 100 m.

A faixa agora é uma única área de buffer com as interseções unidas, preenchida pelo MapLibre. A geometria usa a mesma projeção local e o mesmo raio da seleção C/Wasm. As pontas e curvas são arredondadas, com aproximação máxima de 3,1 cm no raio de 100 m. Buracos fora da faixa permanecem vazios. A rota enviada à contagem não foi alterada, e os acessos pontilhados continuam excluídos.

O cálculo usa JSTS 2.12.1 (licença EDL-1.0 ou EPL-1.0) em um Web Worker separado, carregado apenas ao desenhar a faixa. Trocar rota/raio ou sair do painel encerra o cálculo anterior; respostas antigas não substituem a seleção atual. Terra 6.1 high identificou o risco de bloqueio na primeira implementação síncrona, e revisou a migração e o cancelamento do worker. O worker gerado ocupa aproximadamente 230 kB sem compressão, fora do módulo principal.

Verificação em 02/11/2026:

- 58 testes aprovados, incluindo comparação da área desenhada com o C/Wasm real em cruzamentos, laços e curvas em U, com 441 pontos por cenário e raios de 25, 50 e 100 m.
- Testes de ida/volta sobre o mesmo segmento, vértices repetidos, extremidades arredondadas, buracos e preservação da rota original.
- Na interface, o exemplo manteve três sinistros nos três limites. Trocas rápidas de limite e zoom foram verificadas; as manchas triangulares de 100 m desapareceram.
- Checagem de tipos, teste de renderização e builds aprovados. Capturas comparáveis: `outputs/boardui-qa-v2/corridor-before.jpg` e `corridor-after.jpg`.

Referência da operação geométrica: [JSTS](https://github.com/bjornharrtell/jsts).

### Integração com a principal

O merge `327910a` reúne o BoardUI (`768e996`) e a principal (`9af03d4`). Foram preservados o guia de 15 estruturas, as referências, os textos revisados sobre cobertura e os arquivos do motor, dos dados e dos benchmarks. Os conflitos em Atlas, StatisticsPanel e EventDetail mantêm os controles BoardUI e as explicações da principal. Terra 6.1 high revisou essa preservação sem alterar o checkout do outro agente.

Verificação em 02/11/2026:

- 58 testes, checagem de tipos, teste de renderização e builds do frontend/API aprovados; os dois testes do benchmark em C passaram com sanitizadores. O workflow do merge também concluiu build e publicação no GitHub Pages.
- No navegador: filtros municipais, tabela de 8.004 sinistros de São Carlos, abertura de detalhe e retorno com seleção preservada. O guia abre Quadtree (13 de 15); os cinco métodos da consulta atual retornam 34.524 registros e conferências coincidentes.
- A API publicada devolve os mesmos metadados, estatísticas, registros e detalhes da versão local. Os pontos do relato retornam uma proposta de rota e, após aceitação, 13 sinistros na faixa de 100 m. A API também retorna 11 na faixa de 50 m.
- A inspeção do build publicado encontrou o botão de percurso com fundo transparente. A importação remota de Manrope, ao ser movida para o início do CSS minificado dentro de `layer(atlas)`, registrava essa camada antes da base do BoardUI. A fonte agora é importada fora da camada; o build compilado voltou a exibir o fundo verde `#2d7164`. A hierarquia e o conteúdo do guia permanecem intactos.
