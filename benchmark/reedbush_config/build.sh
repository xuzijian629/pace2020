#!/bin/sh
#PBS -q h-small
#PBS -Wgroup_list=gk36
#PBS -l select=1:ncpus=1:mpiprocs=1:ompthreads=1
#PBS -l walltime=06:00:00
#PBS -N build
####################

set -e

cd $PBS_O_WORKDIR
. benchmark/reedbush_config/env.sh

base_dir=$(pwd -P)
$base_dir/src/build.sh
$base_dir/benchmark/scripts/build.sh
