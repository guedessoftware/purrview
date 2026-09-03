# Changelog

Todas as alterações relevantes do PurrView (anteriormente Impage) são registradas neste arquivo. O formato segue
[Keep a Changelog](https://keepachangelog.com/) e as versões seguem Semantic Versioning.

## [Unreleased]

## [0.8.4] - 2026-09-03

### Documentation

- README reformulado com identidade visual do PurrView, capturas reais e anônimas do Composer e do
  Viewer, visão geral dos recursos e caminhos mais claros para instalação e contribuição.

### Fixed

- Viewer e Composer voltam a carregar corretamente no Qt 6.4 do Ubuntu 24.04; o shell não tenta
  mais instanciar componentes QML `Bound` fora de seu contexto de criação.
- Diálogos do shell e do Viewer deixam de produzir ciclos de `implicitWidth` nas versões mais
  antigas do Qt declaradas como compatíveis.
- A fonte local do Flatpak ignora o cache de empacotamento, evitando falhas de permissão causadas
  por resíduos de containers anteriores.
- A matriz da CI instala também os codecs e utilitários verificados pelo ciclo do bootstrap,
  evitando falhas artificiais após o build e os testes já terem passado.
- As instruções do Flatpak agora configuram o Flathub explicitamente, distinguem a instalação do
  bundle pronto da compilação com `flatpak-builder` e indicam o diretório correto do projeto.
- O Flatpak migrou do runtime KDE 6.8 descontinuado para o ramo estável 6.10, mantendo manifesto,
  CI, containers, documentação e validações de release sincronizados.

## [0.8.3] - 2026-09-03

### Changed

- Licença do projeto alterada de MIT para GNU GPL versão 3 (`GPL-3.0-only`) no código-fonte,
  aplicativo, AppStream e pacotes DEB, RPM, Arch, Flatpak, source e bootstrap.
- Informações pessoais de autoria e contato foram removidas da interface e dos metadados públicos;
  a autoria coletiva passa a ser apresentada como **PurrView contributors**.
- Repositório preparado para publicação limpa, preservando testes, integração contínua e scripts
  reproduzíveis de geração de pacotes, sem incluir builds ou artefatos gerados.

## [0.8.2] - 2026-09-03

### Changed

- O diálogo **Sobre o PurrView** ganhou cabeçalho próprio, fundo opaco, melhor hierarquia visual e
  espaçamento mais confortável, sem interferência do conteúdo exibido atrás dele.
- O acesso ao GitHub agora é um botão completo e destacado; o contato ganhou interação clara e o
  fechamento foi consolidado em um único controle no canto superior.

## [0.8.1] - 2026-09-03

### Added

- Diálogo **Sobre o PurrView** com logotipo, versão automática, descrição, licença, sistema,
  arquitetura, versão real do Qt e acesso ao projeto no GitHub.
- Acesso ao diálogo pelo menu **Ajuda** do Composer e pelo menu de opções do Viewer, inclusive sem
  uma imagem carregada.

### Changed

- Site, suporte, changelog e metadados dos pacotes passam a usar o endereço definitivo
  `https://github.com/guedessoftware/purrview`.

## [0.8.0] - 2026-09-03

### Added

- Pacotes binários DEB, RPM, Arch e Flatpak, além dos arquivos universais source e bootstrap.
- Script único `scripts/package-all.sh` para compilar, testar, instalar, inspecionar e assinar com
  SHA-256 todos os formatos em ambientes isolados.
- Containers de validação baseados em Ubuntu 24.04, AlmaLinux 9 e Arch Linux, além do container
  oficial KDE 6.8 do ecossistema Flathub.
- Receita Arch via `makepkg` e metadados CPack específicos para DEB e RPM.
- Documento da matriz de compatibilidade e instruções de instalação para cada pacote.

### Changed

- A baseline mínima do Qt foi reduzida de 6.5 para 6.4 para suportar nativamente o Ubuntu 24.04.
- DEB e RPM usam o backend básico de metadados do Qt nas bases estáveis; Arch e Flatpak mantêm
  metadados avançados com Exiv2 0.28.
- O Flatpak passa a empacotar `inih`, exportar o ícone sob o ID da aplicação e validar o bundle por
  reimportação em um repositório vazio.

## [0.7.1] - 2026-09-02

### Changed

- Marca PurrView e ação **Imprimir** foram integradas à barra flutuante existente; o cabeçalho
  adicional do Viewer foi removido para devolver toda a área útil à imagem.
- Película de miniaturas agora sobrepõe a fotografia com transparência leve e desliza para baixo
  após 2,3 segundos sem navegação, reaparecendo ao alcançar a borda inferior.
- Contador textual de posição foi removido da película e do rodapé do Viewer.

## [0.7.0] - 2026-09-02

### Added

- Cabeçalho próprio do PurrView no Viewer, com logotipo e ação de impressão em gradiente.
- Contador de posição integrado à película de miniaturas e logotipo no estado vazio.

### Changed

- Barra flutuante, navegação, seleção e película adotam superfícies translúcidas e os destaques
  violeta, rosa e coral da identidade PurrView.
- Painel de informações foi reorganizado em seções mais claras, com controles consistentes com o
  tema escuro e melhor leitura de metadados.
- O cabeçalho se recolhe automaticamente em tela cheia para preservar a experiência imersiva.

## [0.6.1] - 2026-09-02

### Fixed

- Ícone do PurrView agora é instalado como PNG nos tamanhos padrão do tema Hicolor, corrigindo o
  ícone de interrogação no menu de aplicativos e na barra de tarefas do KDE.
- Lançadores instalados usam o caminho absoluto do PNG de 256 px e o instalador atualiza também o
  cache de ícones.
- A janela passa a publicar um PNG incorporado em `_NET_WM_ICON`, evitando diferenças entre os
  renderizadores SVG do Qt e do Plasma.

## [0.6.0] - 2026-09-02

### Added

- Nova identidade visual **PurrView**, usando o logotipo vetorial do gatinho no aplicativo e na
  integração com o desktop Linux.
- Barra de status do Composer com resumo de papel, orientação e grade, além de controles de zoom.
- Novo comando público `purrview`; o comando histórico `impage` permanece disponível por
  compatibilidade.

### Changed

- Composer reorganizado com cabeçalho de marca, painel lateral em cartão, cores de destaque e botão
  de impressão em gradiente.
- Lançadores, associação de arquivos, menu de contexto do Dolphin, título da janela e AppStream
  agora apresentam o nome PurrView.

## [0.5.7] - 2026-09-02

### Added

- Reordenação direta no preview: uma foto pode ser segurada e arrastada de um quadro para outro.
- Guia **Ajuda > Como organizar as imagens** com seleção, reordenação, duplicação e remoção.
- `Shift+clique` seleciona um intervalo iniciado pela última miniatura marcada.

### Changed

- Clique simples na faixa apenas navega para a página correspondente; seleção passa a exigir
  `Ctrl+clique`, seguindo a interação esperada no desktop.

### Fixed

- Reordenação na faixa superior agora acompanha fisicamente o ponteiro e calcula o destino pela
  posição final, corrigindo o arraste que não alterava a ordem.

## [0.5.6] - 2026-09-02

### Added

- Seleção múltipla das miniaturas do Composer com ações visíveis, menu **Imagens**, menu de contexto
  e atalhos para selecionar, duplicar e remover ocorrências da composição.
- Reordenação por arrastar e soltar; quando várias miniaturas estão marcadas, o bloco inteiro é
  movido preservando sua ordem relativa.

### Changed

- A descrição redundante do tamanho da página foi removida do cabeçalho lateral; dimensões ficam
  concentradas no seletor de papel.
- Duplicação e remoção passam a editar somente a composição de impressão, sem duplicar, excluir ou
  reordenar os arquivos originais na sessão do Viewer.
- Novas importações são acrescentadas à ordem editada sem restaurar imagens removidas anteriormente.

## [0.5.5] - 2026-09-02

### Added

- Seleção de papel A3, A4, A5, Carta, Ofício/Legal e Foto 10 × 15 cm, aplicada ao preview e à
  configuração enviada para a impressora.
- Presets rápidos de grade 1×1, 1×2, 2×2, 2×3, 3×3 e 3×4 entre a orientação e os controles
  detalhados de linhas e colunas.

### Changed

- Novas composições começam em 1 linha × 1 coluna, aproveitando toda a página quando somente uma
  imagem é recebida.
- A ação visível do Viewer e do Composer agora se chama apenas **Imprimir**; a descrição acessível
  continua informando o conteúdo da seleção.

## [0.5.4] - 2026-09-02

### Added

- Suporte real, condicionado aos codecs Qt instalados, para AVIF, HEIF, HEIC e ICNS no Viewer,
  Composer, diálogo de abertura, associação MIME e menu de contexto do Dolphin.
- O bootstrap instala os plugins KDE Image Formats necessários a AVIF e HEIF/HEIC.
- A opção `--verify` permite compilar e executar todos os testes também na máquina do usuário.
- A opção `--set-default-viewer` registra o Impage apenas para os MIME types realmente suportados,
  sem depender da seleção global de formatos do KDE.

### Changed

- A instalação e a atualização bootstrap normais não compilam novamente os testes já executados na
  preparação do release, reduzindo o tempo e o número de alvos compilados.

## [0.5.3] - 2026-09-02

### Changed

- O Shell declara explicitamente decoração, menu do sistema e controles nativos de minimizar,
  maximizar e fechar para integração com o gerenciador de janelas.
- A toolbar do Viewer agora sobrepõe a imagem, fica mais transparente em repouso, ganha opacidade
  ao receber o ponteiro e pode ser fixada pelo novo botão de alfinete.
- O título interno duplicado do Viewer foi removido; o nome do arquivo permanece somente no título
  nativo da janela.

## [0.5.2] - 2026-09-02

### Fixed

- Os lançadores e o menu de contexto criados pelo bootstrap agora usam o caminho absoluto do
  executável instalado, sem depender de `~/.local/bin` estar no `PATH` da sessão gráfica.
- Ao concluir, o instalador informa o comando exato para iniciar o aplicativo.
- A documentação diferencia os comandos da instalação bootstrap dos comandos exclusivos do build
  a partir do fonte.

## [0.5.1] - 2026-09-02

### Fixed

- O teste de apresentação de metadados agora respeita o separador decimal do locale ativo.
- O bootstrap recusa instalação por usuário em `/root` e orienta a execução no usuário do desktop.

## [0.5.0] - 2026-09-02

### Added

- Primeira baseline formal do projeto.
- Viewer com catálogo de pasta, filmstrip, zoom, rotação, seleção, metadados e tela cheia.
- Composer A4 com grade configurável, paginação automática, preview e impressão.
- Sessão compartilhada e navegação preservada entre Viewer e Composer.
- Integração Linux com MIME, lançadores, arrastar e soltar, clipboard e menu do Dolphin.
- Menu de contexto do Viewer, cópia de caminho e envio seguro para a lixeira.
- Presets CMake, manifesto Flatpak, bootstrap nativo, CI e geração de artefatos versionados.

### Changed

- A versão do aplicativo passa a ser derivada exclusivamente do arquivo `VERSION`.
- Exiv2 passa a utilizar a política configurável `AUTO`, `ON` ou `OFF`.
- O bootstrap declara CUPS e XKB explicitamente em todas as famílias suportadas.
- A CI usa executor versionado e actions fixadas por commit.

### Fixed

- Registro do item QML de preview consolidado para builds limpos e Release.
- Metadados desktop e AppStream preparados para validação e distribuição.
- `--help` e `--version` funcionam sem inicializar o backend gráfico, inclusive no CI headless.

### Performance

- Cache LRU de miniaturas limitado a 128 MiB e cache de metadados limitado a 256 entradas.
- Composer permanece sob demanda quando a aplicação inicia diretamente no Viewer.

[Unreleased]: https://github.com/guedessoftware/purrview/compare/v0.8.4...HEAD
[0.8.4]: https://github.com/guedessoftware/purrview/compare/v0.8.3...v0.8.4
[0.8.3]: https://github.com/guedessoftware/purrview/compare/v0.8.2...v0.8.3
[0.8.2]: https://github.com/guedessoftware/purrview/compare/v0.8.1...v0.8.2
[0.8.1]: https://github.com/guedessoftware/purrview/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/guedessoftware/purrview/compare/v0.7.1...v0.8.0
[0.7.1]: https://github.com/guedessoftware/purrview/compare/v0.7.0...v0.7.1
[0.7.0]: https://github.com/guedessoftware/purrview/compare/v0.6.1...v0.7.0
[0.6.1]: https://github.com/guedessoftware/purrview/compare/v0.6.0...v0.6.1
[0.6.0]: https://github.com/guedessoftware/purrview/compare/v0.5.7...v0.6.0
[0.5.7]: https://github.com/guedessoftware/purrview/compare/v0.5.6...v0.5.7
[0.5.6]: https://github.com/guedessoftware/purrview/compare/v0.5.5...v0.5.6
[0.5.5]: https://github.com/guedessoftware/purrview/compare/v0.5.4...v0.5.5
[0.5.4]: https://github.com/guedessoftware/purrview/compare/v0.5.3...v0.5.4
[0.5.3]: https://github.com/guedessoftware/purrview/compare/v0.5.2...v0.5.3
[0.5.2]: https://github.com/guedessoftware/purrview/compare/v0.5.1...v0.5.2
[0.5.1]: https://github.com/guedessoftware/purrview/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/guedessoftware/purrview/releases/tag/v0.5.0
