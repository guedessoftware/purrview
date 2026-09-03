#!/usr/bin/env bash

impage_build_testing_value() {
    if ((IMPAGE_VERIFY_BUILD)); then
        printf 'ON\n'
    else
        printf 'OFF\n'
    fi
}

impage_build_stage() {
    local version
    version="$(<"${IMPAGE_SOURCE_ROOT}/VERSION")"
    local build_root="${XDG_CACHE_HOME:-${HOME}/.cache}/impage/bootstrap-build-${version}"
    local install_parent
    install_parent="$(dirname "${IMPAGE_INSTALL_ROOT}")"
    mkdir -p "${build_root}" "${install_parent}"
    IMPAGE_STAGE_ROOT="$(mktemp -d "${install_parent}/.impage-stage.XXXXXX")"
    export IMPAGE_STAGE_ROOT
    local build_testing
    build_testing="$(impage_build_testing_value)"

        impage_info "Building PurrView ${version}..."
    cmake -S "${IMPAGE_SOURCE_ROOT}" -B "${build_root}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING="${build_testing}" \
        -DIMPAGE_WITH_EXIV2=AUTO \
        -DCMAKE_INSTALL_RPATH='$ORIGIN;$ORIGIN/../lib'
    cmake --build "${build_root}" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
    if ((IMPAGE_VERIFY_BUILD)); then
        impage_info "Running local verification suite..."
        ctest --test-dir "${build_root}" --output-on-failure
    else
        impage_info "Local test build skipped; use --verify for a full machine-specific check."
    fi
    cmake --install "${build_root}" --prefix "${IMPAGE_STAGE_ROOT}"
    impage_configure_desktop_launchers
    "${IMPAGE_STAGE_ROOT}/bin/impage" --version
    impage_ok "Staged build validated"
}
