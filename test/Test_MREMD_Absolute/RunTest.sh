#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 mremd.opts Hamiltonians.dat absolute.groupfile.save

TESTDIR=`pwd`
TESTDIR=`dirname $TESTDIR`
if [ -z "$TESTDIR" ] ; then
  echo "Error: Could not get absolute path."
  exit 1
fi

cat > mremd.opts <<EOF
DIMENSION   ../Temperatures.dat
DIMENSION   Hamiltonians.dat 
DIMENSION   ../AmdDihedral.dat
NSTLIM      500
DT          0.002
NUMEXCHG    100
TEMPERATURE 300.0
TOPOLOGY    $TESTDIR/full.parm7
MDIN_FILE   $TESTDIR/pme.remd.gamma1.opts
REF_FILE $TESTDIR/CRD
# Only fully archive lowest Hamiltonian
FULLARCHIVE 0
EOF

cat > Hamiltonians.dat <<EOF
#Hamiltonian
$TESTDIR/AltDFC.01.PagF.TIP3P.ff14SB.parm7
$TESTDIR/AltDFC.02.PagF.TIP3P.ff14SB.parm7
EOF

OPTLINE="-i mremd.opts -b 0 -e 0 -c $TESTDIR/CRD"
RunTest "Absolute path test"
DoTest ../mremd.dim.save run.000/remd.dim
sed "s:TESTDIR:$TESTDIR:g" absolute.groupfile.template > absolute.groupfile.save
DoTest absolute.groupfile.save run.000/groupfile
DoTest ../in.001.save run.000/INPUT/in.001

EndTest
