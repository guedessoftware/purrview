# Impage — Guia de Desenvolvimento e Convenções para IA

**Documento:** Development Guide
**Versão:** 0.1
**Status:** Base inicial do projeto
**Plataforma-alvo:** Linux
**Stack principal:** C++20 + Qt 6 + QML + CMake
**Aplicação:** composição visual de imagens em páginas para impressão

---

## 1. Objetivo deste documento

Este documento define as regras de desenvolvimento do **Impage** e deve servir como referência para:

- desenvolvimento manual;
- desenvolvimento assistido por IA / vibe coding;
- revisão de código;
- refatorações;
- testes;
- decisões de arquitetura;
- inclusão de novas funcionalidades.

A regra central é simples:

> **A IA pode acelerar a implementação, mas não pode alterar silenciosamente a arquitetura do projeto.**

Qualquer mudança estrutural relevante deve ser proposta e justificada antes de ser implementada.

---

## 2. Visão do produto

O Impage é uma aplicação Linux focada em tornar a impressão de imagens simples, visual e previsível.

O usuário deve conseguir:

1. escolher uma ou várias imagens;
2. escolher o tamanho da página;
3. definir linhas e colunas;
4. visualizar imediatamente a grade;
5. ajustar margens e espaçamentos;
6. definir como cada imagem ocupa sua célula;
7. visualizar exatamente o resultado esperado;
8. imprimir.

A aplicação deve evitar a complexidade tradicional de ferramentas gráficas e de impressão.

### Princípio de UX

> **O que o usuário vê na pré-visualização deve corresponder ao que será impresso.**

---

## 3. Princípios de engenharia

### 3.1 Simplicidade primeiro

Não adicionar abstrações, dependências ou padrões apenas porque são tecnicamente interessantes.

Uma solução simples, legível e testável é preferível a uma solução sofisticada sem necessidade real.

### 3.2 Separação entre interface e lógica

QML não deve conter regras de negócio complexas.

QML é responsável principalmente por:

- apresentação;
- interação;
- estados visuais;
- animações;
- encaminhamento de ações do usuário.

C++ é responsável por:

- modelo do documento;
- cálculo de layout;
- manipulação de imagem;
- geração da página;
- impressão;
- persistência;
- validação;
- integração com o sistema.

### 3.3 Uma única fonte de verdade para o layout

A pré-visualização e a impressão devem utilizar o **mesmo modelo geométrico**.

Não deve existir:

- um cálculo para preview;
- outro cálculo independente para impressão.

Isso evita divergências entre tela e papel.

### 3.4 Dependências mínimas

Usar preferencialmente os recursos do próprio Qt.

Bibliotecas externas só devem ser adicionadas quando:

- o Qt não oferece a funcionalidade necessária;
- a implementação própria seria significativamente mais complexa;
- houver benefício técnico comprovável.

Toda nova dependência deve ser justificada.

### 3.5 Linux em primeiro lugar

O projeto não precisa comprometer sua arquitetura para atender Windows ou macOS.

Compatibilidade futura pode ser considerada, mas não deve prejudicar a integração e a experiência no Linux.

---

# 4. Stack oficial

## Linguagem

- **C++20**

Não utilizar extensões proprietárias do compilador sem necessidade.

## Framework

- **Qt 6**

Módulos esperados inicialmente:

- Qt Core
- Qt Gui
- Qt Quick
- Qt QML
- Qt Quick Controls
- Qt PrintSupport

Outros módulos só devem ser adicionados conforme necessidade real.

## Interface

- **QML / Qt Quick**

## Build system

- **CMake**

Não introduzir qmake.

## Controle de versão

- Git

---

# 5. Arquitetura

A arquitetura lógica do Impage é:

```text
QML / UI
   │
   ▼
Application / Services
   │
   ▼
Core / Engine
   │
   ▼
System / Qt Platform Integration
```

## 5.1 UI

Responsável por:

- janela principal;
- toolbar;
- painel de configuração;
- seleção de imagens;
- canvas de preview;
- feedback visual;
- diálogos.

A UI não deve conhecer detalhes de impressão, coordenadas físicas de impressora ou algoritmos de composição.

## 5.2 Application / Services

