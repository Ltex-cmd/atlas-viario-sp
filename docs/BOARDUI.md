# BoardUI no Atlas Viário

A interface usa código dos componentes gratuitos do BoardUI, instalado com a CLI `boardui@0.5.9` em 2 de novembro de 2026. Os arquivos pertencem ao projeto e não exigem conta, chave ou acesso ao BoardUI durante a execução do site.

O inventário em [boardui-free.json](boardui-free.json) registra os 26 itens resolvidos, incluindo suas dependências internas. Todos foram conferidos no registro público gratuito, `https://www.boardui.com/r/registry.json`. A licença MIT original está em [vendor/boardui.LICENSE](../vendor/boardui.LICENSE). Nenhum componente ou template Pro foi instalado.

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

O mapa MapLibre, suas camadas, os gráficos estatísticos próprios, a API e o motor de consultas continuam com suas implementações existentes. Os gráficos e templates Pro do BoardUI não são necessários.

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
