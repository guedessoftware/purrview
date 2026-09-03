# PurrView

PurrView é uma aplicação Linux para visualizar imagens e compô-las em páginas configuráveis para impressão. O Viewer descobre
automaticamente as imagens da pasta, oferece filmstrip com miniaturas progressivas, Fit, 100%, zoom,
pan, rotação não destrutiva e navegação natural pelo catálogo. O Composer mantém importação de várias
imagens, paginação automática, grade configurável, margens, espaçamentos e modos Fit, Fill e Stretch.
Os formatos aceitos são PNG, JPEG, WebP, BMP, GIF, TIFF, AVIF, HEIF/HEIC e ICNS, desde que o codec
Qt correspondente esteja disponível no sistema.

O Viewer integra a identidade PurrView à barra de ferramentas translúcida sobre a imagem. A película
inferior também sobrepõe a fotografia e se recolhe suavemente após a navegação, enquanto o painel
lateral organiza os metadados sem alterar o arquivo original.

O diálogo **Sobre o PurrView**, disponível no menu Ajuda do Composer e nas opções do Viewer, mostra
versão, licença, ambiente de execução e acesso ao projeto no GitHub.

A janela é fornecida por um Shell modular. Viewer e Composer são páginas carregáveis com backends
independentes. Uma `ImageSession` pertencente ao Shell mantém lista, imagem atual, seleção e rotação
compartilhadas durante as transições entre os módulos.

## Requisitos

- CMake 3.21 ou mais recente
- compilador com C++20
- Qt 6.4 ou mais recente: Core, Gui, Network, QML, Quick, Quick Controls 2, Widgets e PrintSupport
- Exiv2 0.28 ou compatível (opcional, para EXIF avançado; o aplicativo funciona sem ela)

## Instalação

Pacotes binários DEB, RPM, Arch e Flatpak são produzidos em ambientes isolados, além dos arquivos
universais source e bootstrap. Consulte `docs/installation.md` para escolher o pacote adequado.

Para instalação nativa por usuário:

```bash
scripts/install-impage.sh --check
scripts/install-impage.sh --user
```

Execute esses comandos como usuário normal, sem `sudo`; o instalador solicitará autorização apenas
se precisar instalar dependências do sistema.
Os testes completos são executados na preparação do release e não são recompilados durante uma
instalação comum. Use `scripts/install-impage.sh --user --verify` para repeti-los localmente.
Para tornar o Viewer padrão somente nos formatos suportados, acrescente `--set-default-viewer`.

Depois do bootstrap, abra o PurrView pelo menu de aplicativos ou execute:

```bash
~/.local/bin/purrview
```

O caminho `./build/release/impage` existe somente depois de compilar o código-fonte com o preset
`release`; ele não é criado dentro do pacote bootstrap.

As diferenças entre Flatpak, bootstrap e source build estão em
[docs/installation.md](docs/installation.md). Dependências e processo de release ficam em
[docs/dependencies.md](docs/dependencies.md), [docs/packaging.md](docs/packaging.md) e
[docs/releasing.md](docs/releasing.md).

