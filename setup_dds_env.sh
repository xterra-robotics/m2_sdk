#!/bin/bash
set -e  # Abort on any non-zero exit status.

echo "==================================================="
echo "        Svan M2 DDS Environment Setup"
echo "==================================================="

ROOT_DIR="${PWD}"
THIRD_PARTY_DIR="${ROOT_DIR}/third_party"
INSTALL_DIR="${THIRD_PARTY_DIR}/install"

echo "Repository root   : ${ROOT_DIR}"
echo "Dependency install: ${INSTALL_DIR}"
echo ""

mkdir -p "${THIRD_PARTY_DIR}"
cd "${THIRD_PARTY_DIR}"

# ---------------------------------------------------------
# Step 1/4 — Clone Eclipse CycloneDDS (C core library)
# ---------------------------------------------------------
echo "[1/4] Cloning Eclipse CycloneDDS (core)..."
if [ ! -d "cyclonedds" ]; then
    git clone https://github.com/eclipse-cyclonedds/cyclonedds.git
else
    echo "  -> cyclonedds already present, skipping clone."
fi

# ---------------------------------------------------------
# Step 2/4 — Clone Eclipse CycloneDDS-CXX (C++ bindings)
# ---------------------------------------------------------
echo "[2/4] Cloning Eclipse CycloneDDS-CXX (C++ bindings)..."
if [ ! -d "cyclonedds-cxx" ]; then
    git clone https://github.com/eclipse-cyclonedds/cyclonedds-cxx.git
else
    echo "  -> cyclonedds-cxx already present, skipping clone."
fi

# ---------------------------------------------------------
# Step 3/4 — Build and install CycloneDDS core
#
# Type-library and discovery features are enabled because the M2 hardware
# layer relies on them for DDS participant and topic introspection.
# ---------------------------------------------------------
echo "[3/4] Building Eclipse CycloneDDS (core)..."
mkdir -p "${THIRD_PARTY_DIR}/cyclonedds/build"
cd "${THIRD_PARTY_DIR}/cyclonedds/build"

cmake .. \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -DBUILD_TESTING=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DENABLE_TYPELIB=ON \
    -DENABLE_TYPE_DISCOVERY=ON \
    -DENABLE_TOPIC_DISCOVERY=ON

make -j"$(nproc)"
make install

# ---------------------------------------------------------
# Step 4/4 — Build and install CycloneDDS-CXX bindings
#
# CMAKE_PREFIX_PATH must point to the installed CycloneDDS CMake config
# so that find_package(CycloneDDS) resolves to our local copy.
# ---------------------------------------------------------
echo "[4/4] Building Eclipse CycloneDDS-CXX (C++ bindings)..."
mkdir -p "${THIRD_PARTY_DIR}/cyclonedds-cxx/build"
cd "${THIRD_PARTY_DIR}/cyclonedds-cxx/build"

cmake .. \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
    -DCMAKE_PREFIX_PATH="${INSTALL_DIR}/lib/cmake/CycloneDDS" \
    -DBUILD_TESTING=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DENABLE_TYPELIB=ON \
    -DENABLE_TOPIC_DISCOVERY=ON

make -j"$(nproc)"
make install

echo ""
echo "==================================================="
echo "        DDS Environment Successfully Configured!"
echo "==================================================="
echo "You can now build the SDK with:"
echo "  cmake -DBUILD_DDS=ON [-DBUILD_EXAMPLES=ON] .."