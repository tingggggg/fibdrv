#!/bin/bash
#This script tweak the system setting to minimize the unstable factors while
#analyzing the performance of fibdrv.
#
#2020/3/21  ver 1.0
CPUID=7
ORIG_ASLR=`cat /proc/sys/kernel/randomize_va_space`
ORIG_GOV=`cat /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor`
ORIG_TURBO=`cat /sys/devices/system/cpu/intel_pstate/no_turbo`

sudo bash -c "echo 0 > /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo performance > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo"

#measure the performance of fibdrv
sudo make client_plot
sudo make client_plot_clz
# make client_statistic
sudo make unload
sudo make
sudo make load
# rm -f plot_input_statistic
# sudo taskset -c 7 ./client_statistic
sudo taskset -c 7 ./client_plot > plot_input
sudo taskset -c 7 ./client_plot_clz > plot_input_clz
# gnuplot scripts/plot-statistic.gp
gnuplot scripts/plot.gp
gnuplot scripts/plot_clz.gp
make unload

# restore the original system settings
sudo bash -c "echo $ORIG_ASLR >  /proc/sys/kernel/randomize_va_space"
sudo bash -c "echo $ORIG_GOV > /sys/devices/system/cpu/cpu$CPUID/cpufreq/scaling_governor"
sudo bash -c "echo $ORIG_TURBO > /sys/devices/system/cpu/intel_pstate/no_turbo"