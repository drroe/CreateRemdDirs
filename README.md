# CreateRemdDirs
Automatically generate input for running M-REMD simulations (and more) in Amber.

About CreateRemdDirs
====================
CreateRemdDirs can be used to automatically generate input for running REMD, 
H-REMD, M-REMD, and MD simulations that will be chained together (split up into
several runs). The general idea is to facilitate long simulation runs on e.g.
HPC resources. Currently CreateRemdDirs requires NetCDF libraries to perform
checks on Amber trajectories/restarts.

Disclaimer and Copyright
========================
CreateRemdDirs is Copyright (c) 2015 Daniel R. Roe.
The software is provided AS IS with no warranty.

# Brief Usage Guide
CreateRemdDirs uses an input file (by default named remd.opts) that describes
how a run should be set up. For example, a 2D M-REMD run (temperature + Hamiltonian)
might look like so:
```
DIMENSION   Temperatures.dat
DIMENSION   Hamiltonians.dat
NSTLIM      500
DT          0.002
NUMEXCHG    100
TEMPERATURE 300.0
MDIN_FILE   pme.remd.in
```
Runs are set up in subdirectories named run.XXX, where XXX is a 3 digit number. Each
run after the first is automatically set up to continue from the previous run.

The DIMENSION files describe what each dimension looks like. CreateRemdDirs currently
supports Temperature, Hamiltonian (Topology), and AMD Dihedral. A dimension file has
one header line which describes what kind of dimension it is followed by lines
 describing how the dimension is set up. For example, a Temperature dimension looks like so:
```
#Temperature
277.00
281.30
285.70
290.20
```

A Hamiltonian dimension looks like so:
```
#Hamiltonian
../../AltDFC.01.PagF.TIP3P.ff14SB.parm7
../../AltDFC.02.PagF.TIP3P.ff14SB.parm7
```
Note that the paths in a Hamiltonian dimension need to be either absolute or relative to
the run directories, i.e. if you 'ls' the file from inside a run.XXX directory it will
show up. 

An AMD dihedral boost dimension looks like so:
```
#amd_dihedral
0.0 0.0
50.0 112.0000000000
```
where the first number corresponds to alpha and the second to the threshhold.

Each REMD run directory will contain a groupfile, input files (in an INPUT subdirectory),
and a remd.dim file for M-REMD runs. It is expected that output will also be into
subdirectories, namely  
