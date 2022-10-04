#!/bin/bash
#Exam correctness of the fibonacci numbers calculated with my bn library
#
TEST_DATA=out

./fib_show > $TEST_DATA
python do_fib_check.py -f $TEST_DATA