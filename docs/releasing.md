# Processo de release

Releases seguem Semantic Versioning e são deliberadamente revisadas. O script local não cria tags,
não faz push e não publica no Flathub.

## Checklist

1. Decidir o impacto MAJOR/MINOR/PATCH.
2. Atualizar `VERSION`.
3. Mover alterações de `[Unreleased]` para a nova versão e data no `CHANGELOG.md`.
4. Criar `docs/releases/X.Y.Z.md` e atualizar `cmake/ReleaseMetadata.cmake`.
5. Confirmar versões/checksums em `cmake/DependencyVersions.cmake` e no Flatpak.
6. Executar a matriz em containers com `scripts/package-all.sh`.
7. Validar AppStream/desktop e a instalação realizada em cada container.
8. Testar Viewer, Composer, PDF e impressora física nativa e no Flatpak.
9. Confirmar que `scripts/package-all.sh` terminou sem erros.
10. Verificar cada arquivo com `SHA256SUMS`.
11. Revisar limitações e changelog manualmente.
12. Criar, somente após aprovação explícita, uma tag anotada ou assinada.
13. Publicar artefatos e, quando aprovado, atualizar o manifesto Flathub para a tag/checksum.
14. Criar uma nova seção `[Unreleased]` vazia para o próximo ciclo.

## Artefatos

O script completo gera em `dist/X.Y.Z/`:

- `PurrView-X.Y.Z-source.tar.xz`;
- `PurrView-X.Y.Z-bootstrap.tar.xz`;
- `purrview_X.Y.Z_amd64.deb`;
- `purrview-X.Y.Z-1.x86_64.rpm`;
- `purrview-X.Y.Z-1-x86_64.pkg.tar.zst`;
- `PurrView-X.Y.Z-x86_64.flatpak`;
- `SHA256SUMS`.

Quando houver uma chave GPG configurada, gere também a assinatura destacada:

```bash
scripts/package-all.sh --sign-checksums
# ou escolha a chave explicitamente
scripts/package-all.sh --gpg-key ID_DA_CHAVE
gpg --verify dist/X.Y.Z/SHA256SUMS.asc dist/X.Y.Z/SHA256SUMS
```

Para assinar a tag da release:

```bash
git tag -s vX.Y.Z -m "PurrView X.Y.Z"
git tag -v vX.Y.Z
```

Sem uma chave disponível, use uma tag anotada com `git tag -a`. O projeto nunca cria chaves,
armazena material privado ou torna assinatura obrigatória para builds locais.

Arquivos tar usam ordem, proprietário e data controlados por `SOURCE_DATE_EPOCH`. Builds são
interrompidos se Git existir e a árvore estiver suja. Sem Git, a limitação é informada e nenhuma tag
é simulada.

O changelog é curado; ele não é produzido automaticamente a partir dos commits. Consulte
`docs/packaging.md` para os ambientes, garantias e comandos seletivos.
