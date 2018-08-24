#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 ConstF.rst

touch ConstF.rst

OPTLINE="-i mdrst.opts -b 0 -e 0"
RunTest "MD with restraints relative path test."
#DoTest ../mdsingle.groupfile.save run.000/groupfile
DoTest ../mdrst.in.save run.000/md.in
DoTest RunMD.sh.save run.000/RunMD.sh

EndTest
