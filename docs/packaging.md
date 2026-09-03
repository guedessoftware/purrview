# Empacotamento multiplataforma

O comando oficial para preparar uma versão completa é:

```bash
scripts/package-all.sh
```

Ele primeiro valida o build Release do host e cria os arquivos universais de fonte e bootstrap.
Depois compila, testa, instala e inspeciona cada pacote binário dentro de um container compatível com
seu destino. O Docker é o mecanismo padrão; outro mecanismo compatível pode ser informado com
`--engine` ou `CONTAINER_ENGINE`.

## Matriz de ambientes

| Formato | Ambiente de construção e teste | Compatibilidade pretendida | Metadados |
|---|---|---|---|
| `.deb` | Ubuntu 24.04, Qt 6.4 | Ubuntu 24.04 e derivados binariamente compatíveis, como Mint 22 e Zorin baseado em 24.04 | Qt básico |
| `.rpm` | AlmaLinux 9 + EPEL/CRB, Qt 6.6 | AlmaLinux, Rocky e RHEL 9 compatíveis; outras distribuições RPM dependem da resolução local de dependências | Qt básico |
| Arch `.pkg.tar.zst` | Arch Linux atualizado | Arch e derivados sincronizados com seus repositórios | Exiv2 avançado |
| `.flatpak` | SDK/Runtime KDE 6.8 oficial do Flathub | Distribuições com Flatpak recente | Exiv2 avançado privado no sandbox |
| `source` / `bootstrap` | Build Release do host | Demais distribuições Linux com dependências suportadas | Exiv2 quando disponível |

DEB e RPM usam deliberadamente as versões estáveis mais antigas de Qt disponíveis nas bases
escolhidas e não incorporam bibliotecas do host. Nesses dois pacotes, o backend de metadados do Qt
permanece funcional; Exiv2 avançado fica desativado porque as bases fornecem Exiv2 0.27. Arch e
Flatpak incluem Exiv2 0.28.

Compatibilidade não é presumida apenas pela extensão do pacote: cada artefato é instalado dentro do
container que o produziu, `purrview --version` é executado, e os arquivos desktop e AppStream são
validados. Impressão em hardware físico e integração gráfica final continuam sendo verificações
manuais de release.

## Uso seletivo

```bash
# Todos os formatos
scripts/package-all.sh

# Somente alguns formatos
scripts/package-all.sh --formats deb,rpm

# Atualizar as imagens-base antes de construir
scripts/package-all.sh --refresh-images

# Reutilizar um build Release do host que já foi validado
scripts/package-all.sh --skip-host-build
```

O Flatpak precisa de `--privileged` para o isolamento interno do `flatpak-builder`. O estado das
dependências fica em `${XDG_CACHE_HOME:-~/.cache}/purrview/packaging/flatpak-kde-6.8`, fora da
árvore-fonte, enquanto todos os artefatos finais ficam em `dist/VERSÃO/`. Nenhum comando cria tag,
envia commits ou publica pacotes.

## Estrutura

- `cmake/Packaging.cmake`: metadados CPack comuns a DEB e RPM;
- `packaging/arch/PKGBUILD.in`: receita Arch preenchida a partir do tarball de fonte;
- `packaging/flatpak/`: manifesto Flatpak;
- `packaging/containers/`: ambientes reproduzíveis e validações de instalação;
- `scripts/release.sh`: artefatos universais e validação do host;
- `scripts/package-all.sh`: orquestra a matriz completa.

As imagens de container e o runtime Flatpak ocupam vários gigabytes na primeira execução. Docker e
o cache do Flatpak reutilizam essas camadas nas versões seguintes.
