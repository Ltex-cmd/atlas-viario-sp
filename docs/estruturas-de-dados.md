# Estruturas de dados e consultas do Atlas Viário

O programa original organiza os sinistros em oito estruturas. O motor do site guarda registros compactos e cria índices para selecionar os candidatos de cada consulta. Depois da seleção, os filtros verificam cada candidato e os registros aceitos entram nas contagens.

Os exemplos usam a amostra de 1.000 sinistros da comparação acima. Os tempos medidos incluem o percurso das estruturas, a aplicação dos filtros e o cálculo das estatísticas.

## Como ler os custos

A notação O descreve um limite para o crescimento do custo quando a entrada aumenta. Ela ajuda a comparar operações, mas não determina o tempo em segundos.

O pior caso considera a entrada mais desfavorável. O custo esperado depende de hipóteses sobre a distribuição das entradas. O custo amortizado distribui o trabalho de uma sequência de operações: uma inserção isolada ainda pode ser cara. Nas tabelas, comparações e acesso aos campos têm custo constante, salvo indicação contrária.

## Notação

- **n**: quantidade total de registros.
- **k**: registros candidatos examinados; pode exceder os resultados aceitos.
- **d**: número de dimensões ou níveis de agrupamento.
- **g**: número de células da grade visitadas; a grade fixa tem 128 × 128 células.
- **v**: número de nós da quadtree visitados.
- **h**: profundidade máxima da quadtree, limitada a 14 no motor.
- **m**: número de posições da tabela hash.
- **G**: quantidade de grupos na lista generalizada, incluindo a raiz.
- **c**: capacidade reservada pelo vetor; pode superar o número de elementos.
- **s**: lado da grade opcional de agrupamentos para o mapa; s = 0 quando ela não é solicitada.
- **b**: número de subgrupos ordenados dentro de um grupo da lista generalizada.

## Pilha

Em uma pilha, o último elemento inserido é o primeiro retirado. Na versão encadeada, cada nó aponta para um sinistro e para o nó que ocupava o topo antes dele. Inserir exige criar um nó e atualizar o topo, sem percorrer os anteriores.

No projeto, a pilha tem operações de inserção, percurso e destruição. Não há operação de retirada. O percurso começa no topo e segue os ponteiros até o primeiro sinistro inserido.

| Operação | Custo | Condição |
|---|---|---|
| Inserir no topo | O(1) | desconsidera o custo variável do alocador |
| Construir e percorrer n registros | O(n) | uma inserção por registro; percurso completo |
| Espaço da estrutura | O(n) | além dos registros compartilhados |

O ensaio insere os registros compartilhados e depois percorre a pilha para filtrá-los. Ao destruir a pilha, libera os nós e preserva os registros.

Na consulta por município, o percurso examina 1.000 registros e o filtro aceita 6.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 15, seção 15.1); (Skiena, 2020, p. 75); (Cormen et al., 2009, p. 233); (Ziviani, 1999, p. 47).

## Fila

Uma fila organiza os elementos pela ordem de chegada: o primeiro inserido é o primeiro a ser retirado. Na versão encadeada do projeto, cada nó aponta para um registro e para o nó seguinte.

A fila guarda ponteiros para o início e para o fim. Com acesso direto ao fim, a inserção liga o novo nó ao anterior sem percorrer a fila. O projeto também oferece um cursor para percorrê-la; não implementa uma operação pública de retirada.

| Operação | Custo | Condição |
|---|---|---|
| Inserir no fim | O(1) | ponteiro fim; custo do alocador à parte |
| Construir e percorrer | O(n) | n inserções e um percurso completo |
| Espaço | O(n) | n nós mais descritor, sem contar registros |

No ensaio, o cursor visita os sinistros na ordem de inserção. A destruição libera os nós e preserva os registros compartilhados.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 15); (Cormen et al., 2009, p. 234); (Ziviani, 1999, p. 55, 58).

## Lista sequencial

A lista sequencial guarda referências aos sinistros em um vetor. Como as posições são contíguas, o programa acessa diretamente qualquer posição válida. O vetor reserva uma capacidade que pode ser maior que a quantidade de registros.

As inserções ocorrem no fim. Quando o vetor enche, a lista tenta dobrar a capacidade, respeitando o limite configurado. Uma expansão pode copiar todas as referências, mas as cópias somadas ao longo de inserções sucessivas crescem linearmente. Por isso, acrescentar custa O(1) amortizado e pode custar O(n) em uma inserção isolada.

