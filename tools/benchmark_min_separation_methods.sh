# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
if test -f "benchmark_min_separation"; then
    CCD_WRAPPER_BENCHMARK=$(realpath benchmark_min_separation)
else
    CCD_WRAPPER_BENCHMARK="${CCD_WRAPPER_ROOT}/build/release/benchmark_min_separation"
fi
echo "Using $CCD_WRAPPER_BENCHMARK"

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

COLLISON_TYPES=("vf" "ee")

for DATA_DIR in $DATA_DIRS/unit-tests/ $DATA_DIRS/erleben*/ ; do
    echo $DATA_DIR
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        for method in 5 7; do
            for d in "1e-2" "1e-8" "1e-16" "1e-30" "1e-100"; do
                $CCD_WRAPPER_BENCHMARK ${dirs[$type]} ${COLLISON_TYPES[$type]} $method $d
                echo
            done
        done
    done
done
