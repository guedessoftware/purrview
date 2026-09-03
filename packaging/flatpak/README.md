# Flatpak

O manifesto usa `org.kde.Platform//6.10` e `org.kde.Sdk//6.10`. A série Qt 6.10 é estável e atende ao
Qt 6.4 mínimo do PurrView. `inih` e Exiv2 0.28.8 são construídos no sandbox com fontes e checksums
fixos; não alteram bibliotecas do sistema hospedeiro.

As permissões são deliberadamente limitadas: Wayland, fallback X11, aceleração gráfica, impressão
CUPS e acesso de leitura/escrita apenas às pastas Pictures e Downloads. Arquivos escolhidos pelo
portal fora dessas pastas continuam disponíveis pela concessão do seletor. O aplicativo não pede
rede nem acesso geral ao diretório pessoal.

## Construir

`flatpak-builder` só é necessário para gerar o aplicativo a partir do código. No Arch, Garuda ou
derivado, instale as ferramentas e execute o build a partir da raiz do repositório:

```bash
sudo pacman -S flatpak flatpak-builder
flatpak remote-add --user --if-not-exists flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user -y flathub org.kde.Sdk//6.10 org.kde.Platform//6.10
cd /caminho/para/purrview
flatpak-builder --user --install --force-clean \
  build-flatpak packaging/flatpak/io.github.impage.Impage.yml
flatpak run io.github.impage.Impage
```

Para instalar o bundle pronto não use `flatpak-builder`:

```bash
flatpak remote-add --user --if-not-exists flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo
flatpak install --user ./PurrView-X.Y.Z-x86_64.flatpak
```

Antes de enviar ao Flathub, substitua a fonte local `dir` por um arquivo da tag oficial com SHA-256.
O script de release rejeita divergência entre a versão/Exiv2 do manifesto e a política central.

## Impressão

O socket CUPS permite ao Qt PrintSupport consultar a impressão do host. Diálogo, PDF e impressora
física devem ser validados no Flatpak em cada release. Na baseline 0.5.0 essa validação física ainda
está pendente; o pacote não deve ser publicado como pronto no Flathub até ela ser concluída.

No sandbox, abrir a localização e mover para a lixeira podem depender do portal e das permissões do
arquivo. Não foi concedido `--filesystem=host` para contornar essa limitação.
