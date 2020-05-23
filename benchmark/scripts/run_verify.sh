#!/bin/bash
# usage: ./run_verify.sh input_file execution_id
set -e

base_dir=$(dirname $0)/../..
source $base_dir/benchmark/config.sh

instance=$(basename $1)
input_file=$base_dir/data/$DATA_CATEGORY/$instance
best_file=$base_dir/solutions/best/$DATA_CATEGORY/${instance//.gr/.sol}
output_file=$base_dir/solutions/$2/${instance//.gr/.sol}

verify=$base_dir/benchmark/scripts/verify

res=$($verify $input_file $output_file)
if [[ $res == "*|FAILED" ]]; then
    echo '[WA]'
else
    size=$(head -n 1 $output_file)
    echo '[AC]' treedepth=$size
    if [ -f $best_file ]; then
        best_size=$(head -n 1 $best_file)
        if [ $size -lt $best_size ]; then
            echo best solution updated from $best_size
            cp $output_file $best_file
        elif [ $size -gt $best_size ]; then
            echo suboptimal: known best is $best_size
            exit 133
        fi
    else
        cp $output_file $best_file
    fi
fi
