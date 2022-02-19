#!/bin/bash
#SBATCH --job-name="bp1_refserial_degree"
#SBATCH -o bp1_refserial_degree.out
#SBATCH -e bp1_refserial_degree.err
#SBATCH -p test.q
#SBATCH -t 00:03:00
#SBATCH --nodes=1

source /etc/profile.d/modules.sh

module load intel/xe_2018.2
module load gcc/9.3.0
module load intelmpi/gcc/2019.6
module load likwid/5.0.1

export PETSC_DIR=/ddn/data/vtdb72/petsc
export PETSC_ARCH=arch-linux2-c-opt

for n in $(seq 1 16)
do
    solve_time=$(./bpsraw -problem bp1 -degree $((n)) -ceed /cpu/self/ref/serial | grep 'CG Solve Time' | cut -f 2)
    echo $n $solve_time
done