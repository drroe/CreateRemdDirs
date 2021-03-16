#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 mremd.opts Hamiltonians.dat

OPTLINE="-i hremd.opts -b 0 -e 0"
RunTest "H-REMD single start coords test"
#DoTest ../mremd.dim.save run.000/remd.dim
DoTest groupfile.save run.000/groupfile
#DoTest in.001.save run.000/INPUT/in.001

EndTest
