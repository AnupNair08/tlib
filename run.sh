#! /bin/bash

echo "======================================"
echo -e "Running Unit Tests\n"
echo "======================================"

./bin/unitTests

echo "======================================"
echo -e "Running Lock Tests\n"
echo "======================================"
./bin/lockTests

echo "======================================"
echo -e "Running Matrix Multiplication Test\n"
echo "======================================"
echo "2 2 1 0 0 1 2 2 1 0 0 1" | ./bin/matrix

echo "======================================"
echo -e "Running Readers Writers Problem Test\n"
echo "======================================"
./bin/readers