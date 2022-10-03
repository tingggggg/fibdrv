reset
set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fibonacci runtime'
set term png enhanced font 'Verdana,10'
set output 'plot_compare.png'
set grid
plot [0:1000][0:10000] \
'out_origin' using 1:2 with linespoints linewidth 2 title "fast doubling (v0)",\
'out_origin2' using 1:2 with linespoints linewidth 2 title "fast doubling (v1)"