# Run the benchmark for a directory

if test -f "CCDWrapper_benchmark"; then
    CCD_WRAPPER_BENCHMARK=$(realpath CCDWrapper_benchmark)
else
    CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
    CCD_WRAPPER_BENCHMARK="${CCD_WRAPPER_ROOT}/build/release/CCDWrapper_benchmark"
fi
echo "Using $CCD_WRAPPER_BENCHMARK"

DATA_DIR_ROOT=$1

DATA_DIRS=("$DATA_DIR_ROOT/vertex-face/" "$DATA_DIR_ROOT/edge-edge/")
COLLISON_TYPES=("vf" "ee")

for type in 0 1; do
    for i in $(seq 0 4); do
        $CCD_WRAPPER_BENCHMARK ${DATA_DIRS[$type]} ${COLLISON_TYPES[$type]} $i
        echo
    done
done
