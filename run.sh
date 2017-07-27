#!/bin/bash

runs=`cat $1`
inputDir=$2
outputDir=$3
workingDir=$PWD
mkdir -p $outputDir

for run in $runs; do
   mkdir -p $outputDir/$run
   cd  $outputDir/$run/
   find $inputDir/$run -name dstTree.root > chunkList.txt
   /Users/lukas/phd/analysis/flow/cmake-build-debug/./main chunkList.txt ./input.root
   cd ..
   cd $workingDir
done