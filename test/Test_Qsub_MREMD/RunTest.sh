#!/bin/bash

. ../MasterTest.sh

CleanFiles run.00? mremd.opts Hamiltonians.dat qsub.opts analyze.opts Analyze.0.1 \
           Archive.0.1 archive.sbatch.0.1.sh RunArchive.0.1.sh \
           run1.sbatch.sh run1.sbatch.sh.save \
           run0.qsub.sh run0.qsub.sh.save \
           analyze.sbatch.sh analyze.sbatch.sh.save \
           archive.sbatch.0.1.sh archive.sbatch.0.1.sh.save

if [ -z "$AMBERHOME" ] ; then
  echo "Warning: Skipping submission test."
  echo "AMBERHOME not set."
  echo ""
  exit 0
fi

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
AMBERHOME $AMBERHOME 
EOF
}

MakeOpts SBATCH
OPTLINE="-i ../relative.mremd.opts -b 0 -e 1 -c ../../CRD -s -t"
RunTest "MREMD job submission test (SBATCH)."
DoTest ../mremd.dim.save run.000/remd.dim
#DoTest relative.groupfile.save run.000/groupfile
DoTest ../in.001.save run.000/INPUT/in.001
sed "s:amberhome:$AMBERHOME:g" run1.sbatch.sh.template > run1.sbatch.sh.save
DoTest run1.sbatch.sh.save run.001/sbatch.sh

MakeOpts PBS
OPTLINE=$OPTLINE" -O"
RunTest "MREMD job submission test (PBS)"
sed "s:amberhome:$AMBERHOME:g" run0.qsub.sh.template > run0.qsub.sh.save
DoTest run0.qsub.sh.save run.000/qsub.sh

echo "ANALYZE_FILE analyze.opts" >> qsub.opts
echo "ARCHIVE_FILE analyze.opts" >> qsub.opts
cat > analyze.opts <<EOF
NODES 16
PPN 1
WALLTIME 6:00:00
EMAIL me@fake.com
ACCOUNT testaccount
PROGRAM cpptraj
QSUB SBATCH
MPIRUN mpiexec -n \$THREADS
AMBERHOME $AMBERHOME 
EOF
OPTLINE="-i ../relative.mremd.opts --analyze --archive -b 0 -e 1 -s --nocheck -t"
RunTest "MREMD analysis/archive job submission test (PBS)"
DoTest RunAnalysis.sh.save Analyze.0.1/RunAnalysis.sh
DoTest batch.cpptraj.in.save Analyze.0.1/batch.cpptraj.in
sed "s:amberhome:$AMBERHOME:g" analyze.sbatch.sh.template > analyze.sbatch.sh.save
DoTest analyze.sbatch.sh.save Analyze.0.1/sbatch.sh
sed "s:amberhome:$AMBERHOME:g" archive.sbatch.0.1.sh.template > archive.sbatch.0.1.sh.save
DoTest archive.sbatch.0.1.sh.save archive.sbatch.0.1.sh
DoTest RunArchive.0.1.sh.save RunArchive.0.1.sh

EndTest
