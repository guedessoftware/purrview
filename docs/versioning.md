# Versionamento

O PurrView usa [Semantic Versioning](https://semver.org/) no formato `MAJOR.MINOR.PATCH`.

- Antes de 1.0, `0.x.0` representa funcionalidade compatível relevante e `0.x.y` uma correção.
- Depois de 1.0, MAJOR indica incompatibilidade, MINOR funcionalidade compatível e PATCH correção.
- Pré-releases usam somente sufixos SemVer, por exemplo `0.9.0-dev`, `0.9.0-beta.1` e
  `0.9.0-rc.1`.

O arquivo `VERSION` é a fonte oficial. CMake, binário, AppStream e scripts de release leem esse
arquivo. Toda alteração relevante entra primeiro em `[Unreleased]` no `CHANGELOG.md`.

Durante a fase pré-1.0, a série `0.8.x` permanece como ciclo atual. A futura `0.9.0` inicia
estabilização e feature freeze; `0.9.x` recebe correções, e `1.0.0` será a primeira versão estável
pública. A preparação do repositório não altera `VERSION` automaticamente.

Commits devem usar um prefixo estável quando o projeto estiver sob Git: `feat:`, `fix:`, `perf:`,
`refactor:`, `docs:`, `build:` ou `ci:`. Releases oficiais recebem tags anotadas `vX.Y.Z`, criadas
somente após revisão explícita; scripts locais nunca fazem push ou tag automaticamente.
