#!/bin/bash
for t in 1 2 4 6 8
do
  for n in 2 3 4 5 10 15 20 30 40 50
  do
     g++   main2.cpp  -DNUMT=$t -DNUMCAPITALS=$n  -o proj03-2  -lm  -fopenmp
    ./proj03-2
  done
done