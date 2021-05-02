#! /bin/bash

OneOneTest=./bin/OneOne
ManyOneTest=./bin/ManyOne

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Test Suite\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning One One Library Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

${OneOneTest}/unitTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Lock Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
${OneOneTest}/lockTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning single threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${OneOneTest}/benchmark single > /dev/null; } |& grep user  
done
# echo "======================================"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning multi threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${OneOneTest}/benchmark multi > /dev/null; } |& grep user  
done
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nOne One Tests Finished\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =


printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Many One Library Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =


${ManyOneTest}/unitTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nTesting Scheduling Parameters by running 500x500 multithreaded matrix multiplication"
tests=(1000 10000 100000 1000000 2000000)
for i in "${tests[@]}"
do
   sec=` expr $i / 1000000 `
   ms=` expr $i % 1000000 `
   echo -n "Scheduling Interval $sec s $ms μs => " && { time python3 src/OneOne/tests/script.py 500 | ${ManyOneTest}/matrix multi $i > /dev/null; } |& grep user  
done
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

echo -e "\nRunning Lock Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
${ManyOneTest}/lockTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning single threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${ManyOneTest}/matrix single > /dev/null; } |& grep user  
done
# echo "======================================"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning multi threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${ManyOneTest}/matrix multi > /dev/null; } |& grep user  
done
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nMany One Tests Finished\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
