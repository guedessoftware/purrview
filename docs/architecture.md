# Arquitetura do Impage

O Impage possui um Shell mínimo responsável pela janela e pelo ciclo de vida dos módulos. O
`shell::ApplicationController` expõe o `ModuleManager` ao QML; o gerenciador cria o backend do
Composer somente quando ele é solicitado e preserva sua instância enquanto a aplicação estiver
aberta.

```text
ShellWindow
    |
    +-- shell::ApplicationController
            |
            +-- ApplicationContext
            |       |
            |       +-- ImageSession
            |       +-- ThumbnailCache (128 MiB, memória)
            |
            +-- ModuleManager
                    |
                    +-- ViewerController (criado sob demanda)
                    |       |
                    |       +-- ViewerState
                    |       +-- FolderImageModel
                    |       +-- ViewerPage / ImageCanvas / FilmStrip
                    |
                    +-- ComposerController (criado sob demanda)
                            |
                            +-- ComposerSessionAdapter
                            +-- ComposerPage

    OpenRequestHandler
        ^        ^         ^          ^
        |        |         |          |
       CLI      IPC   QFileOpenEvent  DropArea
```

`ViewerPage` e `ComposerPage` não criam outras janelas. O Shell alterna o conteúdo do único
`ApplicationWindow` com um crossfade curto e conserva os dois controladores enquanto o processo
estiver aberto.

`desktop::OpenRequest` é o contrato único de entrada externa. `QCommandLineParser` converte a CLI,
`SingleInstanceService` transporta o mesmo contrato em JSON por `QLocalServer`, e eventos do
desktop e drops chegam ao `OpenRequestHandler`. Somente ele decide entre Viewer e Composer e filtra
caminhos locais. O socket fica no runtime XDG do usuário e aceita apenas o próprio usuário; não há
dependência de X11 para ativação da janela.

`ImageSession` pertence ao `ApplicationContext`, e não a um módulo. Ela mantém caminhos, IDs,
imagem atual e seleção durante toda a execução. O Composer conserva seu `DocumentModel` específico
para página e impressão; `ComposerSessionAdapter` projeta a seleção da sessão nesse documento sem
duplicar os arquivos ou decodificar novamente as imagens.

O Viewer mantém uma única fonte de verdade para a imagem atual: a `ImageSession`. O
`FolderImageModel` apenas cataloga, ordena naturalmente e observa as imagens disponíveis na pasta;
uma imagem do catálogo entra na sessão somente quando é visitada ou selecionada. Assim, abrir uma
pasta com centenas de arquivos não envia implicitamente todos eles ao Composer. `ViewerState`
centraliza zoom, visibilidade do filmstrip, tela cheia, visibilidade dos controles/cursor e o único
temporizador de inatividade. Menus, diálogos e interações bloqueiam o auto-ocultamento por meio desse
estado; pan permanece no `ImageCanvas`. A rotação em passos de 90° fica no `ImageEntry`, não altera o
arquivo e é aplicada também pelo renderizador do Composer.

O catálogo é enumerado fora da thread da interface e observado por `QFileSystemWatcher`. Miniaturas
são decodificadas de forma escalada em um `QThreadPool`, retornam ao modelo pela thread principal e
ficam em um `QCache` limitado a 128 MiB. A chave combina caminho, tamanho e data de modificação. O
QML solicita somente os delegates visíveis e uma vizinhança da imagem atual; a imagem anterior e a
próxima são pré-carregadas separadamente para navegação rápida.

As ações de arquivo pertencem ao `ViewerController`. Abrir a pasta usa a integração de URL do Qt,
copiar caminho usa o clipboard do sistema e remover usa exclusivamente `QFile::moveToTrash`, após
confirmação nativa no QML. Uma remoção bem-sucedida invalida metadados, atualiza sessão e catálogo e
avança para a próxima imagem; uma falha não tenta apagar o arquivo diretamente. Cores, espaçamentos,
raios e durações curtas compartilhados pela interface ficam no singleton QML `UiTheme`.

## Fluxo do layout

```text
DocumentModel -> LayoutEngine -> PageLayout (milímetros)
                                      |
                                      v
                                 PageRenderer
                                  /        \
                             Preview     Impressão
```

`LayoutEngine` é a única fonte da geometria. `PageRenderer` converte essa geometria física para o
retângulo do dispositivo somente no momento de desenhar. O preview pode acrescentar guias para
células vazias; imagens, margens, cortes e escalas são os mesmos usados na impressão.

## Responsabilidades

- `src/core/document`: estado não destrutivo da página, grade e imagens.
- `src/core/image`: sessão compartilhada, catálogo de pasta, formatos suportados e cache de
  miniaturas.
- `src/core/layout`: cálculo puro das células, encaixe Fit/Fill/Stretch e conversões de unidade.
- `src/core/render`: desenho compartilhado por tela e impressora.
- `src/shell`: janela lógica, seleção do módulo ativo e ciclo de vida dos backends.
- `src/composer`: fachada QML e validação dos comandos do compositor.
- `src/viewer`: comandos do visualizador e estado lógico de zoom, sem dependência do Composer.
- `src/platform/printing`: diálogo e dispositivo de impressão via Qt PrintSupport.
- `src/platform/desktop`: parser da CLI, protocolo de abertura e instância única por usuário.
- `src/ui` e `qml`: apresentação e interação, sem cálculo físico paralelo.

O documento pode conter várias páginas no tamanho de papel selecionado. A capacidade de cada página
é definida por `linhas × colunas`; imagens são atribuídas às células e páginas na ordem editável da
composição. Alterar a grade recalcula a paginação sem alterar os arquivos ou duplicar a geometria de
layout.

## Regra de composição da sessão

- Com seleção vazia, o Composer utiliza todas as imagens da sessão.
- Com seleção ativa, o Composer utiliza somente as imagens selecionadas, preservando sua ordem.
- Ao reordenar, duplicar ou remover uma ocorrência, o adaptador congela uma lista explícita de IDs
  para o documento; os itens da sessão e os arquivos originais permanecem inalterados.
- Uma mesma ID pode aparecer mais de uma vez na lista explícita, possibilitando imprimir várias
  cópias sem decodificar ou importar novamente a mesma imagem.
- Adicionar imagens pelo próprio Composer acrescenta as novas IDs à lista já editada, sem restaurar
  ocorrências removidas anteriormente.
- Ao abrir um arquivo externo no Viewer enquanto o Composer está ativo, o adaptador congela os IDs
  já usados pelo documento. A imagem externa entra na sessão, mas não altera o trabalho inativo.
