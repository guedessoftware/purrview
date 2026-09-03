# Third-party notices

O código-fonte do PurrView é distribuído sob GPL-3.0-only. Dependências não são relicenciadas pelo projeto.

| Componente | Versão/política | Licença | Origem |
|---|---|---|---|
| Qt 6 | 6.5 ou superior; runtime Flatpak 6.10 | LGPL-3.0/GPL-3.0/comercial, conforme distribuição | https://www.qt.io |
| Exiv2 | 0.28 ou superior; fallback Flatpak 0.28.8 | GPL-2.0-or-later | https://exiv2.org |
| KDE Flatpak Runtime | ramo 6.10 | Conjunto de licenças informado pelo runtime | https://invent.kde.org/packaging/flatpak-kde-runtime |

No build nativo, Qt e Exiv2 são dependências do sistema e seus avisos são fornecidos pela
distribuição. No Flatpak, os avisos instalados pelos módulos e pelo runtime permanecem no pacote.
