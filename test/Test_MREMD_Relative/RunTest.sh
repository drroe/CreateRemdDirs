#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 mremd.opts Hamiltonians.dat

OPTLINE="-i ../relative.mremd.opts -b 0 -e 0 -c ../../CRD"
RunTest "M-REMD relative path test."
DoTest ../mremd.dim.save run.000/remd.dim
DoTest relative.groupfile.save run.000/groupfile
DoTest ../in.001.save run.000/INPUT/in.001

EndTest
