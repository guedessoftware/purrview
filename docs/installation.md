# Instalação

## Pacotes binários

Os pacotes prontos não compilam o aplicativo na máquina do usuário. Baixe o artefato da versão e
use o comando correspondente:

```bash
# Ubuntu 24.04, Mint 22 e derivados compatíveis
sudo apt install ./purrview_X.Y.Z_amd64.deb

# AlmaLinux, Rocky e RHEL 9 compatíveis
sudo dnf install ./purrview-X.Y.Z-1.x86_64.rpm

# Arch Linux e derivados sincronizados
sudo pacman -U ./purrview-X.Y.Z-1-x86_64.pkg.tar.zst

# Qualquer distribuição com Flatpak (execute no diretório que contém o arquivo)
flatpak remote-add --user --if-not-exists flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user ./PurrView-X.Y.Z-x86_64.flatpak
flatpak run io.github.guedessoftware.PurrView
```

Use o Flatpak quando a distribuição não for compatível com uma das bases nativas. O bootstrap e o
tarball de fonte continuam disponíveis para plataformas ou configurações que precisem compilar
localmente. A matriz exata está em `docs/packaging.md`.

## Flatpak

É o canal com maior alcance entre distribuições porque mantém runtime e atualizações isolados. O
aplicativo ainda não está publicado no catálogo do Flathub; o remoto é usado para obter o runtime
KDE exigido pelo bundle.

Para instalar o pacote `.flatpak` pronto, basta ter o comando `flatpak`. `flatpak-builder` é uma
ferramenta de desenvolvimento e não participa da instalação. Para construir a partir do código no
Arch, Garuda ou derivado, instale-a e execute os comandos a partir da raiz do projeto:

```bash
sudo pacman -S flatpak flatpak-builder
flatpak remote-add --user --if-not-exists flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user -y flathub org.kde.Sdk//6.10 org.kde.Platform//6.10
cd /caminho/para/purrview
flatpak-builder --user --install --force-clean \
  build-flatpak packaging/flatpak/io.github.guedessoftware.PurrView.yml
flatpak run io.github.guedessoftware.PurrView
```

Consulte `packaging/flatpak/README.md` para permissões e a pendência de impressão física.

## Bootstrap nativo

Oferece integração máxima com bibliotecas, impressoras e desktop da distribuição:

```bash
scripts/install-purrview.sh --check
scripts/install-purrview.sh --user
```

O padrão instala em `~/.local/opt/purrview`, cria somente links próprios em `~/.local` e registra um
manifesto. Dependências ausentes são mostradas antes de qualquer ação; o gerenciador de pacotes só
é executado após confirmação. `--non-interactive` nunca instala pacotes.

Instalações bootstrap anteriores em `~/.local/opt/impage` ou `/opt/impage` são reconhecidas durante
o upgrade. Os links registrados no manifesto antigo são migrados sem remover arquivos externos.

O fluxo padrão compila somente a aplicação e valida o binário instalado. Os testes já executados na
criação do release não são recompilados na máquina do usuário. Para uma verificação local completa,
use `scripts/install-purrview.sh --user --verify` ou combine `--verify` com `--upgrade`.

O instalador não substitui associações de arquivo sem consentimento. Para tornar o PurrView padrão
para PNG, JPEG, WebP, BMP, GIF, TIFF, AVIF, HEIF/HEIC e ICNS sem incluir outros formatos, use:

```bash
scripts/install-purrview.sh --upgrade --set-default-viewer
```

Execute `--user` no terminal do usuário do desktop, sem `sudo` e fora de um shell `root`. O script
pode solicitar `sudo` somente para instalar dependências do sistema. Isso garante que lançadores e
integrações sejam gravados no `~/.local` da pessoa correta.

```bash
scripts/install-purrview.sh --upgrade
scripts/install-purrview.sh --uninstall
sudo scripts/install-purrview.sh --system
```

`--system` é a única forma de escolher `/opt/purrview` e `/usr/local`; requer autorização explícita.
Upgrade troca o diretório da aplicação somente após build/teste e preserva configurações. Uninstall
remove apenas links registrados e restaura arquivos preexistentes que o bootstrap tenha guardado.
O log fica em `${XDG_CACHE_HOME:-~/.cache}/purrview/install.log`.

O comando instalado fica em `~/.local/opt/purrview/bin/purrview`, com um link conveniente em
`~/.local/bin/purrview`. Abra-o pelo menu de aplicativos ou execute `~/.local/bin/purrview`. O
comando legado `impage` permanece disponível para compatibilidade. Os
lançadores e o menu de contexto do Dolphin usam diretamente o primeiro caminho e não dependem do
`PATH` da sessão gráfica.

Consulte `docs/identity-migration.md` para a mudança de App ID e para a atualização do Flatpak.

Não execute `./build/release/purrview` nem `cmake --install build/release` dentro do pacote bootstrap:
esse diretório pertence exclusivamente ao fluxo de build a partir do fonte descrito abaixo. O
bootstrap compila temporariamente em `~/.cache/purrview/` e já realiza a instalação ao terminar.

## Build a partir do fonte

É recomendado para desenvolvimento e controle completo:

```bash
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

Essa instalação CMake não possui o manifesto de uninstall do bootstrap. Mais detalhes estão em
`docs/building.md`.