Camada intermediária entre UI e domínio.

Exemplos:

- DocumentService
- ImageImportService
- PrintService
- SettingsService
- ExportService

Responsabilidades:

- coordenar operações;
- validar comandos;
- expor objetos adequados para QML;
- transformar ações da interface em operações do Core.

## 5.3 Core / Engine

É a parte mais importante do projeto.

Deve poder funcionar sem depender da interface QML.

Componentes previstos:

- DocumentModel
- PageModel
- GridLayout
- ImageItem
- LayoutEngine
- ImagePlacementEngine
- PageRenderer

O Core deve ser altamente testável.

## 5.4 System

Integração com:

- sistema de arquivos;
- diálogo de impressão;
- impressoras;
- CUPS por meio das APIs do Qt;
- configurações persistentes;
- recursos específicos da plataforma.

Evitar acessar CUPS diretamente enquanto Qt PrintSupport atender às necessidades do projeto.

---

# 6. Estrutura inicial de diretórios

```text
impage/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── docs/
│   ├── architecture.md
│   └── development-guide.md
│
├── src/
│   ├── main.cpp
│   │
│   ├── app/
│   │   ├── ApplicationController.cpp
│   │   ├── ApplicationController.h
│   │   └── services/
│   │
│   ├── core/
│   │   ├── document/
│   │   ├── layout/
│   │   ├── image/
│   │   └── render/
│   │
│   ├── platform/
│   │   ├── printing/
│   │   ├── filesystem/
│   │   └── settings/
│   │
│   └── models/
│
├── qml/
│   ├── Main.qml
│   ├── components/
│   ├── dialogs/
│   ├── panels/
│   └── pages/
│
├── resources/
│   ├── icons/
│   └── translations/
│
└── tests/
    ├── unit/
    ├── integration/
    └── fixtures/
```

Esta estrutura pode evoluir, mas mudanças de alto nível devem ser justificadas.

---

# 7. Modelo conceitual

## 7.1 Document

Representa o trabalho atual do usuário.

Contém:

- página;
- grade;
- imagens;
- parâmetros de layout;
- parâmetros de impressão.

## 7.2 Page

Exemplos de propriedades:

```text
width
height
orientation
marginTop
marginBottom
marginLeft
marginRight
```

As medidas internas devem possuir unidade claramente definida.

## 7.3 Grid

Exemplos:

```text
rows
columns
horizontalSpacing
verticalSpacing
```

## 7.4 Cell

Cada célula da grade é um retângulo calculado pelo LayoutEngine.

Exemplo conceitual:

```text
x
y
width
height
```

## 7.5 ImageItem

Representa uma imagem adicionada ao documento.

Possíveis propriedades:

```text
source
rotation
placementMode
alignment
cellIndex
```

---

# 8. Modos de encaixe da imagem

O projeto deve trabalhar inicialmente com poucos modos claros.

## Fit

A imagem inteira é visível dentro da célula.

- preserva proporção;
- pode gerar espaço vazio.

## Fill

A célula é totalmente preenchida.

- preserva proporção;
- pode cortar parte da imagem.

## Stretch

A imagem ocupa exatamente a célula.

- pode alterar proporção.

Este modo pode existir, mas não deve ser o padrão.

### Padrão recomendado

**Fit**.

---

# 9. Unidades e geometria

Este ponto é crítico para impressão.

Não misturar indiscriminadamente:

- pixels;
- milímetros;
- pontos;
- DPI;
- coordenadas de tela.

O modelo do documento deve trabalhar com uma unidade lógica previsível.

### Recomendação

Usar uma unidade física internamente para a página, preferencialmente **milímetros**, e converter para o dispositivo de renderização apenas na etapa apropriada.

A conversão deve ficar centralizada em utilitários específicos.

Nunca espalhar fórmulas de conversão por diferentes arquivos.

---

# 10. Preview e impressão

A seguinte cadeia deve ser mantida:

```text
DocumentModel
      │
      ▼
 LayoutEngine
      │
      ▼
 PageLayout
      │
   ┌──┴───────────┐
   ▼              ▼
Preview       Print Renderer
```

O resultado do `LayoutEngine` deve ser reutilizável.

### Regra

