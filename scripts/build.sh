#!/usr/bin/env bash
# Builds the borne m57 firmware in a docker container and extracts the .uf2.
# Output: build/m57_via.uf2 (relative to repo root)
#
# Requires: docker (with BuildKit, default since Docker 23.0).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$REPO_ROOT"
mkdir -p build

DOCKER_BUILDKIT=1 docker build --target export --output type=local,dest=build .

echo "wrote: $REPO_ROOT/build/m57_via.uf2"
