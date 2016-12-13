#!/bin/bash
g++ -o main main.cpp
echo "test begining..."
echo "test0.txt find prime between 2 and 100"
./main ./test/test0.txt
echo "press any key to continue"
read tmp
echo "test1.txt find gcd in two numbers"
./main ./test/test1.txt 
echo "press any key to continue"
read tmp
echo "basic.txt test for basic"
./main ./test/basic.txt
echo "press any key to continue"
read tmp
echo "basic1.txt test for basic"
./main ./test/basic1.txt 
echo "press any key to continue"
read tmp
echo "basic2.txt test for if else write"
./main ./test/basic2.txt 
echo "press any key to continue"
read tmp
