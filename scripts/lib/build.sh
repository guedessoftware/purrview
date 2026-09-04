#!/usr/bin/env bash

purrview_build_testing_value() {
    if ((PURRVIEW_VERIFY_BUILD)); then
        printf 'ON\n'
    else
        printf 'OFF\n'
    fi
}

purrview_build_stage() {
    local version
    version="$(<"${PURRVIEW_SOURCE_ROOT}/VERSION")"
    local build_root="${XDG_CACHE_HOME:-${HOME}/.cache}/purrview/bootstrap-build-${version}"
    local install_parent
    install_parent="$(dirname "${PURRVIEW_INSTALL_ROOT}")"
    mkdir -p "${build_root}" "${install_parent}"
    PURRVIEW_STAGE_ROOT="$(mktemp -d "${install_parent}/.purrview-stage.XXXXXX")"
    export PURRVIEW_STAGE_ROOT
    local build_testing
    build_testing="$(purrview_build_testing_value)"

        purrview_info "Building PurrView ${version}..."
    cmake -S "${PURRVIEW_SOURCE_ROOT}" -B "${build_root}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TESTING="${build_testing}" \
        -DPURRVIEW_WITH_EXIV2=AUTO \
        -DCMAKE_INSTALL_RPATH='$ORIGIN;$ORIGIN/../lib'
    cmake --build "${build_root}" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
    if ((PURRVIEW_VERIFY_BUILD)); then
        purrview_info "Running local verification suite..."
        ctest --test-dir "${build_root}" --output-on-failure
    else
        purrview_info "Local test build skipped; use --verify for a full machine-specific check."
    fi
    cmake --install "${build_root}" --prefix "${PURRVIEW_STAGE_ROOT}"
    purrview_configure_desktop_launchers
    "${PURRVIEW_STAGE_ROOT}/bin/purrview" --version
    purrview_ok "Staged build validated"
}
