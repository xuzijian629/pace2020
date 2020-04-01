workdir=$(dirname $0)
g++ -O2 -std=c++17 -o $workdir/verify $workdir/verify.cpp