A impressão nunca deve recalcular a grade por uma lógica paralela escondida dentro de `PrintService`.

---

# 11. Convenções C++

## Standard

C++20.

## Classes

PascalCase:

```cpp
class LayoutEngine;
class PrintService;
class ImageItem;
```

## Métodos e funções

camelCase:

```cpp
calculateLayout();
loadImage();
setPageSize();
```

## Variáveis

camelCase:

```cpp
pageWidth
cellCount
imagePath
```

## Membros privados

Usar sufixo `_`:

```cpp
double pageWidth_;
int rowCount_;
```

## Constantes

Preferir `constexpr`.

Exemplo:

```cpp
constexpr double MillimetersPerInch = 25.4;
```

## `auto`

Permitido quando o tipo estiver evidente ou quando melhorar significativamente a legibilidade.

Evitar:

```cpp
auto value = calculateSomethingComplicated();
```

quando o tipo é importante para compreender o código.

## Ponteiros

Preferir:

- objetos por valor;
- referências;
- `std::unique_ptr`;
- `std::shared_ptr` apenas quando houver propriedade realmente compartilhada.

Evitar `new` e `delete` explícitos.

Para objetos Qt com ownership QObject, respeitar o modelo parent-child do Qt.

---

# 12. Regras para QObject

Não transformar toda classe em `QObject`.

Uma classe só deve herdar `QObject` quando precisar de:

- signals;
- slots;
- properties expostas ao Qt;
- integração com event loop;
- ownership QObject.

Algoritmos puros como `LayoutEngine` devem preferencialmente ser C++ puro.

Isso reduz acoplamento e facilita testes.

---

# 13. Regras QML

## QML deve ser declarativo

Evitar grandes funções JavaScript.

Não implementar cálculo geométrico complexo em QML.

## IDs

camelCase:

```qml
pagePreview
imageGrid
printButton
```

## Componentes

PascalCase:

```text
ImageCell.qml
PagePreview.qml
LayoutPanel.qml
```

## Bindings

Preferir bindings a atualizações imperativas quando possível.

## Estados

Não espalhar flags booleanas desconectadas.

Quando existir estado significativo da aplicação, ele deve estar em um modelo/controlador claramente definido.

---

# 14. Comunicação QML ↔ C++

A interface deve conversar com uma API C++ pequena e estável.

Evitar expor dezenas de classes internas diretamente ao QML.

### Preferência

```text
QML
 │
 ▼
ApplicationController
 │
 ├── DocumentService
 ├── PrintService
 └── SettingsService
```

O `ApplicationController` pode funcionar como fachada inicial.

À medida que a aplicação crescer, objetos específicos podem ser expostos de maneira controlada.

---

# 15. Signals e Slots

Signals devem representar eventos relevantes.

Bom:

```cpp
documentChanged();
printStarted();
printFinished();
printFailed(QString message);
```

Ruim:

```cpp
doSomething();
valueUpdatedAgain();
internalStepDone();
```

Não usar signals como substituto de chamadas normais de função dentro do Core.

---

# 16. Tratamento de erros

Erros esperados devem ser tratados explicitamente.

Exemplos:

- imagem não encontrada;
- arquivo inválido;
- formato não suportado;
- impressora indisponível;
- falha ao abrir diálogo;
- resolução inadequada.

A interface deve receber mensagens compreensíveis ao usuário.

Não mostrar diretamente mensagens internas do C++.

### Exemplo

Interno:

```text
QImageReader: unsupported format
```

Usuário:

```text
Não foi possível abrir esta imagem. O formato pode não ser compatível.
```

---

# 17. Logging

Utilizar `QLoggingCategory`.

Categorias sugeridas:

```text
impage.core
impage.layout
impage.image
impage.print
impage.ui
```

Não deixar `qDebug()` espalhado como mecanismo definitivo de observabilidade.

Dados pessoais ou caminhos sensíveis não devem ser registrados sem necessidade.

---

# 18. Testes

O motor de layout deve possuir testes desde o início.

## Testes unitários prioritários

### LayoutEngine

Testar:

- 1 × 1;
- 2 × 2;
- 3 × 4;
- margens;
- espaçamento;
- orientação portrait;
- orientação landscape;
- células com dimensões esperadas;
- valores inválidos.

