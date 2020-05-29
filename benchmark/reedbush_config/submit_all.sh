#!/bin/sh
set -e

base_dir=$(dirname $0)
abs_base_dir=$(cd $base_dir && pwd -P)

num_parallel=10
use_reedbush_l=0

git_log=$(git log | head -n 1)

rm -rf $base_dir/tmp
mkdir $base_dir/tmp

echo "$git_log
$abs_base_dir
$num_parallel
$use_reedbush_l" | python $base_dir/generate-script.py

for f in $(ls $base_dir/tmp/*.sh); do
    qsub $f
done
