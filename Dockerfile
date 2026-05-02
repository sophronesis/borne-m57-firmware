# Multi-stage Dockerfile that compiles the borne m57 firmware to a .uf2.
#
# Build via the helper:
#   scripts/build.sh
#
# Or directly:
#   DOCKER_BUILDKIT=1 docker build --target export --output type=local,dest=build .
#
# Output: ./build/m57_via.uf2

# ---- builder ----
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# arm cross-toolchain + python (qmk's prep scripts) + git for the clone.
RUN apt-get update && apt-get install -y --no-install-recommends \
        git \
        make \
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        libnewlib-arm-none-eabi \
        python3 \
        python3-pip \
        ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# clone vial-qmk + init submodules. this layer is cached unless the RUN line changes.
RUN git clone --depth 1 https://github.com/vial-kb/vial-qmk /vial-qmk \
 && cd /vial-qmk \
 && make git-submodule

WORKDIR /vial-qmk

# drop the keyboard source in. AFTER submodule init so source edits don't bust the heavy layer.
COPY source/m57 keyboards/m57

# stm32 linker scripts that vial-qmk doesn't ship by default.
COPY source/ld/*.ld platforms/chibios/boards/common/ld/

RUN make m57:via

# ---- export stage ----
# scratch image with just the artifact, so `--output type=local` extracts cleanly.
FROM scratch AS export
COPY --from=builder /vial-qmk/.build/m57_via.uf2 /
