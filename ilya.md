
#TODO


##v_n calculation


##stability checks
* R_A[B,C] under different B and C

##comparison to published
* rewrite macros to produce nice plots

##Q Vectors arithmetic
* do not use abs q normalization. it is not possible to revert the normalization right now.  

## Statistics
* fix variance calculation; naive version is not stable.
* add histogram feature

##Find invalid bins


#DONE

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