## Compilar e testar a partir do fonte

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
```

O CMake detecta Exiv2 automaticamente. Para produzir deliberadamente uma versão somente com os
metadados básicos fornecidos pelo Qt, configure com `-DIMPAGE_WITH_EXIV2=OFF`.

## Executar

```bash
./build/release/impage
```

O modo padrão continua abrindo o Composer. Para iniciar diretamente no Viewer (as demais imagens
compatíveis da mesma pasta serão descobertas sem importar a pasta inteira para a sessão):

```bash
./build/release/impage --viewer foto1.jpg foto2.png
```

Imagens podem ser selecionadas pelo botão **Adicionar imagens** ou arrastadas para a área de
pré-visualização. A impressão usa o tamanho de papel e a orientação selecionados; A4 é o padrão. As margens configuradas devem
respeitar a área fisicamente imprimível da impressora. Quando as imagens ultrapassam a capacidade
da grade, novas páginas são criadas automaticamente e podem ser navegadas no preview. A barra
superior apresenta miniaturas de todas as imagens; clicar em uma miniatura abre sua página.

Na faixa de miniaturas do Composer, clique apenas navega para a página; use `Ctrl+clique` para
marcar ou desmarcar imagens e `Shift+clique` para selecionar um intervalo. Clique, segure e arraste
uma miniatura para alterar a ordem. Também é possível arrastar uma foto diretamente de um quadro
para outro no preview. As selecionadas podem ser duplicadas com `Ctrl+D` ou removidas da composição
com `Delete`; essas ações nunca apagam nem modificam os arquivos originais. O menu **Imagens**, o
menu de contexto e **Ajuda > Como organizar as imagens** apresentam as mesmas funções e instruções.

Use **Ctrl+V** ou **Arquivo > Colar imagens** para adicionar uma imagem copiada de outro
aplicativo ou arquivos de imagem copiados no gerenciador de arquivos.

Sem opções, informar um ou vários arquivos abre o Viewer; sem arquivos, abre o Composer. Os modos
também podem ser escolhidos explicitamente:

```bash
purrview foto1.jpg foto2.png foto3.webp
purrview --viewer foto1.jpg foto2.png
purrview --compose foto1.jpg foto2.png
```

O PurrView mantém uma instância por usuário. Novos comandos e aberturas vindas do Dolphin são
encaminhados à janela existente por um socket privado em `XDG_RUNTIME_DIR`. Abrir uma imagem
enquanto o Composer está ativo troca temporariamente para o Viewer; voltar ao Composer preserva a
composição, a grade e as demais configurações. O Viewer e o Composer também aceitam arquivos
arrastados diretamente para a janela.

Para instalar os lançadores, o ícone, a associação **PurrView** e a ação **Montar no PurrView**
no menu de contexto do Dolphin/KDE:

```bash
cmake --install build/release --prefix "$HOME/.local"
update-desktop-database "$HOME/.local/share/applications"
kbuildsycoca6
```

## Atalhos do Viewer

| Atalho | Ação |
|---|---|
| `←` / `→` | Imagem anterior / próxima |
| `+` / `-` | Aumentar / diminuir zoom |
| `0` ou `Ctrl+0` | Ajustar à janela |
| `1` | Tamanho real (100%) |
| `R` / `Shift+R` | Girar à direita / esquerda |
| `Ctrl+A` | Selecionar todas as imagens da pasta ativa |
| `Ctrl+Shift+A` | Limpar a seleção |
| `I` ou `Alt+Enter` | Mostrar ou ocultar informações da imagem |
| `F11` ou duplo clique | Entrar ou sair da tela cheia |
| `Esc` | Fechar menu/painel, sair do modo temporário ou da tela cheia, nessa ordem |
| `Delete` | Enviar a imagem atual à lixeira, após confirmação |
| `Ctrl+Shift+C` | Copiar o caminho absoluto da imagem atual |
| `Ctrl+P` | Compor a seleção ou, sem seleção, a imagem atual |

No filmstrip, clique troca a imagem atual, `Ctrl+clique` alterna a seleção e `Shift+clique` seleciona
um intervalo. Com foco na faixa, as setas navegam, `Espaço` alterna a seleção e `Enter` ativa o item.
A roda do mouse sobre a faixa faz rolagem horizontal; o primeiro botão da toolbar mostra ou oculta
as miniaturas. O menu de contexto da imagem reúne navegação, zoom, rotação, informações, pasta,
cópia de caminho e lixeira.

Em tela cheia, toolbar, filmstrip e cursor são ocultados após um curto período sem atividade e
reaparecem imediatamente ao mover o ponteiro ou usar o teclado. Menus, diálogos, painel de
informações, pan e interação com os controles suspendem esse temporizador. A exclusão usa somente a
lixeira disponibilizada pelo sistema; se ela falhar, o arquivo permanece intacto e o erro é
mostrado sem fallback destrutivo.

A imagem atual e a seleção são independentes. A borda forte identifica a imagem visualizada; o
check identifica itens selecionados. A composição segue a ordem visual da pasta, mesmo quando os
cliques ocorreram em outra ordem. Selecionar uma pasta grande cria somente referências leves — os
arquivos originais continuam sendo lidos apenas quando necessários. O cache LRU de miniaturas é
limitado a 128 MiB e o cache de metadados a 256 entradas; a imagem em tamanho integral não fica
duplicada no cache do QML.

## Integração Viewer e Composer

O `ModuleManager` mantém um histórico simples e estados explícitos para cada módulo. Ao iniciar no
Viewer, o backend do Composer, o renderizador e o serviço de impressão permanecem `NotLoaded`; eles
são criados na primeira ação de impressão e reutilizados nas próximas transições. Viewer e Composer
não se referenciam diretamente: a ativação atravessa o Shell com uma lista ordenada de IDs da
`ImageSession` compartilhada.

Uma ativação vinda do Viewer substitui as imagens da composição, mas preserva grade, margens,
orientação e modo de encaixe. Alterações posteriores na seleção não modificam silenciosamente o
documento inativo. Ao voltar, imagem atual, seleção, zoom, pan, rotação, painel de informações,
visibilidade e posição do filmstrip são restaurados.

## Metadados e privacidade

O painel **Informações** mostra imediatamente nome, caminho, formato, tamanho, datas e dimensões.
Quando Exiv2 está disponível, câmera, lente, captura, orientação, exposição e coordenadas GPS são
lidos em segundo plano. Campos e seções ausentes permanecem ocultos.

A extração usa um serviço do Shared Core, compartilhável por Viewer e Composer. O serviço limita o
trabalho em background, ignora resultados de imagens que já deixaram de ser a imagem atual e mantém
um cache em memória identificado por caminho absoluto, tamanho e data de modificação. Coordenadas
GPS são somente lidas do arquivo local: o PurrView não realiza geocodificação nem envia metadados a
serviços externos.

## Atalhos do Composer

| Atalho | Ação |
|---|---|
| `Ctrl+A` ou `Ctrl+O` | Adicionar imagens |
| `Ctrl+V` | Colar imagens da área de transferência |
| `Ctrl+Shift+V` | Voltar ao Viewer |
| `Ctrl+P` | Imprimir |
| `Ctrl+Shift+A` | Selecionar todas as imagens da composição |
| `Ctrl+D` | Duplicar imagens selecionadas |
| `Delete` | Remover imagens selecionadas somente da composição |
| `Ctrl+Shift+Delete` | Limpar imagens, com confirmação |
| `Ctrl+Shift+R` / `Ctrl+Shift+L` | Orientação retrato / paisagem |
| `Ctrl+1` / `Ctrl+2` / `Ctrl+3` | Fit / Fill / Stretch |
| `Alt+↑` / `Alt+↓` | Adicionar / remover linha |
| `Alt+→` / `Alt+←` | Adicionar / remover coluna |
| `PgUp` / `PgDown` | Página anterior / próxima página |
| `Ctrl+Home` / `Ctrl+End` | Primeira / última página |
| `F1` | Mostrar a lista de atalhos |
| `Ctrl+Q` ou `Ctrl+W` | Sair |

## Licença

O PurrView é software livre distribuído sob a GNU General Public License versão 3
(`GPL-3.0-only`). Consulte [LICENSE](LICENSE) para os termos completos. As dependências mantêm suas
próprias licenças, documentadas em [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
