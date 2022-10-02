#!/bin/bash

VERSION=${1:-unknown}  

echo $VERSION

perf stat -r 10 -e cycles,instructions,cache-misses,cache-references,branch-instructions,branch-misses ./fib >> stat_results_${VERSION}.txt

perf record -g --call-graph dwarf ./fib
perf report --stdio -g graph,0.5,caller >> >> report_results_${VERSION}.txt
