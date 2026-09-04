# Preparação para o AUR

O projeto não publica pacotes automaticamente no AUR. A receita em `packaging/arch/PKGBUILD.in` é
usada para construir e testar o pacote oficial `.pkg.tar.zst` a partir do source tarball.

Uma futura publicação pode adotar dois pacotes independentes:

- `purrview`: compila o source tarball oficial com CMake e executa a suíte de testes;
- `purrview-bin`: instala o artefato Arch oficial, sem recompilar.

O `PKGBUILD` submetido deve apontar para uma release imutável, conter o SHA-256 correspondente e não
baixar dependências durante a etapa `build()`. Dependências são declaradas em `depends`,
`makedepends` e `optdepends`.

Antes de publicar, gere a receita final, execute `makepkg --cleanbuild`, valide com `namcap` e revise
o conteúdo com `pacman -Qlp`. A publicação no AUR e o uso de credenciais SSH permanecem operações
manuais do mantenedor.
