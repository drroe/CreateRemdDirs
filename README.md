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

## Creating Input
Basic usage is `CreateRemdDirs -b <start run #> -e <end run #>`. If only one run is desired
only `-b` need be specified.`Runs are set up in subdirectories named run.XXX, where XXX is a
3 digit number corresponding to the run number. Input coordinates can be either specified
in the remd.opts file or on the command line with `-c` (path must be relative to inside
the run.XXX directories). Each run after the first is automatically set up to continue from
the previous run. Run #0 is special in that it will not be set up as a restart (irest = 0).

The MDIN_FILE should contain Amber MD input flags that belong in the &cntrl namelist. The
following flags are set from the remd.opts file and/or automatically: imin (always 0), 
nstlim, dt, irest, ntx, ig, numexchg, temp0, tempi. All other necessary flags should be
in MDIN_FILE. For example:
```
    ntwx = 5000, ioutfm = 1, ntwr = 100000, ntxo = 2, ntpr = 5000,
    iwrap = 1, nscm = 1000, 
    ntc = 2, ntf = 2, ntb = 1, cut = 8.0,
    ntt = 3, gamma_ln = 1, 
    ntp = 0,
```

Each REMD run directory will contain a groupfile, input files (in an INPUT subdirectory),
and a remd.dim file for M-REMD runs. It is expected that output will also be into
subdirectories, namely INFO for mdinfo files, LOG for log files (PMEMD only),
OUTPUT for mdout files, RST for restart files, and TRAJ for trajectory files. An aMD
directory will be created for aMD output files. 

## Dimensions
The DIMENSION files describe what each dimension looks like. CreateRemdDirs currently
supports Temperature, Hamiltonian (Topology), and accelerated MD (aMD) Dihedral.
A dimension file has one header line which describes what kind of dimension it is followed by lines
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

An aMD dihedral boost dimension looks like so:
```
#amd_dihedral
0.0 0.0
50.0 112.0000000000
```
where the first number corresponds to alpha and the second to the threshhold.


