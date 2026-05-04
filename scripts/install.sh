#!/bin/bash

set -u

if [ "$#" -gt 1 ]; then
  echo "Usage: $0 [DIRPATH]"
  exit 1
fi

BINARY="build/src/autolab"
CMAKE_CACHE="build/CMakeCache.txt"

if [ ! -f "${CMAKE_CACHE}" ]; then
  echo "Could not find ${CMAKE_CACHE}, Run ./scripts/build.sh first."
  exit 1
fi

if [ ! -f "${BINARY}" ]; then
  echo "Could not find ${BINARY}. Run ./scripts/build.sh first."
  exit 1
fi

if [ "$#" -eq 1 ]; then
  DEST_DIR="$1"
else
  echo "No target install directory supplied, using default."

  CACHED_INSTALL_PREFIX="$(awk -F= '/^CMAKE_INSTALL_PREFIX:/ { print $2; exit }' "${CMAKE_CACHE}")"
  if [ -n "${CACHED_INSTALL_PREFIX}" ]; then
    DEST_DIR="${CACHED_INSTALL_PREFIX}/bin"
    echo "Found default install location ${DEST_DIR} from CMAKE_INSTALL_PREFIX"
  else
    DEST_DIR="/usr/local/bin"
    echo "CMAKE_INSTALL_PREFIX not found, using ${DEST_DIR} as default."
  fi
fi


if [ ! -d "${DEST_DIR}" ]; then
  echo "Install destination does not exist or is not a directory: ${DEST_DIR}"
  exit 1
fi

if cp "${BINARY}" "${DEST_DIR}/"; then
  echo "Installed autolab to ${DEST_DIR}"
  exit 0
fi

echo "Could not copy to ${DEST_DIR}; it may be protected. Retrying with sudo..."
if sudo cp "${BINARY}" "${DEST_DIR}/"; then
  echo "Installed autolab to ${DEST_DIR}"
  exit 0
fi

echo "Failed to install autolab to ${DEST_DIR}"
exit 1
