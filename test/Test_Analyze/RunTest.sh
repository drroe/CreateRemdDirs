#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 Analyze.0.0

mkdir -p run.000/TRAJ
touch run.000/TRAJ/rem.crd.001

OPTLINE="-i ../relative.mremd.opts -b 0 -e 0 --analyze --nocheck"
RunTest "Analyze input test"
DoTest batch.cpptraj.in.save Analyze.0.0/batch.cpptraj.in
DoTest RunAnalysis.sh.save Analyze.0.0/RunAnalysis.sh

EndTest
