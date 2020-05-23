#!/bin/bash
set -e

base_dir=$(dirname $0)/..
abs_base_dir=$(cd $base_dir && pwd -P)

echo building cpp files

$base_dir/src/build.sh
$base_dir/benchmark/scripts/build.sh

execution_id=$(date +%s)
mkdir -p $base_dir/solutions/$execution_id

source $base_dir/benchmark/config.sh

for input_file in $(ls $base_dir/data/$DATA_CATEGORY/*.gr | head -n $MAX_TESTS); do
    echo testing case $(basename $input_file)
    instance=$(basename $input_file)
    best_file=$base_dir/solutions/best/$DATA_CATEGORY/${instance//.gr/.sol}
    if [ $DATA_CATEGORY != 'rand' -a -f $best_file -a $SKIP_SOLVED -ne 0 ]; then
        echo 'skipped'
        continue
    fi
    set +e
    timeout $TIME_LIMIT_SEC $base_dir/benchmark/scripts/run_main.sh $input_file $execution_id
    exit_code=$?
    if [ $exit_code -eq 124 ]; then
        echo '[TLE]'
    elif [ $exit_code -ne 0 ]; then
        echo '[RE]'
    else
        set -e
        $base_dir/benchmark/scripts/run_verify.sh $input_file $execution_id
        if [ $? -eq 133 ]; then
            exit 1
        fi
    fi
done