| Operação | Custo | Condição |
|---|---|---|
| Acesso por posição | O(1) | posição válida no vetor |
| Acrescentar | O(1) amortizado; O(n) no pior caso | duplicação e eventual realocação |
| Construir / percorrer | O(n) | n acréscimos amortizados / percurso completo |
| Espaço | O(c) | vetor de capacidade c, excluindo registros |

Para consultar a amostra, o ensaio percorre o vetor e aplica o filtro a cada sinistro. Os registros são compartilhados e permanecem alocados depois que a lista é destruída.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 13); (Ziviani, 1999, p. 35).

## Lista encadeada

Na lista simplesmente encadeada, cada nó contém uma referência ao sinistro e um ponteiro para o próximo nó. Para encontrar um registro pela ordem da lista, o programa segue esses ponteiros a partir do início.

O projeto guarda também um ponteiro para o último nó. Assim, uma inserção no fim altera apenas algumas ligações. Já uma busca por valor pode precisar visitar a lista inteira. Os nós ocupam espaço proporcional ao número de registros.

| Operação | Custo | Condição |
|---|---|---|
| Inserir no fim | O(1) | há ponteiro fim; alocador à parte |
| Busca por valor / percurso completo | O(n) | pior caso |
| Construção e espaço | O(n) | um nó por registro, sem incluir o próprio registro |

O ensaio acrescenta os sinistros na ordem da amostra e usa um cursor para percorrê-los. A lista pode ser destruída sem liberar os registros que ela referencia.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 14, seção 14.1); (Roberts, 1998, p. 392); (Skiena, 2020, p. 72).

## Lista com cabeça

Uma lista com cabeça começa com um nó sentinela. Esse nó não guarda um sinistro. Seu ponteiro seguinte indica o primeiro registro; quando a lista está vazia, o ponteiro de fim aponta para a própria sentinela.

A cabeça permite ligar um novo nó depois do último usando a mesma operação, inclusive na primeira inserção. O ponteiro de fim evita procurar esse último nó. Para localizar um sinistro por valor, porém, continua sendo necessário percorrer os nós.

| Operação | Custo | Condição |
|---|---|---|
| Criar / inserir no fim | O(1) | alocador à parte, com ponteiro fim |
| Percorrer ou buscar sem índice | O(n) | pior caso |
| Construção e espaço | O(n) | inclui sentinela de tamanho constante |

O cursor começa no primeiro nó que contém um registro. No ensaio, a sentinela não entra na contagem de candidatos e os registros são preservados na destruição.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 14, seção 14.1); (Thareja, 2014, p. 207); (Ziviani, 1999, p. 38).

## Lista ordenada

A lista ordenada mantém as referências aos sinistros em um vetor, na ordem de um campo escolhido. Antes de inserir, uma busca binária encontra a posição adequada. A lista então desloca as referências seguintes para abrir espaço.

A busca da posição faz O(log n) comparações, mas o deslocamento pode atingir n elementos. Por isso, inserir tem custo O(n) no pior caso. Ao inserir n registros sucessivamente, o pior caso de construção chega a O(n²).

| Operação | Custo | Condição |
|---|---|---|
| Localizar posição | O(log n) | busca binária em vetor ordenado |
| Inserir mantendo ordem | O(n) | pior caso, inclui deslocamentos |
| Construir por inserções | O(n²) | pior caso; ordem de entrada influencia |
| Percurso completo / espaço | O(n) / O(c) | vetor de ponteiros, sem incluir registros |

No ensaio, a lista é ordenada por identificador do sinistro. A consulta percorre o vetor inteiro e aplica os filtros a cada registro.

A consulta por município aceita 6 registros depois de examinar os 1.000 da amostra.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 11, busca e ordenação; cap. 13); (Skiena, 2020, p. 77); (Thareja, 2014, p. 426).

## Lista generalizada

Uma lista generalizada pode conter elementos e sublistas. No projeto, ela forma uma hierarquia de grupos. Cada grupo tem um campo de comparação, um registro representante e um vetor cujas posições apontam para subgrupos ou para sinistros.

Para inserir, o programa busca um subgrupo em cada nível. Se o grupo ainda não existir, cria-o na posição ordenada e desloca os seguintes. Na folha, acrescenta uma referência ao sinistro. Os representantes também são referências; não são cópias dos registros.

O percurso desce recursivamente pelos grupos e retorna os sinistros das folhas. O espaço depende de n registros e de G grupos. Em d níveis, até d grupos podem ser criados por inserção, além da raiz.

