#!/bin/bash
set -e
##### EDIT HERE #####
BASE_FILE=min-sep.cpp
#####################

base_dir=$(dirname $0)/..
abs_base_dir=$(cd $base_dir && pwd -P)

echo building cpp files

docker run -v $abs_base_dir:$abs_base_dir gcc:9.3 bash -c "\
    g++ -Ofast -std=c++17 -mpopcnt -static -o $abs_base_dir/src/main $abs_base_dir/src/$BASE_FILE &&
    cd $abs_base_dir/src &&
    tar -czvf main.tgz main &&
    mv main.tgz .."
