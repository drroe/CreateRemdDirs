#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000

OPTLINE="-i md.opts -b 0 -e 0"
RunTest "Single MD override irest/ntx test"
DoTest ../RunMD.sh.save run.000/RunMD.sh
DoTest override.md.in.save run.000/md.in

EndTest
