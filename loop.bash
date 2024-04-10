#!/bin/bash
for t in 1 2 4 6 8
do
  for n in 1 10 100 1000 10000 100000 500000 1000000
  do
     g++   main.cpp  -DNUMT=$t -DNUMTRIALS=$n  -o proj01  -lm  -fopenmp
    ./proj01
  done
done