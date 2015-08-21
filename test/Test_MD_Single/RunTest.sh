#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000

OPTLINE="-i md.opts -b 0 -e 0"
RunTest "Single MD relative path test"
#DoTest ../mdsingle.groupfile.save run.000/groupfile
DoTest ../RunMD.sh.save run.000/RunMD.sh
DoTest mdsingle.in.save run.000/md.in

EndTest
