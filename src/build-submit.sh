#!/bin/bash
set -e
##### EDIT HERE #####
BASE_FILE=min-sep.cpp
#####################

base_dir=$(dirname $0)/..
abs_base_dir=$(cd $base_dir && pwd -P)

cd $abs_base_dir/src
tar -czvf solver.tgz lib/* min-sep.cpp solver
mv solver.tgz ..