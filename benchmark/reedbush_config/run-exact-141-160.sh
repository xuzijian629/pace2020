#!/bin/sh
#PBS -q h-small
#PBS -Wgroup_list=gk36
#PBS -l select=1:ncpus=1:mpiprocs=1:ompthreads=1
#PBS -l walltime=06:00:00
#PBS -N exact-141-160
####################

set -e

cd $PBS_O_WORKDIR
. benchmark/reedbush_config/env.sh

base_dir=$(pwd -P)
execution_id=$(date +%s)
mkdir -p $base_dir/solutions/$execution_id

for i in {141..160..2}; do
    input_file=$base_dir/data/exact/exact_$i.gr
    echo testing case $(basename $input_file)
    instance=$(basename $input_file)
    best_file=$base_dir/solutions/best/exact/${instance//.gr/.sol}
    set +e
    timeout 1800 $base_dir/benchmark/scripts/run_main.sh $input_file $execution_id
    exit_code=$?
    if [ $exit_code -eq 124 ]; then
        echo '[TLE]'
    elif [ $exit_code -ne 0 ]; then
        echo '[RE]'
    else
        set -e
        $base_dir/benchmark/scripts/run_verify.sh $input_file $execution_id
    fi
done
