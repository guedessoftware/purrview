#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE_ROOT=/src
WORK_ROOT=/work
OUTPUT_ROOT=/out
VERSION_VALUE="$(<"${SOURCE_ROOT}/VERSION")"
BUNDLE_PATH="${OUTPUT_ROOT}/PurrView-${VERSION_VALUE}-x86_64.flatpak"

rm -rf -- "${WORK_ROOT}/build" "${WORK_ROOT}/repo" "${WORK_ROOT}/verify-repo"
flatpak-builder --force-clean --disable-rofiles-fuse \
    --state-dir="${WORK_ROOT}/state" \
    --repo="${WORK_ROOT}/repo" \
    "${WORK_ROOT}/build" \
    "${SOURCE_ROOT}/packaging/flatpak/io.github.impage.Impage.yml"
flatpak build "${WORK_ROOT}/build" /app/bin/purrview --version \
    | grep -Fqx "PurrView ${VERSION_VALUE}"
flatpak build-bundle \
    --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo \
    "${WORK_ROOT}/repo" "${BUNDLE_PATH}" io.github.impage.Impage
ostree init --repo="${WORK_ROOT}/verify-repo" --mode=archive-z2
flatpak build-import-bundle "${WORK_ROOT}/verify-repo" "${BUNDLE_PATH}"
ostree refs --repo="${WORK_ROOT}/verify-repo" \
    | grep -Fqx 'app/io.github.impage.Impage/x86_64/master'
chown --reference="${OUTPUT_ROOT}" "${BUNDLE_PATH}"

printf 'Flatpak bundle validated with KDE runtime 6.8: %s\n' "${BUNDLE_PATH}"
