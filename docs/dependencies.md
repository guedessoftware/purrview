# Dependências

## Mínimos oficiais

| Dependência | Versão | Classificação | Uso |
|---|---:|---|---|
| CMake | 3.21 | Core | Configuração e instalação |
| Compilador C++ | C++20 | Core | Todo o código nativo |
| Ninja | versão da distribuição | Core no fluxo oficial | Presets e bootstrap |
| Qt | 6.4 | Core | Core, Gui, Network, Qml, Quick, QuickControls2, PrintSupport e Widgets |
| Plugins Qt imageformats | mesma série do Qt | Runtime | WebP e TIFF |
| KDE Image Formats | série compatível com Qt 6 | Runtime | AVIF e HEIF/HEIC |
| Exiv2 | 0.28 | Opcional | EXIF/XMP/IPTC avançado |
| CUPS/stack de impressão | distribuição | Sistema | Impressoras e PDF via Qt PrintSupport |

O mínimo Qt 6.4 permite produzir o DEB no Ubuntu 24.04 sem incorporar um Qt privado. APIs de carga
QML usadas pela aplicação permanecem compatíveis com essa baseline. O bootstrap não compila Qt
automaticamente.

`PURRVIEW_WITH_EXIV2=AUTO` usa Exiv2 quando compatível, `ON` exige a dependência e `OFF` produz o
backend básico do Qt. A ausência de Exiv2 nunca impede Viewer, Composer ou impressão.

## Pacotes por família

- Debian 13 / Ubuntu 26.04: `build-essential cmake ninja-build pkg-config git xdg-utils qt6-base-dev
  qt6-declarative-dev qt6-image-formats-plugins kimageformat6-plugins libcups2-dev
  libxkbcommon-dev`; opcional
  `libexiv2-dev`.
- Fedora 44: `gcc-c++ cmake ninja-build pkgconf-pkg-config git xdg-utils qt6-qtbase-devel
  qt6-qtdeclarative-devel qt6-qtimageformats kf6-kimageformats libavif libheif cups-devel
  libxkbcommon-devel`; opcional
  `exiv2-devel`.
- Arch e derivados: `base-devel cmake ninja pkgconf git xdg-utils qt6-base qt6-declarative
  qt6-imageformats kimageformats libavif libheif cups libxkbcommon`; opcional `exiv2`.

As versões fixadas para conteúdo empacotado ficam em `cmake/DependencyVersions.cmake`. O Flatpak
usa Exiv2 0.28.8 com URL versionada e SHA-256, instalado somente em `/app`. Não se usa `latest`,
`main`, `master`, `LD_LIBRARY_PATH` nem instalação de bibliotecas privadas em `/usr/lib`.
