#!/usr/bin/env bash
set -euo pipefail

# Resolve LIB the same way as cmake/BotfLibRoot.cmake

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Pin: matches typical system protobuf 3.21.x; avoids bleeding-edge main + heavy deps.
PROTOBUF_TAG="${BOTF_PROTOBUF_TAG:-v21.12}"
PROTOBUF_URL="https://github.com/protocolbuffers/protobuf.git"

lib_looks_ok() {
  local d="$1"
  [[ -d "${d}/SFML" || -d "${d}/sfml" || -d "${d}/googletest" || -d "${d}/protobuf" || -d "${d}/spdlog" || -f "${d}/README.md" ]]
}

if [[ -n "${BOTF_LIB_ROOT:-}" ]]; then
  LIB_DIR="${BOTF_LIB_ROOT}"
else
  # Sibling of this repo (canonical). Legacy WorkTraining nesting as fallback.
  LIB_SIBLING="$(cd "${SCRIPT_DIR}/.." && pwd)/LIB"
  LIB_LEGACY="$(cd "${SCRIPT_DIR}/../../.." && pwd)/LIB"
  if lib_looks_ok "${LIB_SIBLING}"; then
    LIB_DIR="${LIB_SIBLING}"
  elif lib_looks_ok "${LIB_LEGACY}"; then
    LIB_DIR="${LIB_LEGACY}"
  else
    LIB_DIR="${LIB_SIBLING}"
  fi
fi

PARENT_DIR="$(cd "${LIB_DIR}/.." && pwd)"
if [[ -n "${CMAKE_BUILDS_ROOT:-}" ]]; then
  BUILD_DIR="${CMAKE_BUILDS_ROOT}"
else
  BUILD_DIR="${PARENT_DIR}/BUILD"
fi

echo "[botf] LIB directory:   ${LIB_DIR}"
echo "[botf] BUILD directory: ${BUILD_DIR}"
mkdir -p "${LIB_DIR}" "${BUILD_DIR}"
cd "${LIB_DIR}"

sfml_present() {
  [[ -d SFML || -d sfml ]] && return 0
  compgen -G 'SFML-*' >/dev/null 2>&1
}

clone_if_missing() {
  local name="$1"
  local url="$2"
  if [[ "${name}" == "SFML" ]] && sfml_present; then
    echo "[botf] SFML already present in ${LIB_DIR} — skip clone"
    return 0
  fi
  if [[ -d "${name}" ]]; then
    echo "[botf] ${name} already present in ${LIB_DIR} — skip clone"
    return 0
  fi
  echo "[botf] Cloning ${name} into ${LIB_DIR}/${name} ..."
  git clone --depth 1 "${url}" "${name}"
}

ensure_protobuf() {
  # Incomplete tree (interrupted clone) → remove and re-clone.
  if [[ -d protobuf && ! -f protobuf/CMakeLists.txt ]]; then
    echo "[botf] protobuf looks incomplete — removing and re-cloning"
    rm -rf protobuf
  fi

  if [[ -f protobuf/CMakeLists.txt ]]; then
    echo "[botf] protobuf already present in ${LIB_DIR}/protobuf — skip clone"
  else
    rm -rf protobuf
    echo "[botf] Cloning protobuf ${PROTOBUF_TAG} into ${LIB_DIR}/protobuf ..."
    git clone --depth 1 --branch "${PROTOBUF_TAG}" "${PROTOBUF_URL}" protobuf
  fi

  # Submodules: run when missing (not only on first clone).
  if [[ ! -f protobuf/third_party/googletest/CMakeLists.txt ]]; then
    echo "[botf] Initializing protobuf submodules..."
    (cd protobuf && git submodule update --init --recursive)
  else
    echo "[botf] protobuf submodules look present — skip submodule update"
  fi

  if [[ ! -f protobuf/CMakeLists.txt ]]; then
    echo "[botf] ERROR: protobuf CMakeLists.txt missing after setup" >&2
    exit 1
  fi
  echo "[botf] protobuf ready: ${LIB_DIR}/protobuf"
}

clone_if_missing SFML https://github.com/SFML/SFML.git
clone_if_missing googletest https://github.com/google/googletest.git
clone_if_missing spdlog https://github.com/gabime/spdlog.git
ensure_protobuf

if command -v apt-get >/dev/null 2>&1; then
  echo "[botf] Optional: sudo apt-get install -y libsfml-dev protobuf-compiler libprotobuf-dev libspdlog-dev"
fi

echo "[botf] Done."
echo "[botf] LIB=${LIB_DIR}"
echo "[botf] protobuf=${LIB_DIR}/protobuf  (tag ${PROTOBUF_TAG})"
echo "[botf] spdlog=${LIB_DIR}/spdlog"
if [[ -n "${CMAKE_BUILDS_ROOT:-}" ]]; then
  echo "[botf] CMAKE_BUILDS_ROOT=${CMAKE_BUILDS_ROOT}"
  echo "[botf] Use preset: cmake --preset debug"
else
  echo "[botf] WARNING: CMAKE_BUILDS_ROOT is not set — set it to your BUILD folder."
fi
