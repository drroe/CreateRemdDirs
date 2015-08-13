#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 ConstF.rst.001 ConstF.rst.002

touch ConstF.rst.001
touch ConstF.rst.002

OPTLINE="-i umbrella.opts -b 0 -e 0 -c ../../CRD"
RunTest "Umbrella MD relative path test"
DoTest umbrella.groupfile.save run.000/groupfile
DoTest umbrella.mdin.2.save run.000/md.in.002

EndTest
