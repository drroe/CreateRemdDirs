#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 Archive.0.0 RunArchive.0.0.sh

mkdir -p run.000/TRAJ
touch run.000/TRAJ/rem.crd.001

OPTLINE="-i ../relative.mremd.opts -b 0 -e 0 --archive --nocheck"
RunTest "Archive input test."
DoTest ar1.0.cpptraj.in.save Archive.0.0/ar1.0.cpptraj.in
DoTest ar2.0.cpptraj.in.save Archive.0.0/ar2.0.cpptraj.in
DoTest RunArchive.0.0.sh.save RunArchive.0.0.sh

EndTest
