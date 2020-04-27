set -e
base_dir=$(dirname $0)
g++ -O2 -std=c++17 $base_dir/test_min-sep.cpp -o $base_dir/a.out
set +e
for i in {1..8}; do
    $base_dir/a.out < $base_dir/in/$i.in > $base_dir/tmp
    d=$(diff $base_dir/tmp $base_dir/out/$i.out)
    if [ $? = 1 ]; then
        echo "test $i failed"
        echo "=== output ==="
        cat $base_dir/tmp
        echo "=== expected ==="
        cat $base_dir/out/$i.out
        exit 1
    else
        echo "test $i passed"
    fi
done;

for i in {1..5000}; do
    $base_dir/a.out < $base_dir/in/rand_$i.in > $base_dir/tmp
    d=$(diff $base_dir/tmp $base_dir/out/rand_$i.out)
    if [ $? = 1 ]; then
        echo "test rand_$i failed"
        echo "=== output ==="
        cat $base_dir/tmp
        echo "=== expected ==="
        cat $base_dir/out/rand_$i.out
        exit 1
    else
        echo "test rand_$i passed"
    fi
done;
