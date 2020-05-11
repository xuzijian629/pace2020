#!/bin/sh
#PBS -q h-small
#PBS -Wgroup_list=gk36
#PBS -l select=1:ncpus=1:mpiprocs=1:ompthreads=1
#PBS -l walltime=06:00:00
#PBS -N exact-1-20
####################

set -e

cd $PBS_O_WORKDIR
. benchmark/reedbush_config/env.sh

base_dir=$(pwd -P)
execution_id=$(date +%s)
mkdir -p $base_dir/solutions/$execution_id
source $base_dir/benchmark/config.sh

mkdir -p $base_dir/src/tmp
for i in {181..200..2}; do
    input_file=$base_dir/data/$DATA_CATEGORY/$DATA_CATEGORY\_$i.gr
    echo testing case $(basename $input_file)
    instance=$(basename $input_file)
    max_node=$(head -n 1 $input_file | sed 's/p tdp //' | sed 's/ .*//')
    echo building with -DBITSET_MAX_SIZE=$max_node
    g++ -Ofast -std=c++17 -DBITSET_MAX_SIZE=$max_node -march=native -o $base_dir/src/tmp/main-$instance $base_dir/src/min-sep.cpp
    best_file=$base_dir/solutions/best/$DATA_CATEGORY/${instance//.gr/.sol}
    set +e
    output_file=$base_dir/solutions/$execution_id/${instance//.gr/.sol}
    main=$base_dir/src/tmp/main-$instance
    touch $output_file
    $main < $input_file > $output_file
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
