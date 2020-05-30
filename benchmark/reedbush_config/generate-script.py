import subprocess
import random


def template(commit_hash, first, use_reedbushl):
    return """#!/bin/sh
#PBS -q {}-{}
#PBS -Wgroup_list=gk36
#PBS -l select=1:ncpus=1:mpiprocs=1:ompthreads=1
#PBS -l walltime=02:00:00
#PBS -N {}-{}
####################

set -e

cd $PBS_O_WORKDIR
. benchmark/reedbush_config/env.sh

base_dir=$(pwd -P)
execution_id=$(date +%s)
mkdir -p $base_dir/solutions/$execution_id
source $base_dir/benchmark/config.sh
""".format('l' if use_reedbushl else 'h', 'small' if use_reedbushl else 'short', commit_hash, first)


def add_problem(problem):
    return """
input_file=$base_dir/data/$DATA_CATEGORY/$DATA_CATEGORY\_""" + problem + """.gr
echo testing case $(basename $input_file)
instance=$(basename $input_file)
max_node=$(head -n 1 $input_file | sed 's/p tdp //' | sed 's/ .*//')
echo building with -DBITSET_MAX_SIZE=$max_node
g++ -Ofast -std=c++17 -DBITSET_MAX_SIZE=$max_node -march=native -o $base_dir/src/tmp/main-$instance $base_dir/src/min-sep.cpp
best_file=$base_dir/solutions/best/$DATA_CATEGORY/${instance//.gr/.sol}
set +e
output_file=$base_dir/solutions/$execution_id/${instance//.gr/.sol}
main=$base_dir/src/tmp/main-$instance
touch $output_file
/usr/bin/time -f "time: %e [sec]\nmemory: %M [KB]" timeout 1800 $main < $input_file > $output_file
exit_code=$?
if [ $exit_code -eq 124 ]; then
    echo '[TLE]'
elif [ $exit_code -ne 0 ]; then
    echo '[RE]'
else
    set -e
    $base_dir/benchmark/scripts/run_verify.sh $input_file $execution_id
fi
"""


def generate_scripts(commit_hash, problems, use_reedbushl, base_dir):
    script = template(commit_hash, problems[0], use_reedbushl)
    for problem in problems:
        script += add_problem(problem)
    output = '{}/tmp/run-{}-{}.sh'.format(base_dir, commit_hash, problems[0])
    with open(output, 'w') as f:
        f.write(script)


if __name__ == '__main__':
    commit_hash = input().split()[1][:5]
    base_dir = input()
    num_parallel = int(input())
    mode = int(input())
    problems = subprocess.getoutput(
        'cat {}/hard-cases.txt'.format(base_dir)).split()
    each = (len(problems) + num_parallel - 1) // num_parallel
    for i in range(0, len(problems), each):
        if mode == 0:
            use_reedbushl = random.randint(0, 1)
        elif mode == 1:
            use_reedbushl = 0
        else:
            use_reedbushl = 1
        generate_scripts(commit_hash,
                         problems[i: min(i + each, len(problems))], use_reedbushl, base_dir)
