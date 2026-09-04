#!/usr/bin/env bash
set -Eeuo pipefail

CHECKSUM_FILE=""
GPG_KEY="${PURRVIEW_GPG_KEY:-}"

usage() {
    cat <<'EOF'
Usage: scripts/sign-checksums.sh CHECKSUM_FILE [--key KEY_ID]

Creates an armored detached signature beside CHECKSUM_FILE. The private key is
read from the maintainer's local GnuPG keyring and is never stored by PurrView.
EOF
}

while (($# > 0)); do
    case "$1" in
    --key)
        shift
        (($# > 0)) || { usage >&2; exit 2; }
        GPG_KEY="$1"
        ;;
    --help | -h) usage; exit 0 ;;
    -*) usage >&2; exit 2 ;;
    *)
        [[ -z "${CHECKSUM_FILE}" ]] || { usage >&2; exit 2; }
        CHECKSUM_FILE="$1"
        ;;
    esac
    shift
done

[[ -n "${CHECKSUM_FILE}" && -f "${CHECKSUM_FILE}" ]] || { usage >&2; exit 2; }
command -v gpg >/dev/null 2>&1 || {
    printf 'GPG is required only when checksum signing is requested.\n' >&2
    exit 1
}

arguments=(--batch --yes --armor --detach-sign --output "${CHECKSUM_FILE}.asc")
[[ -z "${GPG_KEY}" ]] || arguments+=(--local-user "${GPG_KEY}")
gpg "${arguments[@]}" "${CHECKSUM_FILE}"
printf 'Checksum signature created: %s.asc\n' "${CHECKSUM_FILE}"