| Operação | Custo | Condição |
|---|---|---|
| Buscar subgrupo por nível | O(log b) | b subgrupos ordenados nesse grupo |
| Inserir registro | O(d n) no pior caso | inclui deslocamentos e expansões em d níveis |
| Construir | O(d n²) no pior caso | n inserções; limite superior, distribuição influencia |
| Percorrer | O(d n + G) | caminhos recursivos e G grupos; limite superior |
| Espaço | O(n + G + d), até O(d n + d) | capacidades dobradas, nomes de campos de tamanho fixo; G ≤ d n + 1; exclui registros |

No ensaio, os níveis correspondem a ano, mês e município. O percurso atravessa essa hierarquia e aplica o filtro aos registros encontrados.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 16, árvores e representação de hierarquias); (Hanson, 1997, p. 105).

## Lista cruzada

A lista cruzada do projeto mantém uma ordem para cada campo escolhido. Para isso, usa um vetor de referências por campo. O mesmo sinistro aparece em todos os vetores, e cada vetor possui seu próprio cursor.

A inserção procura por busca binária a posição adequada em cada vetor. Depois desloca as referências seguintes para conservar cada ordem. Com d campos, esse deslocamento pode envolver até d vezes o número de registros. Na destruição que também libera os sinistros, o programa usa apenas o primeiro vetor para liberar cada um uma vez.

| Operação | Custo | Condição |
|---|---|---|
| Localizar posições | O(d log n) | uma busca binária em cada dimensão |
| Inserir | O(d n) | pior caso por deslocamentos |
| Construir | O(d n²) | pior caso para inserções sucessivas |
| Percorrer uma dimensão / espaço | O(d n) / O(d c) | procura o nome do campo a cada chamada; d fixo reduz percurso a O(n) |

No ensaio, um vetor é ordenado pelo código do município e outro pelo identificador. A consulta percorre o vetor municipal e aplica o filtro a cada sinistro.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 11, busca e ordenação); (Thareja, 2014, p. 210).

## Vetor compacto de registros

O motor do site reúne os dados de cada sinistro em um registro compacto. Cada registro contém coordenadas, identificador, código municipal, data, quantidades e categorias codificadas. Os registros ficam lado a lado em um vetor; cada um ocupa 48 bytes na representação usada pelo motor.

Uma posição do vetor pode ser acessada diretamente. Na varredura, o motor visita as posições em sequência, aplica o filtro e atualiza as contagens dos registros aceitos. O tamanho dos registros e sua disposição na memória influenciam o tempo, assim como o trabalho de cada filtro.

| Operação | Custo | Condição |
|---|---|---|
| Acesso por índice | O(1) | posição válida |
| Varredura e agregação | O(n + s²) | filtros e contadores de custo constante; inclui zerar os agrupamentos opcionais |
| Espaço dos registros | O(n) | 48 bytes por registro nesta representação |

Os índices do motor guardam posições desse vetor. O vetor pertence ao chamador: ao destruir o motor, o programa libera os índices, mas preserva os registros.

A varredura examina os 1.000 registros em cada cenário do ensaio.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 13).

## Índices ordenados e busca binária

Um índice ordenado associa cada chave à posição de um sinistro no vetor compacto. O motor cria três índices, um por data, um por município e outro por identificador. A busca binária encontra os limites de uma sequência de chaves sem percorrer o índice inteiro.

Para consultar, o motor calcula o intervalo de datas e, se houver município, o intervalo desse município. Ele escolhe o intervalo menor e aplica todos os filtros aos candidatos. O índice de identificador atende a uma busca separada.

A construção usa a função qsort da biblioteca C. O custo dessa função depende de sua implementação; o padrão da linguagem não garante um limite O(n log n) para ela.

| Operação | Custo | Condição |
|---|---|---|
| Localizar limites / ID | O(log n) | busca binária |
| Consulta e agregação | O(log n + k + s²) | filtros de custo constante; k candidatos e s² agrupamentos opcionais a zerar |
| Construção | 3 chamadas a qsort mais O(n) | complexidade de qsort depende da implementação C |
| Espaço | O(n) | três vetores de entradas, além dos registros |

A seleção de registros para a lista de resultados usa o intervalo de datas e depois aplica os demais filtros. Nessa operação, o motor não escolhe o intervalo municipal.

