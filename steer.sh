#!/bin/bash

runs=`cat $1`
inputDir=$2
outputDir=$3
nworkers=$4
task=$5
mkdir -p $outputDir
for i in `seq 1 $nworkers`; do
   > $outputDir/runList.$i.txt
done

currentFile=1
for run in $runs; do
   content=`cat $outputDir/runList.$currentFile.txt`
   printf "$content\n$run" > $outputDir/runList.$currentFile.txt
   let currentFile=currentFile+1
   if [ $currentFile -gt $nworkers ]; then
      currentFile=1
   fi
done

workDir=$PWD
for i in `seq 1 $nworkers`; do
   sbatch $workDir/run.sh $outputDir/runList.$i.txt $inputDir $outputDir > $outputDir/worker$i.out &
done