#!/bin/bash
# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
if test -f "CCDWrapper_benchmark"; then
    CCD_WRAPPER_BENCHMARK=$(realpath CCDWrapper_benchmark)
else
    CCD_WRAPPER_BENCHMARK="${CCD_WRAPPER_ROOT}/build/release/CCDWrapper_benchmark"
fi
echo "Using $CCD_WRAPPER_BENCHMARK"

DATA_DIRS="$CCD_WRAPPER_ROOT/data"

COLLISON_TYPES=("vf" "ee")

function run_benchmark {
    for i in 0 2 4 6 7 8; do
        $CCD_WRAPPER_BENCHMARK $1 $2 $i -d 0 > /dev/null
    done
}

for DATA_DIR in $DATA_DIRS/*/; do
    echo $DATA_DIR
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        # run processes and store pids in array
        run_benchmark ${dirs[$type]} ${COLLISON_TYPES[$type]} &
        pids[${i}]=$!
    done
done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done
