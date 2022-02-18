#!/bin/bash
#SBATCH --job-name="myjob"
#SBATCH -o myjob.%A.out
#SBATCH -e myjob.%A.err
#SBATCH -p test.q
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --cpus-per-task=24
#SBATCH --mail-type=ALL
#SBATCH --mail-user=qnrn96@durham.ac.uk

source /etc/profile.d/modules.sh

module load intel/xe_2018.2
module load gcc/9.3.0
module load intelmpi/gcc/2019.6
module load likwid/5.0.1

export PETSC_DIR=/ddn/data/vtdb72/petsc
export PETSC_ARCH=arch-linux2-c-opt

./bpsraw -problem bp1