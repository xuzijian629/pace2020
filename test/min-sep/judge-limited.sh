set -e
base_dir=$(dirname $0)
g++ -O2 -std=c++17 $base_dir/test_min-sep.cpp -o $base_dir/a.out
set +e
for i in {1..8}; do
    size=$(($RANDOM%9))
    $base_dir/a.out $size < $base_dir/in/$i.in > $base_dir/tmp
    cat $base_dir/out/$i.out | sed "/^[$(($size+1))-9] .*/d" > $base_dir/tmp2
    sed -i '' '1d' $base_dir/tmp
    sed -i '' '1d' $base_dir/tmp2
    d=$(diff $base_dir/tmp $base_dir/tmp2)
    if [ $? = 1 ]; then
        echo "test $i failed"
        echo "size = $size"
        echo "=== output ==="
        cat $base_dir/tmp
        echo "=== expected ==="
        cat $base_dir/tmp2
        exit 1
    else
        echo "test $i passed"
    fi
done;

for i in {1..5000}; do
    size=$(($RANDOM%9))
    $base_dir/a.out $size < $base_dir/in/rand_$i.in > $base_dir/tmp
    cat $base_dir/out/rand_$i.out | sed "/^[$(($size+1))-9] .*/d" > $base_dir/tmp2
    sed -i '' '1d' $base_dir/tmp
    sed -i '' '1d' $base_dir/tmp2
    d=$(diff $base_dir/tmp $base_dir/tmp2)
    if [ $? = 1 ]; then
        echo "test rand_$i failed"
        echo "size = $size"
        echo "=== output ==="
        cat $base_dir/tmp
        echo "=== expected ==="
        cat $base_dir/tmp2
        exit 1
    else
        echo "test rand_$i passed"
    fi
done;
