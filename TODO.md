
#TODO


##v_n calculation


##stability checks
* R_A[B,C] under different B and C

##comparison to published
* rewrite macros to produce nice plots

## Profile
* fix variance calculation; naive version is not stable.
* add histogram feature

##Find invalid bins

## Save correlations to file

## XeXe train reduced tree
* no fmd information inside the AOD


## special functions for adding mult scaling ...


victor synchronizing qncorrectionsframework

cut functionality for detectors

correction histograms with enough entries for correction?
check in framework

## Random subevent anti Q implementation
Anti Q

#DONE

remove autocorrelation select and integrate other components

####Add bootstrapping subsampling to the correlations for calculation of uncertainty
bootstrap in subsamples then add up.
possibly inside loop directly after building the correlations
temporary object single event information.
correlations

new samples container propagating subsamples the operations
mean from statistics in pos 0; error from subsampling in pos1...n

####possible to use hadd which functions are used?

##Add missing QA histograms
###tracks QA histograms
* ncls
* dcaxy dcaz
* TPC dE/dx
* TOF
* chi2/n.d.f.
* pt, eta, phi
* charge
###event QA histograms
* vtx
* centrality [VZERO, SPD tracklets, ZDC]
* signals [TPC, VO, FMD, SPD, ZDC]

##run-by-run corrections
* rewrite scripts to correct all runs individually.
* This should keep track of correction files and merge outputs accordingly.
* Run-wise QA is important.

##Q Vectors arithmetic
* projection of datacontainer qn
* adding two qvectors
* correlation of N data containers with lambda function.
* support for adding two qvectors  
* support for setting normalization  

d/dx x^-2 = 1 * -2x^-3