#!/bin/bash

function loadroot {
  if [[ $(which root) ]]; then
    echo "Root is available"
  else
    echo "Loading Root"
    source /lustre/nyx/alice/users/lkreis/root/bin/thisroot.sh
  fi
}
loadroot
app=$2
list=$1/slurmlists/chunk$SLURM_ARRAY_TASK_ID
listnumber=$SLURM_ARRAY_TASK_ID
output=$1
correction=$output/correction/
working=$PWD
cd $output
cd data
while read chunk; do
  arr=($chunk)
  runname=${arr[0]}
  chunknumber=${arr[1]}
  if [ $listnumber -le $chunknumber ] ;then
    printf -v listnumber "%04d" $listnumber
    mkdir -p $runname/$listnumber
    cd $runname/$listnumber
    break
  fi
done < $output/runmap
$app $list $correction/$runname/correction.root
cd $working