No ensaio, o índice ordenado examina 331 candidatos no período e 6 no município. Na consulta por área, examina os 1.000.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 11, busca e ordenação); (Roberts, 1998, p. 183); (Thareja, 2014, p. 426).

## Tabela hash de ID

Uma tabela hash usa o identificador para calcular uma posição inicial. Quando ela está ocupada por outro sinistro, o motor examina as posições seguintes até encontrar o identificador procurado ou uma posição vazia. Essa técnica é chamada sondagem linear.

Cada posição ocupada guarda o índice do registro somado a um. O valor zero indica posição vazia. Na construção, a capacidade é uma potência de dois pelo menos duas vezes maior que o número de registros. Isso limita a ocupação inicial à metade, mas não impede sequências longas de colisões. O custo esperado depende da distribuição das chaves; no pior caso, a busca percorre uma quantidade linear de posições.

| Operação | Custo | Condição |
|---|---|---|
| Buscar ou inserir ID | O(1) esperado; O(n) pior caso | esperado depende da distribuição e carga; sondagem linear |
| Construir tabela | O(n) esperado; O(n²) pior caso | n inserções; capacidade m proporcional a n |
| Espaço | O(m) | m potência de 2, ao menos 2n para n > 0 |

O motor usa essa tabela na busca por identificador. As quatro consultas agregadas do ensaio não chamam essa busca, de modo que seus tempos não comparam diretamente a tabela hash com o índice ordenado de identificadores.

Referências: (Skiena, 2020, p. 94); (Cormen et al., 2009, p. 256); (Ziviani, 1999, p. 140).

## Grade espacial

Uma grade espacial divide a área do mapa em células e reúne as referências aos sinistros de cada célula. O motor usa uma grade fixa de 128 por 128. Na construção, conta os registros de cada célula, calcula as posições iniciais e distribui as referências em um vetor.

Para consultar uma área, o motor percorre as células que cruzam o retângulo pedido. Em seguida, verifica as coordenadas de cada candidato. Uma célula na borda pode conter sinistros fora do retângulo, por isso a verificação ainda é necessária.

| Operação | Custo | Condição |
|---|---|---|
| Construção | O(n + 128²) | contagem, prefixos e distribuição |
| Consulta espacial | O(log n + g + k + s²) | buscas iniciais nos índices, g células, k candidatos e limpeza dos agrupamentos de saída opcionais; filtros constantes |
| Espaço | O(n + 128²) | referências e posições iniciais; área temporária de construção separada |

A grade recebe os registros com coordenadas dentro do recorte adotado. Sem filtro geográfico, a consulta usa o índice ordenado. Antes de escolher o caminho espacial, o motor calcula os limites dos índices de data e, quando há filtro municipal, de município.

Na área da amostra, a grade examina 7 candidatos e o filtro aceita 5. Na consulta por período, sem filtro geográfico, usa o índice ordenado e examina 331.

Referências: (Nievergelt; Hinterberger; Sevcik, 1984, p. 38, introdução).

## Quadtree

Uma quadtree por região divide o espaço em quatro partes e repete a divisão nas regiões que precisam de mais detalhe. O Atlas usa uma variante com baldes: cada folha reúne referências aos sinistros de sua região.

O motor divide um retângulo nos pontos médios dos eixos e distribui as referências entre os quatro quadrantes. Ele subdivide regiões com mais de 128 registros até a profundidade 14. Ao atingir esse limite, uma folha pode guardar mais de 128 registros.

Na consulta por área, o motor descarta nós cujos retângulos não cruzam a região pedida. Nas folhas restantes, verifica as coordenadas dos candidatos. A distribuição dos pontos e o tamanho da área afetam quantos nós e registros serão examinados.

| Operação | Custo | Condição |
|---|---|---|
| Construção | O(n h + número de nós) | particionamento por nível e expansão geométrica do vetor de nós; h ≤ 14 |
| Consulta espacial | O(log n + v + k + s²) | buscas iniciais nos índices, v nós, k candidatos e limpeza dos agrupamentos de saída opcionais; filtros constantes |
| Espaço | O(n + número de nós) | vetor espacial e capacidade dos nós |

Antes do percurso espacial, o motor calcula os limites dos índices ordenados de data e, quando há filtro municipal, de município. Sem filtro geográfico, usa o índice ordenado.

Na área da amostra, a quadtree examina 93 candidatos e o filtro aceita 5. A grade examina 7 candidatos para a mesma consulta.

Referências: (Samet, 1984, seção 3.2, p. 235-236; p. 187).

## Vetores de contadores e agrupamentos

