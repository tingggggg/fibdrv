#!/usr/bin/env python3

import os
import glob
import numpy as np

def outlier_filter(datas, threshold = 2):
    datas = np.array(datas)
    z = np.abs((datas - datas.mean()) / datas.std())
    return datas[z < threshold]

def generate_gp(filepath_list):
    with open('results/compare.gp', 'w') as f:
        f.writelines([
            "reset\n",
            "set xlabel 'F(n)'\n",
            "set ylabel 'time (ns)'\n",
            "set title 'Fibonacci runtime'\n",
            "set term png enhanced font 'Verdana,10'\n",
            "set output 'plot_compare.png'\n",
            "set grid\n",
            "plot [0:1000][0:10000] \\\n"
        ])

        for filepath in filepath_list:
            # print(filepath)
            if filepath != filepath_list[-1]:
                f.write(f"""'{filepath}' using 1:2 with linespoints linewidth 2 title "fast doubling ({filepath.split('_')[-1]})" """ + ",\\\n")
            else:
                f.write(f"""'{filepath}' using 1:2 with linespoints linewidth 2 title "fast doubling ({filepath.split('_')[-1]})"\n""")

def remove_file():
    for ret_file in glob.glob('results/rm_outlier*'):
        os.remove(ret_file)
    os.remove('results/compare.gp')

filepath_list = []
for ret_file in sorted(glob.glob('results/out_*')):
    with open(ret_file, 'r') as f:
        lines = f.readlines()
        datas = []
        for line in lines:
            datas.append(int(line.rstrip('\n').split(' ')[1]))
        datas = outlier_filter(datas, 0.8)

    rm_outlier_filepath = os.path.join(os.path.dirname(ret_file), f'rm_outlier_{os.path.basename(ret_file)}')
    filepath_list.append(rm_outlier_filepath)
    with open(rm_outlier_filepath, 'w') as f:
        for i, data in enumerate(datas):
            f.write(f'{i} {data}\n')

generate_gp(filepath_list)

os.system('gnuplot results/compare.gp')

remove_file()