### ImagePlacement

Testar:

- Fit;
- Fill;
- Stretch;
- imagens landscape;
- imagens portrait;
- imagens quadradas.

### Conversão de unidades

Testar:

- mm → pixels;
- pixels → mm;
- DPI distintos.

## Regra

Toda correção de bug no Core deve, quando possível, incluir um teste que reproduza o problema.

---

# 19. Formatação e análise estática

Usar:

- `clang-format`;
- `clang-tidy` quando o projeto estiver estabilizado.

O arquivo `.clang-format` deve fazer parte do repositório.

A formatação não deve depender da preferência individual de cada desenvolvedor ou da IA utilizada.

---

# 20. Warnings

Durante desenvolvimento, habilitar warnings relevantes.

Objetivo:

- código sem warnings do projeto.

Não corrigir warnings de bibliotecas externas alterando o código do projeto de forma incorreta.

Nunca desabilitar um warning globalmente apenas para esconder um problema sem investigar a causa.

---

# 21. Commits

Cada commit deve representar uma mudança coerente.

Formato recomendado:

```text
feat: add configurable page grid
fix: correct image fit calculation
refactor: isolate print service
test: add layout engine coverage
docs: update development guide
build: configure Qt Quick module
```

Evitar commits genéricos:

```text
updates
changes
fix
final
teste2
```

---

# 22. Branches

Modelo inicial simples:

```text
main
develop
feature/<nome>
fix/<nome>
```

Para um projeto pequeno, `develop` pode ser eliminado futuramente se não trouxer benefício.

Não criar complexidade de Git Flow sem necessidade.

---

# 23. Pull Requests e revisão

Mesmo quando uma IA implementar uma funcionalidade, a alteração deve ser revisada por unidade lógica.

Uma revisão deve conferir:

- arquitetura;
- ownership;
- thread safety;
- vazamentos;
- duplicação;
- impacto sobre preview;
- impacto sobre impressão;
- testes;
- regressões de UX.

---

# 24. Regras específicas para desenvolvimento com IA

Esta seção deve ser usada como instrução para qualquer agente de programação.

## A IA deve

1. Ler a documentação relevante antes de alterar código.
2. Entender a arquitetura atual antes de criar novas classes.
3. Reutilizar componentes existentes.
4. Fazer alterações pequenas e verificáveis.
5. Explicar qualquer mudança arquitetural proposta.
6. Manter compatibilidade com C++20 e Qt 6.
7. Criar ou atualizar testes para lógica de Core.
8. Compilar após alterações relevantes.
9. Corrigir erros de compilação antes de declarar a tarefa concluída.
10. Informar claramente arquivos criados, removidos ou alterados.

## A IA não deve

1. Reescrever grandes partes do projeto sem solicitação.
2. Alterar arquitetura silenciosamente.
3. Adicionar frameworks concorrentes ao Qt.
4. Adicionar bibliotecas externas sem justificativa.
5. Duplicar lógica entre QML e C++.
6. Colocar lógica de impressão dentro da UI.
7. Criar funções gigantes.
8. Criar classes "Manager" genéricas sem responsabilidade clara.
9. Usar singletons como solução padrão.
10. implementar uma funcionalidade diferente da solicitada "porque seria útil".

---

# 25. Formato recomendado de prompt para implementação

Ao solicitar uma funcionalidade a uma IA, utilizar preferencialmente:

```text
Projeto: Impage
Stack: C++20 + Qt 6 + QML + CMake
Plataforma: Linux

Objetivo:
[descrever apenas a funcionalidade]

Arquitetura relevante:
[arquivos/módulos envolvidos]

Requisitos:
- ...
- ...
- ...

Restrições:
- não alterar a arquitetura global;
- não adicionar dependências externas;
- manter lógica de negócio fora do QML;
- utilizar o LayoutEngine como fonte única do layout;
- preservar compatibilidade com Qt 6.

Antes de implementar:
1. analise os arquivos relacionados;
2. informe brevemente o plano;
3. identifique riscos ou conflitos.

Após implementar:
1. compile;
2. execute os testes relacionados;
3. informe os arquivos alterados;
4. descreva o que foi feito;
5. informe qualquer limitação restante.
```

