#!/bin/bash
#SBATCH --job-name="ref_parallel"
#SBATCH -o ref_parallel.out
#SBATCH -e ref_parallel.err
#SBATCH -p test.q
#SBATCH -t 00:10:00
#SBATCH --nodes=1

source /etc/profile.d/modules.sh

module load intel/xe_2018.2
module load gcc/9.3.0
module load intelmpi/gcc/2019.6
module load likwid/5.0.1

export PETSC_DIR=/ddn/data/vtdb72/petsc
export PETSC_ARCH=arch-linux2-c-opt

for n in $(seq 1 20)
do
    solve_time=$(mpiexec -n $((n)) ./bpsraw -problem bp1 -ceed /cpu/self/ref/blocked | grep 'CG Solve Time' | cut -f 2)
    echo $n $solve_time
done

for n in $(seq 1 20)
do
    solve_time=$(mpiexec -n $((n)) ./bpsraw -problem bp2 -ceed /cpu/self/ref/blocked | grep 'CG Solve Time' | cut -f 2)
    echo $n $solve_time
done