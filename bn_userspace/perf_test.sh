#!/bin/bash

VERSION=${1:-unknown}  

echo $VERSION

sudo perf stat -o stat_results_${VERSION}.txt -r 10 -e cycles,instructions,cache-misses,cache-references,branch-instructions,branch-misses ./fib

sudo perf record -g --call-graph dwarf ./fib
sudo perf report --stdio -g graph,0.5,caller >> report_results_${VERSION}.txt

sudo rm *.data