---

# 26. Regra para refatorações feitas por IA

Uma refatoração não deve alterar comportamento observável sem autorização.

Antes de uma refatoração ampla, a IA deve explicar:

- problema atual;
- impacto;
- solução proposta;
- arquivos afetados;
- risco;
- estratégia de teste.

Só depois deve executar.

---

# 27. Performance

No MVP, priorizar correção e simplicidade.

Otimizar somente quando houver evidência de problema.

Entretanto, alguns cuidados devem existir desde o início:

- não recarregar imagens do disco a cada frame;
- evitar cópias desnecessárias de imagens grandes;
- utilizar cache quando houver benefício claro;
- não bloquear a UI durante operações pesadas;
- não renderizar preview em resolução de impressão sem necessidade.

---

# 28. Threads

Não introduzir multithreading prematuramente.

Operações que futuramente podem sair da thread principal:

- carregamento de muitas imagens;
- geração de thumbnails;
- exportação pesada;
- renderização de documentos grandes.

Qualquer código em thread secundária deve respeitar as regras de thread affinity do Qt.

Objetos visuais QML nunca devem ser manipulados diretamente por workers.

---

# 29. Imagens

O arquivo original não deve ser modificado.

O Impage trabalha de forma não destrutiva.

Alterações como:

- rotação;
- crop;
- fit;
- fill;
- escala;

devem ser representadas como parâmetros do documento, e não aplicadas permanentemente ao arquivo fonte.

---

# 30. Salvamento de projeto

Não é obrigatório para o primeiro protótipo.

Quando implementado, o formato deve preferencialmente armazenar:

- configuração da página;
- grade;
- referências às imagens;
- transformações;
- opções de encaixe.

Um formato legível e versionável, como JSON, é adequado inicialmente.

---

# 31. MVP técnico

A primeira versão funcional deve ser deliberadamente pequena.

## MVP 0.1

- abrir imagens;
- página A4;
- portrait / landscape;
- definir linhas;
- definir colunas;
- margens;
- espaçamento;
- preencher células automaticamente;
- Fit;
- Fill;
- preview;
- diálogo de impressão;
- imprimir.

Não incluir no primeiro MVP:

- editor de imagem;
- filtros;
- texto;
- molduras sofisticadas;
- templates online;
- conta de usuário;
- cloud;
- sincronização;
- banco de dados;
- plugins.

---

# 32. Critério de conclusão do MVP

O MVP pode ser considerado funcional quando:

1. o usuário abre a aplicação;
2. adiciona imagens;
3. cria uma grade;
4. vê todas as imagens posicionadas;
5. altera linhas/colunas e o preview responde corretamente;
6. alterna Fit/Fill;
7. abre o diálogo de impressão;
8. imprime uma página;
9. o resultado físico corresponde ao preview dentro das limitações da impressora.

---

# 33. Prioridade de desenvolvimento

Ordem recomendada:

```text
1. Project skeleton
2. DocumentModel
3. PageModel
4. GridLayout
5. LayoutEngine
6. Unit tests
7. Basic QML window
8. Preview renderer
9. Image import
10. Fit / Fill
11. Page controls
12. PrintService
13. Physical print validation
14. UX polish
15. Packaging
```

A impressão deve ser validada fisicamente antes de investir pesado em polimento visual.

---

# 34. Definition of Done

Uma tarefa só deve ser marcada como concluída quando:

- compila;
- não quebra testes existentes;
- atende aos requisitos definidos;
- respeita a arquitetura;
- não introduz dependência desnecessária;
- possui tratamento mínimo de erro;
- possui código formatado;
- documentação relevante foi atualizada quando necessário.

---

# 35. Filosofia final

O Impage não pretende ser um editor gráfico completo.

Ele deve fazer uma coisa muito bem:

> **transformar imagens em páginas prontas para impressão de maneira simples, visual e previsível no Linux.**

Toda funcionalidade nova deve ser avaliada pela pergunta:

> **Isso melhora diretamente a experiência de composição e impressão?**

Se a resposta for não, provavelmente não pertence ao núcleo do Impage.
