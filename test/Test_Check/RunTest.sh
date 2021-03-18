#!/bin/bash

. ../MasterTest.sh

CleanFiles 

OPTLINE="-b 0 --check"
RunTest "MD check test."


EndTest
