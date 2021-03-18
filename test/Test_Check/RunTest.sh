#!/bin/bash

. ../MasterTest.sh

CleanFiles 

OPTLINE="-b 0 --check"
RunTest "MD check test."
TrimTestOutputHeader

if [ $HAS_NETCDF -eq 0 ] ; then
  DoTest nonetcdf.test.out.save test.out
else
  DoTest netcdf.test.out.save test.out
fi

EndTest
