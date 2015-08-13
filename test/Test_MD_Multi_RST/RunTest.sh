#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 ConstF.rst

touch ConstF.rst

OPTLINE="-i multi.mdrst.opts -b 0 -e 0 -c ../../CRD"
RunTest "Multi MD with restraints relative path test"
DoTest mdmulti.groupfile.save run.000/groupfile
DoTest ../mdrst.in.save run.000/md.in

EndTest
