#!/bin/sh

# Compatibility entry point for bootstrap archives released under the former
# Impage technical name. All behavior lives in install-purrview.sh.
exec "$(dirname "$0")/install-purrview.sh" "$@"
