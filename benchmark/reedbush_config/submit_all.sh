#!/bin/sh

work_dir=$(dirname $0)
for i in {1..200..20}; do
    qsub $workdir/run-exact-$i-$(($i+20)).sh
done
