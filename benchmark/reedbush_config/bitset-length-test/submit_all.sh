#!/bin/sh

work_dir=$(dirname $0)
for i in {1..200..20}; do
    qsub $work_dir/run-exact-$i-$(($i+19)).sh
done