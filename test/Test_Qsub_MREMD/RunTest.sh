#!/bin/bash

. ../MasterTest.sh

CleanFiles run.00? mremd.opts Hamiltonians.dat qsub.opts

MakeOpts() {
  cat > qsub.opts <<EOF
JOBNAME test
NODES 16
PPN 8
WALLTIME 24:00:00
EMAIL invalid@fake.com
ACCOUNT testaccount
PROGRAM pmemd
QSUB $1 
MPIRUN mpiexec -n \$THREADS
AMBERHOME /home/droe/Amber/GIT/amber
EOF
}

MakeOpts SBATCH
OPTLINE="-i ../relative.mremd.opts -b 0 -e 1 -c ../../CRD -s -t"
RunTest "MREMD job submission test (SBATCH)."
DoTest ../mremd.dim.save run.000/remd.dim
#DoTest relative.groupfile.save run.000/groupfile
DoTest ../in.001.save run.000/INPUT/in.001
DoTest run1.sbatch.sh.save run.001/sbatch.sh

MakeOpts PBS
OPTLINE=$OPTLINE" -O"
RunTest "MREMD job submission test (PBS)"
DoTest run0.qsub.sh.save run.000/qsub.sh

EndTest
