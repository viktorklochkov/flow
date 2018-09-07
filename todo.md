# TODO

## TODO

### v\_n calculation

* systematics axes... created from cuts...

  not true axis correlated bins... introduce category axis and fill with phi\[N\]

  where N is the number of variations... 

  Apply cut which modifies category variable and assigns to bin number

**Profile**

* fix variance calculation; naive version is not stable.
* add histogram feature

**special functions for adding mult scaling ...**

victor synchronizing qncorrectionsframework

correction histograms with enough entries for correction? check in framework

**Random subevent anti Q implementation**

Anti Q

## DONE

### comparison to published

* rewrite macros to produce nice plots

cut functionality for detectors

### Add track histograms to histogram manager

### Better filling of detectors using one values array

remove autocorrelation select and integrate other components

**Add bootstrapping subsampling to the correlations for calculation of uncertainty**

bootstrap in subsamples then add up. possibly inside loop directly after building the correlations temporary object single event information. correlations

new samples container propagating subsamples the operations mean from statistics in pos 0; error from subsampling in pos1...n

**possible to use hadd which functions are used?**

### Add missing QA histograms

#### tracks QA histograms

* ncls
* dcaxy dcaz
* TPC dE/dx
* TOF
* chi2/n.d.f.
* pt, eta, phi
* charge

  **event QA histograms**

* vtx
* centrality \[VZERO, SPD tracklets, ZDC\]
* signals \[TPC, VO, FMD, SPD, ZDC\]

### run-by-run corrections

* rewrite scripts to correct all runs individually.
* This should keep track of correction files and merge outputs accordingly.
* Run-wise QA is important.

### Q Vectors arithmetic

* projection of datacontainer qn
* adding two qvectors
* correlation of N data containers with lambda function.
* support for adding two qvectors  
* support for setting normalization  

d/dx x^-2 = 1 \* -2x^-3

