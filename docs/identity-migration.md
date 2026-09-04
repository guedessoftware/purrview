# Identidade técnica e compatibilidade

PurrView é o nome público e técnico principal do projeto. O executável real, os targets CMake, o
módulo QML, os logs e as integrações novas usam `purrview` ou `PurrView`.

## Executáveis

- principal: `purrview`;
- compatibilidade temporária: `impage`, um wrapper mínimo que encaminha todos os argumentos para
  `purrview`.

O alias existe para scripts e atalhos criados antes da consolidação do nome. Novas integrações não
devem depender dele.

## App ID

- atual: `io.github.guedessoftware.PurrView`;
- legado: `io.github.impage.Impage`.

O ID atual identifica AppStream, Flatpak, desktop files, instância única e integrações exportadas.
O ID legado deve aparecer somente nesta documentação, no histórico e em rotinas explícitas de
migração.

## Atualização de instalações nativas

Pacotes DEB, RPM e Arch removem arquivos que deixaram de pertencer ao pacote durante a atualização.
O bootstrap reconhece instalações antigas em `~/.local/opt/impage` ou `/opt/impage`, instala o novo
conteúdo em `purrview` e remove somente os links registrados pelo manifesto legado. O script
`scripts/install-impage.sh` permanece como entrada compatível para chamar `install-purrview.sh`.

As preferências antigas do aplicativo são importadas uma única vez, sem apagar configurações do
usuário.

## Atualização do Flatpak

Para o Flatpak, uma troca de App ID representa outro aplicativo. Versões instaladas com o ID legado
não recebem automaticamente o bundle do novo ID. Depois de confirmar que não há trabalho pendente,
o usuário pode trocar a instalação com:

```bash
flatpak uninstall io.github.impage.Impage
flatpak install --user ./PurrView-X.Y.Z-x86_64.flatpak
flatpak run io.github.guedessoftware.PurrView
```

O sandbox novo usa `~/.var/app/io.github.guedessoftware.PurrView`. O PurrView não solicita acesso ao
sandbox antigo; preferências locais antigas do Flatpak devem ser recriadas após a migração.
