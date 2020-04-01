#!/bin/bash
# usage: ./run_main.sh input_file execution_id
set -e

base_dir=$(dirname $0)/../..
source $base_dir/benchmark/config.sh

instance=$(basename $1)
input_file=$base_dir/data/$DATA_CATEGORY/$instance
output_file=$base_dir/solutions/$2/${instance//.gr/.sol}

main=$base_dir/src/main

touch $output_file
$main < $input_file > $output_file
