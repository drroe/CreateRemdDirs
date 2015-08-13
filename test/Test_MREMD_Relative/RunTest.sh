#!/bin/bash

. ../MasterTest.sh

CleanFiles run.000 mremd.opts Hamiltonians.dat

cat > mremd.opts <<EOF
DIMENSION   ../Temperatures.dat
DIMENSION   Hamiltonians.dat 
DIMENSION   ../AmdDihedral.dat
NSTLIM      500
DT          0.002
NUMEXCHG    100
TEMPERATURE 300.0
MDIN_FILE   ../pme.remd.gamma1.opts
# Only fully archive lowest Hamiltonian
FULLARCHIVE 0
EOF

cat > Hamiltonians.dat <<EOF
#Hamiltonian
../../AltDFC.01.PagF.TIP3P.ff14SB.parm7
../../AltDFC.02.PagF.TIP3P.ff14SB.parm7
EOF

OPTLINE="-i mremd.opts -b 0 -e 0 -c ../../CRD"
RunTest "M-REMD relative path test."
DoTest mremd.dim.save run.000/remd.dim
DoTest relative.groupfile.save run.000/groupfile
DoTest in.001.save run.000/INPUT/in.001

EndTest
