#! /bin/bash

OneOneTest=./bin/OneOne
ManyOneTest=./bin/ManyOne

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Unit Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

${OneOneTest}/unitTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Lock Tests\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
${OneOneTest}/lockTests

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Readers Writers Problem Test\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo 2 | ${OneOneTest}/readers 

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning single threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${OneOneTest}/matrix single > /dev/null; } |& grep user  
done
# echo "======================================"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =

printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning multi threaded matrix multiplication"
tests=(10 100 200 500 800 1000)
for i in "${tests[@]}"
do
   echo -n "Input size $i x $i => " && { time python3 src/OneOne/tests/script.py $i | ${OneOneTest}/matrix multi > /dev/null; } |& grep user  
done
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =


printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
echo -e "\nRunning Many One Test\n"
printf '%*s\n' "${COLUMNS:-$(tput cols)}" '' | tr ' ' =
${ManyOneTest}/manyTests