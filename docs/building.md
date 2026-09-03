# Compilação

Instale as dependências descritas em `docs/dependencies.md` e obtenha o repositório ou o source
archive oficial. O build nunca deve ocorrer dentro da árvore de fontes.

## Presets

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev

cmake --preset release
cmake --build --preset release
ctest --preset release
```

Os diretórios são `build/dev` e `build/release`. Para diagnóstico de memória:

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan
```

## Recursos opcionais

```bash
cmake --preset release -DIMPAGE_WITH_EXIV2=AUTO
cmake --preset release -DIMPAGE_WITH_EXIV2=ON
cmake --preset release -DIMPAGE_WITH_EXIV2=OFF
```

`AUTO` é o padrão. `ON` falha claramente se Exiv2 0.28+ não estiver disponível.

## Validações adicionais

```bash
cmake --build build/release --target impage_qmllint
scripts/validate-release.sh build/release/generated/io.github.impage.Impage.metainfo.xml
```

`SOURCE_DATE_EPOCH` é respeitado pelo fluxo de artefatos. O código não incorpora data aleatória,
caminho absoluto do desenvolvedor nem commit Git na versão oficial.
