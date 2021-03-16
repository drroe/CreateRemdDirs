#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000

OPTLINE="-i md.opts -b 0 -e 0"
RunTest "Single MD Additional namelist test"
DoTest additionalNamelist.md.in.save run.000/md.in

EndTest
