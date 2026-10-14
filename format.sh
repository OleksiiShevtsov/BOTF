#!/usr/bin/env bash
set -euo pipefail

# Format only BOTF project C++ files: *.cpp *.h *.hpp
# under client/, server/, net/, common/sources/, tests/.
# Does NOT touch LIB, .proto, or generated *.pb.cc.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

if ! command -v clang-format >/dev/null 2>&1; then
  echo "[botf] clang-format not found on PATH." >&2
  echo "[botf] e.g. sudo apt-get install -y clang-format" >&2
  exit 1
fi

echo "[botf] Formatting with:"
clang-format --version

roots=(client server net protocol common/sources tests)
files=()
for root in "${roots[@]}"; do
  [[ -d "${root}" ]] || continue
  while IFS= read -r -d '' f; do
    files+=("${f}")
  done < <(find "${root}" -type f \( -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) \
            ! -name '*.pb.cc' ! -name '*.pb.h' -print0)
done

if [[ ${#files[@]} -eq 0 ]]; then
  echo "[botf] No project C++ sources found."
  exit 0
fi

clang-format -i "${files[@]}"
echo "[botf] Formatted ${#files[@]} project C++ file(s)."
