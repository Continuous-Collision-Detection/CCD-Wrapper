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

function run_all_methods
{
    for i in $(seq 0 7); do
        $CCD_WRAPPER_BENCHMARK $1 $2 $i > /dev/null
    done
}

for DATA_DIR in $DATA_DIRS/*/; do
    echo $DATA_DIR
    dirs=("$DATA_DIR/vertex-face/" "$DATA_DIR/edge-edge/")
    for type in 0 1; do
        # run processes and store pids in array
        run_all_methods ${dirs[$type]} ${COLLISON_TYPES[$type]} &
        pids[${i}]=$!
    done
done

# wait for all pids
for pid in ${pids[*]}; do
    wait $pid
done