Um vetor de contadores guarda, em cada posição, a quantidade de registros de uma categoria. Quando um sinistro passa pelo filtro, o motor incrementa os contadores do mês, do tipo e do município e soma as quantidades de vítimas por gravidade.

Os vetores de estatísticas têm 12 posições para meses, 8 para tipos e 1.024 para municípios. Quando a consulta solicita agrupamentos para o mapa, outro vetor guarda a contagem e a soma das coordenadas em cada célula de saída. Antes de usá-lo, o motor zera suas posições.

| Operação | Custo | Condição |
|---|---|---|
| Atualizar contadores por registro aceito | O(1) | quantidade fixa de campos e acesso por índice |
| Consulta e agregação | O(k + s²) | além da seleção de candidatos, que inclui buscas ordenadas iniciais; zera s² agrupamentos de saída se fornecidos |
| Espaço | O(1 + s²) | contadores fixos; s² agrupamentos para o mapa, quando solicitados |

O ensaio compara os totais, as gravidades e os histogramas. Ele não solicita os agrupamentos por célula do mapa.

Referências: (Celes; Cerqueira; Rangel, 2016, cap. 13); (Hanson, 1997, p. 121).

## Planejador automático

O planejador é a regra que escolhe como percorrer os candidatos. Ele calcula o intervalo de datas e, quando há filtro municipal, o intervalo do município. Se houver filtro geográfico, estima quantos registros estão nas regiões da quadtree que cruzam a área pedida.

Quando a estimativa é menor que o tamanho do intervalo ordenado, o planejador escolhe a quadtree. Caso contrário, usa o índice ordenado. A estimativa também tem custo: ela visita nós da árvore e pode parar quando alcança o limite usado na comparação.

A regra compara quantidades estimadas, sem medir o tempo das alternativas durante a consulta. A grade não participa dessa escolha.

| Operação | Custo | Condição |
|---|---|---|
| Escolher caminho | O(log n + vₑ) | vₑ nós visitados na estimativa, apenas quando há filtro espacial |
| Executar consulta | custo da escolha + custo do caminho escolhido | índice ordenado ou quadtree; inclui zerar s² agrupamentos de saída opcionais |
| Espaço adicional persistente | O(1) | política usa índices existentes; pilha recursiva depende da profundidade |

O planejador usa os índices já construídos pelo motor. Sua escolha determina o percurso, e os filtros ainda verificam os candidatos retornados.

Na consulta por área da amostra, o planejador escolhe a quadtree e examina 93 candidatos para 5 resultados.

## Referências

CELES, Waldemar; CERQUEIRA, Renato; RANGEL, José Lucas. **Introdução a estruturas de dados**: com técnicas de programação em C. 2. ed. Rio de Janeiro: Elsevier, 2016.

CORMEN, Thomas H.; LEISERSON, Charles E.; RIVEST, Ronald L.; STEIN, Clifford. **Introduction to algorithms**. 3. ed. Cambridge, Massachusetts: MIT Press, 2009.

HANSON, David R. **C interfaces and implementations**: techniques for creating reusable software. Reading, Massachusetts: Addison-Wesley, 1997.

NIEVERGELT, Jürg; HINTERBERGER, Hans; SEVCIK, Kenneth C. The grid file: an adaptable, symmetric multikey file structure. **ACM Transactions on Database Systems**, v. 9, n. 1, p. 38-71, mar. 1984. DOI: https://doi.org/10.1145/348.318586. Disponível em: https://dsl.cds.iisc.ac.in/~course/TIDS/papers/gridfile.pdf. Acesso em: 2 nov. 2026.

ROBERTS, Eric S. **Programming abstractions in C**: a second course in computer science. Reading, Massachusetts: Addison-Wesley, 1998.

SAMET, Hanan. The quadtree and related hierarchical data structures. **ACM Computing Surveys**, v. 16, n. 2, p. 187-260, jun. 1984. DOI: https://doi.org/10.1145/356924.356930. Disponível em: https://www.cs.umd.edu/~hjs/pubs/SameCSUR84.pdf. Acesso em: 2 nov. 2026.

SKIENA, Steven S. **The algorithm design manual**. 3. ed. Cham: Springer, 2020.

THAREJA, Reema. **Data structures using C**. 2. ed. New Delhi: Oxford University Press, 2014.

ZIVIANI, Nivio. **Projeto de algoritmos**: com implementações em Pascal e C. 4. ed. São Paulo: Pioneira, 1999.
