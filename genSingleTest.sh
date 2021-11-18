#!/bin/bash

#output single test
ost()
{
    echo $1 >> output/SingleTest.cpp
}

#create single test
cst()
{
    ost "void innerTest();"

    for ((i=1; i<=$1; i++))
    do
        ost "static void st_${i}_0(){innerTest();}"
        for ((j=1; j<=$2; j++))
        do
            ((k=j-1))
            ost "static void st_${i}_${j}(){st_${i}_${k}();}"
        done
        ost "static void st_${i}(){st_${i}_${2}();}"
    done

    ost "void singleTest${3}(){"
    for ((i=1; i<=$1; i++))
    do
        ost "st_${i}();"
    done
    ost "}"
}

if [ ! -d output ]; then
    mkdir output
fi

fileCount=$1
btCount=$2
btDepth=$3

rm output/SingleTest* -rf

for ((x=1; x <= $fileCount; x++))
do
    rm output/SingleTest.cpp -rf
    cst $btCount $btDepth $x
    mv output/SingleTest.cpp output/SingleTest${x}.cpp
    g++ -c output/SingleTest${x}.cpp -o output/SingleTest${x}.o
done

for ((y=1; y<=$fileCount; y++))
do
    ost "void singleTest${y}();"
done

ost "void singleTest(){"
for ((z=1; z<=fileCount; z++))
do
    ost "singleTest${z}();"
done
ost "}"

g++ -c output/SingleTest.cpp -o output/SingleTest.o

exit 0

