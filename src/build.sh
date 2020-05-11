##### EDIT HERE #####
BASE_FILE=min-sep.cpp
#####################
src_dir=$(dirname $0)

g++ -Ofast -std=c++17 -o $src_dir/main $src_dir/$BASE_FILE
