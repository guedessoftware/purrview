# Preparação para o Flathub

O PurrView ainda não é submetido automaticamente ao Flathub. O repositório contém dois manifestos:

- `packaging/flatpak/io.github.guedessoftware.PurrView.yml`, usado em desenvolvimento e na CI com a
  árvore local;
- `io.github.guedessoftware.PurrView.flathub.yml.in`, modelo de submissão que aponta para um source
  tarball público e exige checksum.

## Preparar uma submissão

1. Concluir a release, publicar a tag e o `PurrView-X.Y.Z-source.tar.xz`.
2. Copiar o modelo para `io.github.guedessoftware.PurrView.yml` no fork do repositório Flathub.
3. Substituir `@VERSION@` pela versão publicada e `@SOURCE_SHA256@` pelo hash presente em
   `SHA256SUMS`.
4. Confirmar que as quatro URLs de screenshots do AppStream estão públicas.
5. Construir com `flatpak-builder` e validar AppStream e desktop file.
6. Testar Viewer, Composer, exportação PDF e uma impressora CUPS real.
7. Abrir o pull request no Flathub somente após a revisão manual.

O manifesto não solicita rede, acesso geral ao diretório pessoal nem permissões administrativas.
Ele limita arquivos a Pictures e Downloads, além de usar Wayland, fallback X11, GPU e o socket CUPS.
Arquivos escolhidos fora desses diretórios dependem das concessões do portal.

O App ID solicitado contém letras maiúsculas no componente final. Isso é válido para o projeto, mas
`appstreamcli --pedantic` pode emitir a recomendação `cid-contains-uppercase-letter`. Uma eventual
mudança para um ID todo em minúsculas deve ser decidida antes da primeira publicação no Flathub.
