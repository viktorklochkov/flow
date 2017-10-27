#!/bin/bash
numberofchunks=5
working=$PWD

#load Root if unavailable
function loadroot {
  if [[ ! $(which root) ]]; then
    source /lustre/nyx/alice/users/lkreis/root/bin/thisroot.sh
  fi
}
#merge outputfiles
function merge {
  #check slurm jobs before merging
  slurmjobs=`squeue -u$USER | wc -l`
  if [[ slurmjobs -ne 1 ]]; then
    slurmjobs=$(($slurmjobs-1))
    echo "not merging $slurmjobs jobs still running"
    return 1
  fi
  local runs=`cat $1`
  input=$2
  loadroot
  echo "run-by-run merging"
  for run in $runs; do
    cd $input/data/$run
    outputfiles=`find -mindepth 2 -name output.root`
    calibfiles=`find -mindepth 2 -name qn.root`
    if [[ -e output.root ]]; then
      prev=`ls -p1 | grep -v / | wc -l`.root
      mv output.root output$prev
    fi
    if [[ -e $input/correction/$run/correction.root ]]; then
      prev=`ls -p1 | grep -v / | wc -l`.root
      mv $input/correction/$run/correction.root $input/correction/$run/correction$prev
    fi
    hadd output.root $outputfiles 
    hadd correction.root $calibfiles
    mv "correction.root" "$input/correction/$run/correction.root"
    echo "deleting unmerged files"
    rm -R -- */
  done
  cd $working
  echo "merging done" >> $input/run.log
}

#runwise qa
function runqa {
  runs=`cat $1`
  output=$2
  cd $output/qa
  for run in $runs; do
    root -b -q -l "$working/qatask.C(\"${output}/data/${run}/output.root\", \"${run}\")"
  done
  cd $working
}

#split runlist into sublists of a specified length
#pass global list number and list of all chunks
function splitlist {
  number=$1
  output=$3
  split -l$numberofchunks $2 chunk
  for file in chunk*
  do
    ((number++))
    mv "$file" "$output/slurmlists/chunk$number"
    echo "chunk$number" >> $output/logs/list.log
  done
}
#create consecutively numbered lists of all chunks. 
#chunks are split by runs to allow run by run correction when executing the analysis
#a map of runnumber to chunknumber is kept for execution and merging
function createlists {
  echo "creating lists"
  runs=`cat $1`
  input=$2
  output=$3
  mkdir -p $output
  number=0
  for run in $runs
  do
    cd $output
    mkdir -p slurmlists
    mkdir -p slurmlogs
    mkdir -p logs
    mkdir -p runlists
    mkdir -p data
    mkdir -p qa
    mkdir -p correction/$run
    cd runlists
    mkdir -p $run
    cd $run
    find $input/$run -name dstTree.root > parts
    splitlist $number parts $output
    cd $output
    echo "$run $number" >> runmap
    cd $working
  done
} 
#submit jobs as a jobarray to slurm
#output directory of jobs
function submitjobs {
  script=/lustre/nyx/alice/users/lkreis/tools/runbyrun/run.sh
  app=/lustre/nyx/alice/users/lkreis/analysis/flow/build/main
  output=$1
   
  local cnumber=`ls -p1 $output/slurmlists/ | grep -v / | wc -l`
  if [[ $cnumber -ne $number ]]; then
    echo "something whent wrong number of jobs $number not equal number of lists $cnumber"
    return 2
  fi
  echo "submitting $number jobs."
  sbatch -J flow -o $output/slurmlogs/slurm-%A_%a.out -e $output/slurmlogs/slurm-%A_%a.out --array=1-$number $script $output $app 
  echo "jobs submitted" >> $output/run.log
}

#execute analysis.
#checks if lists are already generated if not regenerates all lists.
function analysis {
  #check if slurmjobs are still running
  #do not submit jobs if they are
  slurmjobs=`squeue -u$USER | wc -l`
  if [[ slurmjobs -ne 1 ]]; then
    slurmjobs=$(($slurmjobs-1))
    echo "not merging $slurmjobs jobs still running"
    return 1
  fi
  runs=$1
  input=$2
  output=$3
  loadroot
  if [[ -e $output/logs/list.log ]]; then
    echo "checking if lists are complete"
    local number=0
    while IFS= read -r f; do
      ((number++))
      if [[ ! -e $output/slurmlists/$f ]]; then
        echo "files missing $f"
        createlists $runs $input $output
        break
      fi
    done < $output/logs/list.log
    echo "lists complete"
    submitjobs $output
  else 
    createlists $runs $input $output
    submitjobs $output
  fi
}


