#!/bin/bash
# Usage: ./build.sh [--static]
#   --static  produce a portable binary (see STATIC_BUILD in CMakeLists.txt)
set -e

STATIC_BUILD=OFF
case "$1" in
    --static) STATIC_BUILD=ON ;;
    "")       ;;
    *)        echo "unknown option: $1" >&2; echo "usage: $0 [--static]" >&2; exit 1 ;;
esac

cd ../XCSP3-CPP-Parser
./build.sh
cd ../pfactory
./bootstrap
make
cd ../cosoco/
cmake -DCMAKE_BUILD_TYPE=Release -DSTATIC_BUILD=${STATIC_BUILD} -G "Unix Makefiles" .
#cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" .
cmake --build . --target cosoco -- -j 8
