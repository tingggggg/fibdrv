reset
set xlabel 'F(n)'
set ylabel 'time (ns)'
set title 'Fast Doubling runtime'
set term png enhanced font 'Verdana,10'
set output 'plot_output_clz.png'
set grid
plot [0:92][0:300] \
'plot_input_clz' using 1:2 with linespoints linewidth 2 title "1U << 3",\
'' using 1:3 with linespoints linewidth 2 title "1U << 16",\
'' using 1:4 with linespoints linewidth 2 title "1U << 6",\
'' using 1:5 with linespoints linewidth 2 title "__builtin_clz()"