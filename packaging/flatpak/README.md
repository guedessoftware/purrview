# Flatpak

O manifesto usa `org.kde.Platform//6.8` e `org.kde.Sdk//6.8`. A série Qt 6.8 é estável e atende ao
Qt 6.4 mínimo do PurrView. `inih` e Exiv2 0.28.8 são construídos no sandbox com fontes e checksums
fixos; não alteram bibliotecas do sistema hospedeiro.

As permissões são deliberadamente limitadas: Wayland, fallback X11, aceleração gráfica, impressão
CUPS e acesso de leitura/escrita apenas às pastas Pictures e Downloads. Arquivos escolhidos pelo
portal fora dessas pastas continuam disponíveis pela concessão do seletor. O aplicativo não pede
rede nem acesso geral ao diretório pessoal.

## Construir

```bash
flatpak install --user flathub org.kde.Sdk//6.8 org.kde.Platform//6.8
flatpak-builder --user --install --force-clean \
  build-flatpak packaging/flatpak/io.github.impage.Impage.yml
flatpak run io.github.impage.Impage
```

Antes de enviar ao Flathub, substitua a fonte local `dir` por um arquivo da tag oficial com SHA-256.
O script de release rejeita divergência entre a versão/Exiv2 do manifesto e a política central.

## Impressão

O socket CUPS permite ao Qt PrintSupport consultar a impressão do host. Diálogo, PDF e impressora
física devem ser validados no Flatpak em cada release. Na baseline 0.5.0 essa validação física ainda
está pendente; o pacote não deve ser publicado como pronto no Flathub até ela ser concluída.

No sandbox, abrir a localização e mover para a lixeira podem depender do portal e das permissões do
arquivo. Não foi concedido `--filesystem=host` para contornar essa limitação.
