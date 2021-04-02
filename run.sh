#! /bin/bash

# echo "======================================"
# echo -e "Running Unit Tests\n"
# echo "======================================"

# ./bin/unitTests

# echo "======================================"
# echo -e "Running Lock Tests\n"
# echo "======================================"
# ./bin/lockTests

# echo "======================================"
# echo -e "Running Readers Writers Problem Test\n"
# echo "======================================"
# ./bin/readers

echo "======================================"
echo -e "Running single threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 tests/script.py $i | ./bin/matrix single > /dev/null; } |& grep user  
done
echo "======================================"

echo "======================================"
echo -e "Running multi threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 tests/script.py $i | ./bin/matrix multi > /dev/null; } |& grep user  
done
echo "======================================"


# echo "======================================"
# echo -e "Running Many One Test\n"
# echo "======================================"
# ./bin/manyTests