#!/bin/bash

# Array sizes: Choose a range of sizes from 1K to 8M.
# Example sizes: 1024 (1K), 2048, 4096 (4K), 16384 (16K), 65536 (64K), 262144 (256K), 1048576 (1M), 2097152 (2M), 4194304 (4M), 8388608 (8M)

for n in 1024 2048 4096 16384 65536 262144 1048576 2097152 4194304 8388608
do
   echo "Running tests with ARRAYSIZE=$n"
   # Assuming 'main.cpp' is your source file:
   g++ main2.cpp -DARRAYSIZE=$n -o simd_test -lm -fopenmp
   ./simd_test
done
