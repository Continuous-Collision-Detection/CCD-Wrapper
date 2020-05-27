#!/bin/bash
# Run the benchmark for a directory

CCD_WRAPPER_ROOT=$(dirname "$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )")
if test -f "CCDWrapper_benchmark"; then
    CCD_WRAPPER_BENCHMARK=$(realpath CCDWrapper_benchmark)
else
    CCD_WRAPPER_BENCHMARK="${CCD_WRAPPER_ROOT}/build/release/CCDWrapper_benchmark"
fi
echo "Using $CCD_WRAPPER_BENCHMARK"

for d in $(seq -100 2); do
    $CCD_WRAPPER_BENCHMARK $1 $2 7 -d $(python3 -c "import math; print(math.pow(10, $d))")
done
